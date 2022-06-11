//
//  transform_component.hpp
//  AngryMetal
//
//  Created by  Ivan Ushakov on 16.06.2021.
//

#pragma once

#include <simd/simd.h>

#include "matrix.hpp"

namespace angry
{

struct TransformComponent
{
    simd_float3 position;
    simd_float3 scale;
    simd_float3 euler_angles;
    
    simd_float4x4 get_matrix() const
    {
        auto t = simd_mul(matrix_identity_float4x4, matrix::make_transform(position));
        auto s = simd_mul(t, matrix::make_scale(scale));
        auto r1 = simd_mul(s, matrix::make_rotation({0.0f, 1.0f, 0.0f}, euler_angles.y));
        auto r2 = simd_mul(r1, matrix::make_rotation({0.0f, 0.0f, 1.0f}, euler_angles.z));
        return simd_mul(r2, matrix::make_rotation({1.0f, 0.0f, 0.0f}, euler_angles.x));
    }

    simd_float4x4 get_rotation_matrix() const
    {
        auto r1 = simd_mul(matrix_identity_float4x4, matrix::make_rotation({0.0f, 1.0f, 0.0f}, euler_angles.y));
        auto r2 = simd_mul(r1, matrix::make_rotation({0.0f, 0.0f, 1.0f}, euler_angles.z));
        return simd_mul(r2, matrix::make_rotation({1.0f, 0.0f, 0.0f}, euler_angles.x));
    }
};

}
