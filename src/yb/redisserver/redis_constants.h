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

#ifndef YB_REDISSERVER_REDIS_CONSTANTS_H
#define YB_REDISSERVER_REDIS_CONSTANTS_H

#include "yb/common/redis_constants_common.h"

static constexpr const char* const kRedisKeyColumnName = "key";
static constexpr uint16_t kRedisClusterSlots = 16384;

#endif  // YB_REDISSERVER_REDIS_CONSTANTS_H
