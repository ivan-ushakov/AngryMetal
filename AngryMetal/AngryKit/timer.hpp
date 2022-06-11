//
//  timer.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 29.05.2021.
//

#pragma once

#include <chrono>
#include <optional>

namespace angry
{

class Timer final
{
public:
    void reset();
    void update();

    float get_time_since_start() const;
    float get_delta_time() const;

private:
    using ClockType = std::chrono::high_resolution_clock;
    using TimePoint = std::optional<std::chrono::time_point<ClockType>>;

    TimePoint _start_time;
    TimePoint _last_time;

    float _time_since_start = 0;
    float _delta_time = 0;
};

}
