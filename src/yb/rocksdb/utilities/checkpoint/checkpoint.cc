//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.
//
// The following only applies to changes made to this file as part of YugaByte development.
//
// Portions Copyright (c) YugaByte, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
// in compliance with the License.  You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied.  See the License for the specific language governing permissions and limitations
// under the License.
//
// Copyright (c) 2012 Facebook.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ROCKSDB_LITE

#include "yb/rocksdb/utilities/checkpoint.h"

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>
#include <algorithm>
#include <string>
#include "yb/rocksdb/db/filename.h"
#include "yb/rocksdb/db/wal_manager.h"
#include "yb/rocksdb/db.h"
#include "yb/rocksdb/env.h"
#include "yb/rocksdb/transaction_log.h"
#include "yb/rocksdb/util/file_util.h"
#include "yb/rocksdb/port/port.h"

namespace rocksdb {

class CheckpointImpl : public Checkpoint {
 public:
  // Creates a Checkpoint object to be used for creating openable sbapshots
  explicit CheckpointImpl(DB* db) : db_(db) {}

  // Builds an openable snapshot of RocksDB on the same disk, which
  // accepts an output directory on the same disk, and under the directory
  // (1) hard-linked SST files pointing to existing live SST files
  // SST files will be copied if output directory is on a different filesystem
  // (2) a copied manifest files and other files
  // The directory should not already exist and will be created by this API.
  // The directory will be an absolute path
  using Checkpoint::CreateCheckpoint;
  Status CreateCheckpoint(const std::string& checkpoint_dir) override;

