#pragma once
//Author: Ugo Varetto
//
// This file is part of tjpp.
//tjpp is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//tjpp is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with tjpp.  If not, see <http://www.gnu.org/licenses/>.

//! \file SyncQueue.h
//! \brief Synchronized queue
//!
//! Implementation of a synchronized queue.

#include <deque>
#include <mutex>
#include <condition_variable>

namespace tjpp {
//! Synchronized queue:
//! @c Pop() waits for data
template< typename T >
class SyncQueue {
public:
    //! Push data to back of the queue.
    void Push(const T& e) {
        std::lock_guard< std::mutex > guard(mutex_);
        queue_.push_back(e);
        cond_.notify_one(); //notify
    }
    //! Push data to front of queue.
    //! Used to add a high piority message, normally to signal
    //! end of operations
    void PushFront(const T& e) {
        std::lock_guard< std::mutex > guard(mutex_);
        queue_.push_front(e);
        cond_.notify_one(); //notify
    }
    //! Add elemented in [begin, end) interval to queue
    //! in a single atomic operation.
    template< typename FwdT >
    void Buffer(FwdT begin, FwdT end) {
        std::lock_guard< std::mutex > guard(mutex_);
        while(begin++ != end)
            queue_.push_back(*begin);
        cond_.notify_one();
    }
    //! Return and remove element in front of queue.
    //! Waits indefinitely for an element to be available.
    T Pop() {
        std::unique_lock< std::mutex > lock(mutex_);
        //stop and wait for notification if condition is false;
        //continue otherwise
        cond_.wait(lock, [this] { return !queue_.empty(); });
        T e(std::move(queue_.front()));
        queue_.pop_front();
        return e;
    }
    //! Return and remove element or return passed default argument if empty
    T Pop(T&& d) {
        std::unique_lock< std::mutex > lock(mutex_);
        if(queue_.empty())
            return (d);
        T r = queue_.front();
        queue_.pop_front();
        return r;
    }
    //! Empty ?
    bool Empty() const {
        std::lock_guard< std::mutex > lg(mutex_);
        const bool e = queue_.empty();
        return e;
    }
private:
    std::deque< T > queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_;
};
}