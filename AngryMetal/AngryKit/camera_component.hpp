//
//  camera_component.hpp
//  AngryMetal
//
//  Created by  Ivan Ushakov on 06.06.2021.
//

#pragma once

#include <simd/simd.h>

namespace angry
{

struct CameraComponent
{
    float aspect = 0;

    simd_float3 position;

    simd_float4x4 projection_matrix;
    simd_float4x4 view_matrix;
    simd_float4x4 projection_view_matrix;
};

}
