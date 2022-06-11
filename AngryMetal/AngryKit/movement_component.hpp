//
//  movement_component.hpp
//  AngryMetal
//
//  Created by  Ivan Ushakov on 25.05.2021.
//

#pragma once

#include <simd/simd.h>

namespace angry
{

struct MovementComponent
{
    float speed;
    simd_float2 direction;
};

}
