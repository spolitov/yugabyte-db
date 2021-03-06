// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
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
#ifndef YB_TABLET_MOCK_ROWSETS_H
#define YB_TABLET_MOCK_ROWSETS_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "yb/gutil/strings/substitute.h"
#include "yb/tablet/rowset.h"
#include "yb/tablet/rowset_metadata.h"

namespace yb {
namespace tablet {

// Mock implementation of RowSet which just aborts on every call.
class MockRowSet : public RowSet {
 public:
  virtual CHECKED_STATUS CheckRowPresent(const RowSetKeyProbe &probe, bool *present,
                                 ProbeStats* stats) const override {
    LOG(FATAL) << "Unimplemented";
    return Status::OK();
  }
  virtual CHECKED_STATUS MutateRow(HybridTime hybrid_time,
                           const RowSetKeyProbe &probe,
                           const RowChangeList &update,
                           const consensus::OpId& op_id_,
                           ProbeStats* stats,
                           OperationResultPB *result) override {
    LOG(FATAL) << "Unimplemented";
    return Status::OK();
  }
  virtual CHECKED_STATUS NewRowIterator(const Schema *projection,
                                const MvccSnapshot &snap,
                                gscoped_ptr<RowwiseIterator>* out) const override {
    LOG(FATAL) << "Unimplemented";
    return Status::OK();
  }
  virtual CHECKED_STATUS NewCompactionInput(const Schema* projection,
                                    const MvccSnapshot &snap,
                                    gscoped_ptr<CompactionInput>* out) const override {
    LOG(FATAL) << "Unimplemented";
    return Status::OK();
  }
  virtual CHECKED_STATUS CountRows(rowid_t *count) const override {
    LOG(FATAL) << "Unimplemented";
    return Status::OK();
  }
  virtual std::string ToString() const override {
    LOG(FATAL) << "Unimplemented";
    return "";
  }
  virtual CHECKED_STATUS DebugDump(vector<std::string> *lines = NULL) override {
    LOG(FATAL) << "Unimplemented";
    return Status::OK();
  }
  virtual CHECKED_STATUS Delete() {
    LOG(FATAL) << "Unimplemented";
    return Status::OK();
  }
  virtual uint64_t EstimateOnDiskSize() const override {
    LOG(FATAL) << "Unimplemented";
    return 0;
  }
  virtual std::mutex *compact_flush_lock() override {
    LOG(FATAL) << "Unimplemented";
    return NULL;
  }
  virtual std::shared_ptr<RowSetMetadata> metadata() override {
    LOG(FATAL) << "Unimplemented";
    return std::shared_ptr<RowSetMetadata>(
      reinterpret_cast<RowSetMetadata *>(NULL));
  }

  virtual size_t DeltaMemStoreSize() const override {
    LOG(FATAL) << "Unimplemented";
    return 0;
  }

  virtual bool DeltaMemStoreEmpty() const override {
    LOG(FATAL) << "Unimplemented";
    return 0;
  }

  virtual int64_t MinUnflushedLogIndex() const override {
    LOG(FATAL) << "Unimplemented";
    return -1;
  }

  virtual double DeltaStoresCompactionPerfImprovementScore(DeltaCompactionType type)
      const override {
    LOG(FATAL) << "Unimplemented";
    return 0;
  }

  virtual CHECKED_STATUS FlushDeltas() override {
    LOG(FATAL) << "Unimplemented";
    return Status::OK();
  }

  virtual CHECKED_STATUS MinorCompactDeltaStores() override {
    LOG(FATAL) << "Unimplemented";
    return Status::OK();
  }

  virtual bool IsAvailableForCompaction() override {
    return true;
  }
};

// Mock which implements GetBounds() with constant provided bonuds.
class MockDiskRowSet : public MockRowSet {
 public:
  MockDiskRowSet(std::string first_key, std::string last_key,
                 int size = 1000000)
      : first_key_(std::move(first_key)),
        last_key_(std::move(last_key)),
        size_(size) {}

  virtual CHECKED_STATUS GetBounds(Slice *min_encoded_key,
                           Slice *max_encoded_key) const override {
    *min_encoded_key = Slice(first_key_);
    *max_encoded_key = Slice(last_key_);
    return Status::OK();
  }

  virtual uint64_t EstimateOnDiskSize() const override {
    return size_;
  }

  virtual std::string ToString() const override {
    return strings::Substitute("mock[$0, $1]",
                               Slice(first_key_).ToDebugString(),
                               Slice(last_key_).ToDebugString());
  }

 private:
  const std::string first_key_;
  const std::string last_key_;
  const uint64_t size_;
};

// Mock which acts like a MemRowSet and has no known bounds.
class MockMemRowSet : public MockRowSet {
 public:
  virtual CHECKED_STATUS GetBounds(Slice *min_encoded_key,
                           Slice *max_encoded_key) const override {
    return STATUS(NotSupported, "");
  }

 private:
  const std::string first_key_;
  const std::string last_key_;
};

} // namespace tablet
} // namespace yb
#endif /* YB_TABLET_MOCK_ROWSETS_H */
