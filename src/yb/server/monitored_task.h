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

#ifndef YB_SERVER_MONITORED_TASK_H
#define YB_SERVER_MONITORED_TASK_H

#include <string>

#include "yb/gutil/ref_counted.h"
#include "yb/util/monotime.h"

namespace yb {

class MonitoredTask : public std::enable_shared_from_this<MonitoredTask> {
 public:
  virtual ~MonitoredTask() {}

  enum State {
    kStateWaiting,  // RPC not issued, or is waiting to be retried.
    kStateRunning,  // RPC has been issued.
    kStateComplete,
    kStateFailed,
    kStateAborted,
    kStateScheduling,
  };

  static string state(State state) {
    switch (state) {
    case MonitoredTask::kStateWaiting:
      return "Waiting";
    case MonitoredTask::kStateRunning:
      return "Running";
    case MonitoredTask::kStateComplete:
      return "Complete";
    case MonitoredTask::kStateFailed:
      return "Failed";
    case MonitoredTask::kStateAborted:
      return "Aborted";
    case MonitoredTask::kStateScheduling:
      return "Scheduling";
    }
    return "UNKNOWN_STATE";
  }

  // Abort this task and return its value before it was successfully aborted. If the task entered
  // a different terminal state before we were able to abort it, return that state.
  virtual State AbortAndReturnPrevState() = 0;

  // Task State
  virtual State state() const = 0;

  enum Type {
    ASYNC_CREATE_REPLICA,
    ASYNC_DELETE_REPLICA,
    ASYNC_ALTER_TABLE,
    ASYNC_CHANGE_CONFIG,
    ASYNC_ADD_SERVER,
    ASYNC_REMOVE_SERVER,
    ASYNC_TRY_STEP_DOWN,
  };

  virtual Type type() const = 0;

  // Task Type Identifier
  virtual std::string type_name() const = 0;

  // Task description
  virtual std::string description() const = 0;

  // Task start time, may be !Initialized()
  virtual MonoTime start_timestamp() const = 0;

  // Task completion time, may be !Initialized()
  virtual MonoTime completion_timestamp() const = 0;
};

} // namespace yb

#endif  // YB_SERVER_MONITORED_TASK_H
