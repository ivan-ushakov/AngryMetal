//
//  input_component.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 04.02.2022.
//

#pragma once

#include <simd/simd.h>

namespace angry
{

struct InputComponent
{
    simd_float2 direction{0.0f, 0.0f};
    float theta = 0.0f;
    bool is_moving = false;
    bool is_shooting = false;
    bool is_restarting = false;
};

}
