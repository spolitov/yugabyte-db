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
// Simplistic block encoding for strings.
//
// The block consists of:
// Header:
//   ordinal_pos (32-bit fixed)
//   num_elems (32-bit fixed)
//   offsets_pos (32-bit fixed): position of the first offset, relative to block start
// Strings:
//   raw strings that were written
// Offsets:  [pointed to by offsets_pos]
//   gvint-encoded offsets pointing to the beginning of each string
#ifndef YB_CFILE_BINARY_PLAIN_BLOCK_H
#define YB_CFILE_BINARY_PLAIN_BLOCK_H

#include <vector>

#include "yb/cfile/block_encodings.h"
#include "yb/util/faststring.h"

namespace yb {
namespace cfile {

struct WriterOptions;

class BinaryPlainBlockBuilder : public BlockBuilder {
 public:
  explicit BinaryPlainBlockBuilder(const WriterOptions *options);

  bool IsBlockFull(size_t limit) const override;

  int Add(const uint8_t *vals, size_t count) override;

  // Return a Slice which represents the encoded data.
  //
  // This Slice points to internal data of this class
  // and becomes invalid after the builder is destroyed
  // or after Finish() is called again.
  Slice Finish(rowid_t ordinal_pos) override;

  void Reset() override;

  size_t Count() const override;

  // Return the first added key.
  // key should be a Slice *
  CHECKED_STATUS GetFirstKey(void *key) const override;

  // Length of a header.
  static const size_t kMaxHeaderSize = sizeof(uint32_t) * 3;

 private:
  faststring buffer_;

  size_t end_of_data_offset_;
  size_t size_estimate_;

  // Offsets of each entry, relative to the start of the block
  std::vector<uint32_t> offsets_;

  bool finished_;

  const WriterOptions *options_;

};

class BinaryPlainBlockDecoder : public BlockDecoder {
 public:
  explicit BinaryPlainBlockDecoder(Slice slice);

  virtual CHECKED_STATUS ParseHeader() override;
  virtual void SeekToPositionInBlock(uint pos) override;
  virtual CHECKED_STATUS SeekAtOrAfterValue(const void *value,
                                    bool *exact_match) override;
  CHECKED_STATUS CopyNextValues(size_t *n, ColumnDataView *dst) override;

  virtual bool HasNext() const override {
    DCHECK(parsed_);
    return cur_idx_ < num_elems_;
  }

  virtual size_t Count() const override {
    DCHECK(parsed_);
    return num_elems_;
  }

  virtual size_t GetCurrentIndex() const override {
    DCHECK(parsed_);
    return cur_idx_;
  }

  virtual rowid_t GetFirstRowId() const override {
    return ordinal_pos_base_;
  }

  Slice string_at_index(size_t idx) const {
    const uint32_t offset = offsets_[idx];
    uint32_t len = offsets_[idx + 1] - offset;
    return Slice(&data_[offset], len);
  }

  // Minimum length of a header.
  static const size_t kMinHeaderSize = sizeof(uint32_t) * 3;

 private:
  Slice data_;
  bool parsed_;

  // The parsed offsets.
  // This array also contains one extra offset at the end, pointing
  // _after_ the last entry. This makes the code much simpler.
  std::vector<uint32_t> offsets_;

  uint32_t num_elems_;
  rowid_t ordinal_pos_base_;

  // Index of the currently seeked element in the block.
  uint32_t cur_idx_;
};

} // namespace cfile
} // namespace yb

#endif // YB_CFILE_BINARY_PREFIX_BLOCK_H