 private:
  DB* db_;
};

Status Checkpoint::Create(DB* db, Checkpoint** checkpoint_ptr) {
  *checkpoint_ptr = new CheckpointImpl(db);
  return Status::OK();
}

Status Checkpoint::CreateCheckpoint(const std::string& checkpoint_dir) {
  return STATUS(NotSupported, "");
}

// Builds an openable snapshot of RocksDB
Status CheckpointImpl::CreateCheckpoint(const std::string& checkpoint_dir) {
  Status s;
  std::vector<std::string> live_files;
  uint64_t manifest_file_size = 0;
  uint64_t sequence_number = db_->GetLatestSequenceNumber();
  bool same_fs = true;
  VectorLogPtr live_wal_files;

  s = db_->GetEnv()->FileExists(checkpoint_dir);
  if (s.ok()) {
    return STATUS(InvalidArgument, "Directory exists");
  } else if (!s.IsNotFound()) {
    assert(s.IsIOError());
    return s;
  }

  s = db_->DisableFileDeletions();
  if (s.ok()) {
    // this will return live_files prefixed with "/"
    s = db_->GetLiveFiles(live_files, &manifest_file_size, true);
  }
  // if we have more than one column family, we need to also get WAL files
  if (s.ok()) {
    s = db_->GetSortedWalFiles(&live_wal_files);
  }
  if (!s.ok()) {
    db_->EnableFileDeletions(false);
    return s;
  }

  size_t wal_size = live_wal_files.size();
  RLOG(db_->GetOptions().info_log,
      "Started the snapshot process -- creating snapshot in directory %s",
      checkpoint_dir.c_str());

  std::string full_private_path = checkpoint_dir + ".tmp";

  // create snapshot directory
  s = db_->GetEnv()->CreateDir(full_private_path);

  // copy/hard link live_files
  for (size_t i = 0; s.ok() && i < live_files.size(); ++i) {
    uint64_t number;
    FileType type;
    bool ok = ParseFileName(live_files[i], &number, &type);
    if (!ok) {
      s = STATUS(Corruption, "Can't parse file name. This is very bad");
      break;
    }
    // we should only get sst, manifest and current files here
    assert(type == kTableFile || type == kTableSBlockFile || type == kDescriptorFile ||
           type == kCurrentFile);
    assert(live_files[i].size() > 0 && live_files[i][0] == '/');
    std::string src_fname = live_files[i];

    // rules:
    // * if it's kTableFile or kTableSBlockFile, then it's shared
    // * if it's kDescriptorFile, limit the size to manifest_file_size
    // * always copy if cross-device link
    bool is_table_file = type == kTableFile || type == kTableSBlockFile;
    if (is_table_file && same_fs) {
      RLOG(db_->GetOptions().info_log, "Hard Linking %s", src_fname.c_str());
      s = db_->GetEnv()->LinkFile(db_->GetName() + src_fname,
                                  full_private_path + src_fname);
      if (s.IsNotSupported()) {
        same_fs = false;
        s = Status::OK();
      }
    }
    if (!is_table_file || (!same_fs)) {
      RLOG(db_->GetOptions().info_log, "Copying %s", src_fname.c_str());
      s = CopyFile(db_->GetEnv(), db_->GetName() + src_fname,
                   full_private_path + src_fname,
                   (type == kDescriptorFile) ? manifest_file_size : 0);
    }
  }
  RLOG(db_->GetOptions().info_log, "Number of log files %" ROCKSDB_PRIszt,
      live_wal_files.size());

  // Link WAL files. Copy exact size of last one because it is the only one
  // that has changes after the last flush.
  for (size_t i = 0; s.ok() && i < wal_size; ++i) {
    if ((live_wal_files[i]->Type() == kAliveLogFile) &&
        (live_wal_files[i]->StartSequence() >= sequence_number)) {
      if (i + 1 == wal_size) {
        RLOG(db_->GetOptions().info_log, "Copying %s",
            live_wal_files[i]->PathName().c_str());
        s = CopyFile(db_->GetEnv(),
                     db_->GetOptions().wal_dir + live_wal_files[i]->PathName(),
                     full_private_path + live_wal_files[i]->PathName(),
                     live_wal_files[i]->SizeFileBytes());
        break;
      }
      if (same_fs) {
        // we only care about live log files
        RLOG(db_->GetOptions().info_log, "Hard Linking %s",
            live_wal_files[i]->PathName().c_str());
        s = db_->GetEnv()->LinkFile(
            db_->GetOptions().wal_dir + live_wal_files[i]->PathName(),
            full_private_path + live_wal_files[i]->PathName());
        if (s.IsNotSupported()) {
          same_fs = false;
          s = Status::OK();
        }
      }
      if (!same_fs) {
        RLOG(db_->GetOptions().info_log, "Copying %s",
            live_wal_files[i]->PathName().c_str());
        s = CopyFile(db_->GetEnv(),
                     db_->GetOptions().wal_dir + live_wal_files[i]->PathName(),
                     full_private_path + live_wal_files[i]->PathName(), 0);
      }
    }
  }

  // we copied all the files, enable file deletions
  db_->EnableFileDeletions(false);

  if (s.ok()) {
    // move tmp private backup to real snapshot directory
    s = db_->GetEnv()->RenameFile(full_private_path, checkpoint_dir);
  }
  if (s.ok()) {
    unique_ptr<Directory> checkpoint_directory;
    db_->GetEnv()->NewDirectory(checkpoint_dir, &checkpoint_directory);
    if (checkpoint_directory != nullptr) {
      s = checkpoint_directory->Fsync();
    }
  }

  if (!s.ok()) {
    // clean all the files we might have created
    RLOG(db_->GetOptions().info_log, "Snapshot failed -- %s",
        s.ToString().c_str());
    // we have to delete the dir and all its children
    std::vector<std::string> subchildren;
    db_->GetEnv()->GetChildren(full_private_path, &subchildren);
    for (auto& subchild : subchildren) {
      Status s1 = db_->GetEnv()->DeleteFile(full_private_path + subchild);
      if (s1.ok()) {
        RLOG(db_->GetOptions().info_log, "Deleted %s",
            (full_private_path + subchild).c_str());
      }
    }
    // finally delete the private dir
    Status s1 = db_->GetEnv()->DeleteDir(full_private_path);
    RLOG(db_->GetOptions().info_log, "Deleted dir %s -- %s",
        full_private_path.c_str(), s1.ToString().c_str());
    return s;
  }

  // here we know that we succeeded and installed the new snapshot
  RLOG(db_->GetOptions().info_log, "Snapshot DONE. All is good");
  RLOG(db_->GetOptions().info_log, "Snapshot sequence number: %" PRIu64,
      sequence_number);

  return s;
}
}  // namespace rocksdb

#endif  // ROCKSDB_LITE
