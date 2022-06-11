//
//  math.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 17.07.2021.
//

#pragma once

#include <simd/simd.h>

namespace angry::math
{

constexpr auto radians = static_cast<float>(M_PI / 180.0);

float oriented_angle(simd_float3 x, simd_float3 y, simd_float3 ref);
float distance_between_point_and_line_segment(simd_float3 point, simd_float3 a, simd_float3 b);
float distance_between_line_segments(simd_float3 a0, simd_float3 a1, simd_float3 b0, simd_float3 b1);

simd_float3 get_world_coordinates(
    simd_float4x4 projection_matrix,
    simd_float4x4 view_matrix,
    float clip_x,
    float clip_y,
    float monster_y
);

}
