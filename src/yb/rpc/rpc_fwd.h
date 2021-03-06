//
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
//

#ifndef YB_RPC_RPC_FWD_H
#define YB_RPC_RPC_FWD_H

#include <chrono>

#include <boost/intrusive_ptr.hpp>

#include "yb/gutil/ref_counted.h"

namespace boost {
namespace asio {

class io_service;

} // namespace asio
} // namespace boost

namespace yb {
namespace rpc {

class AcceptorPool;
class Messenger;
class RpcCommand;
class RpcContext;
class RpcController;
class Scheduler;

class Connection;
typedef std::shared_ptr<Connection> ConnectionPtr;

class InboundCall;
typedef std::shared_ptr<InboundCall> InboundCallPtr;

class OutboundCall;
typedef std::shared_ptr<OutboundCall> OutboundCallPtr;

class ErrorStatusPB;

typedef boost::asio::io_service IoService;

// SteadyTimePoint is something like MonoTime, but 3rd party libraries know it and don't know about
// our private MonoTime.
typedef std::chrono::steady_clock::time_point SteadyTimePoint;

} // namespace rpc
} // namespace yb

#endif // YB_RPC_RPC_FWD_H
