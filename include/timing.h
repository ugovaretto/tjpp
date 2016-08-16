#pragma once
//
// Created by Ugo Varetto on 8/16/16.
//
#include <chrono>
using Timer = std::chrono::steady_clock;
using Time = Timer::time_point;
using Duration = Timer::duration;
namespace {
Time Tick() { return Timer::now(); }
inline std::chrono::milliseconds toms(Duration d) {
    return std::chrono::duration_cast< std::chrono::milliseconds >(d);
}
}