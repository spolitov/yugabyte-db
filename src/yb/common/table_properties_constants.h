// Copyright (c) YugaByte, Inc.
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

#ifndef YB_COMMON_TABLE_PROPERTIES_CONSTANTS_H
#define YB_COMMON_TABLE_PROPERTIES_CONSTANTS_H

#include <cstring>
#include <map>
#include <limits>
#include <set>
#include <utility>
#include "yb/common/common.pb.h"
#include "yb/util/monotime.h"
#include "yb/util/string_case.h"

namespace yb {
namespace common {

// Upper bound for TTL in Cassandra is max int32 value (in seconds).
static const MonoDelta kMaxTtl = MonoDelta::FromSeconds(std::numeric_limits<int32_t>::max());

static const int64_t kMaxTtlSeconds = kMaxTtl.ToSeconds();

static const int64_t kMinTtlSeconds = 0;

// Verifies whether the TTL provided in milliseconds is valid.
inline static bool IsValidTTLSeconds(int64_t ttl_seconds) {
  return (ttl_seconds >= kMinTtlSeconds && ttl_seconds <= kMaxTtlSeconds);
}

static constexpr auto kSpeculativeRetryAlways = "always";
static constexpr auto kSpeculativeRetryMs = "ms";
static constexpr auto kSpeculativeRetryNone = "none";
static constexpr auto kSpeculativeRetryPercentile = "percentile";

static const auto kSpeculativeRetryMsLen = std::strlen(kSpeculativeRetryMs);
static const auto kSpeculativeRetryPercentileLen = std::strlen(kSpeculativeRetryPercentile);

static constexpr auto kCachingKeys = "keys";
static constexpr auto kCachingRowsPerPartition = "rows_per_partition";
static constexpr auto kCachingAll = "ALL";
static constexpr auto kCachingNone = "NONE";

inline static bool IsValidCachingKeysString(const std::string& str) {
  std::string upper_str;
  ToUpperCase(str, &upper_str);
  return (upper_str == kCachingAll || upper_str == kCachingNone);
}

inline static bool IsValidCachingRowsPerPartitionString(const std::string& str) {
  return IsValidCachingKeysString(str);
}

inline static bool IsValidCachingRowsPerPartitionInt(const int64_t val) {
  return val <= std::numeric_limits<int32_t>::max();
}

}  // namespace common
}  // namespace yb

#endif // YB_COMMON_TABLE_PROPERTIES_CONSTANTS_H
