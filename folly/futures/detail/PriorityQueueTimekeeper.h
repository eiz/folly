/*
 * Copyright 2015 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/futures/Future.h>
#include <folly/futures/Timekeeper.h>
#include <thread>

namespace folly { namespace detail {

/// A portable Timekeeper implementation which uses a priority queue and condition
/// variable on a dedicated thread. Users needn't deal with this directly, it is
/// used by default by Future methods that work with timeouts.
///
/// This is probably slower than the fancy HHWheelTimer-based default one, but it
/// doesn't require libevent or the io/async subtree in general.
class PriorityQueueTimekeeper: public Timekeeper {
 public:
  /// But it doesn't *have* to be a singleton.
  PriorityQueueTimekeeper();
  ~PriorityQueueTimekeeper() override;

  /// Implement the Timekeeper interface
  /// This future *does* complete on the timer thread. You should almost
  /// certainly follow it with a via() call or the accuracy of other timers
  /// will suffer.
  Future<Unit> after(Duration) override;

 private:
  struct ScheduledTimer
  {
    static ScheduledTimer* create(PriorityQueueTimekeeper* keeper) {
      return new ScheduledTimer(keeper);
    }

    Future<Unit> getFuture() {
      return promise_.getFuture();
    }
  private:
    ScheduledTimer(PriorityQueueTimekeeper* keeper): keeper_(keeper) {
      promise_.setInterruptHandler(
        std::bind(&ScheduledTimer::interruptHandler, this));
    }

    void interruptHandler() {
      keeper_->remove(this);
      delete this;
    }

    Promise<Unit> promise_;
    PriorityQueueTimekeeper* keeper_;
  };

  void remove(ScheduledTimer*);

  std::thread thread_;
  std::vector<ScheduledTimer*> schedule_;
  std::recursive_mutex mutex_;
  std::condition_variable cvar_;
};

Timekeeper* getPortableTimekeeperSingleton();

}} // folly::detail
