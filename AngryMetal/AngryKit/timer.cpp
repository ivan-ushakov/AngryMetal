//
//  timer.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 29.05.2021.
//

#include "timer.hpp"

using namespace angry;

void Timer::reset()
{
    _start_time.reset();
}

void Timer::update()
{
    if (!_start_time)
    {
        _start_time = std::chrono::high_resolution_clock::now();
    }

    const auto x = std::chrono::high_resolution_clock::now();
    _time_since_start = std::chrono::duration<float>(x - _start_time.value()).count();

    if (!_last_time)
    {
        _delta_time = 0;
    }
    else
    {
        _delta_time = std::chrono::duration<float>(x - _last_time.value()).count();
    }
    _last_time = x;
}

float Timer::get_time_since_start() const
{
    return _time_since_start;
}

float Timer::get_delta_time() const
{
    return _delta_time;
}
