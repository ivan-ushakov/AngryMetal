//
//  math.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 17.07.2021.
//

#include "math.hpp"

using namespace angry;

float math::oriented_angle(simd_float3 x, simd_float3 y, simd_float3 ref)
{
    float angle = acos(simd_clamp(simd_dot(x, y), -1.0f, 1.0f));
    return simd_mix(angle, -angle, simd_dot(ref, simd_cross(x, y)) < 0.0f);
}

float math::distance_between_point_and_line_segment(simd_float3 point, simd_float3 a, simd_float3 b)
{
    const auto ab = b - a;
    const auto ap = point - a;
    if (simd_dot(ap, ab) <= 0.0f)
    {
        return simd_length(ap);
    }

    const auto bp = point - b;
    if (simd_dot(bp, ab) >= 0.0f)
    {
        return simd_length(bp);
    }

    return simd_length(simd_cross(ab, ap)) / simd_length(ab);
}

float math::distance_between_line_segments(simd_float3 a0, simd_float3 a1, simd_float3 b0, simd_float3 b1)
{
    const float EPS = 0.001f;

    const auto A = a1 - a0;
    const auto B = b1 - b0;
    const auto magA = simd_length(A);
    const auto magB = simd_length(B);

    const auto _A = A / magA;
    const auto _B = B / magB;

    const auto cross = simd_cross(_A, _B);
    const auto cl = simd_length(cross);
    const auto denom = cl * cl;
    
    // If lines are parallel (denom=0) test if lines overlap.
    // If they don't overlap then there is a closest point solution.
    // If they do overlap, there are infinite closest positions, but there is a closest distance
    if (denom < EPS)
    {
        const float d0 = simd_dot(_A, (b0 - a0));
        const float d1 = simd_dot(_A, (b1 - a0));

        // Is segment B before A?
        if (d0 <= 0.0f && 0.0f >= d1)
        {
            if (abs(d0) < abs(d1))
            {
                return simd_length(a0 - b0);
            }
            return simd_length(a0 - b1);
        }
        else if (d0 >= magA && magA <= d1)
        {
            if (abs(d0) < abs(d1))
            {
                return simd_length(a1 - b0);
            }
            return simd_length(a1 - b1);
        }

        // Segments overlap, return distance between parallel segments
        return simd_length(((d0 * _A) + a0) - b0);
    }

    // Lines criss-cross: Calculate the projected closest points
    const auto t = (b0 - a0);
    const auto detA = simd_determinant(simd_float3x3{t, _B, cross});
    const auto detB = simd_determinant(simd_float3x3{t, _A, cross});

    const auto t0 = detA / denom;
    const auto t1 = detB / denom;

    auto pA = a0 + (_A * t0); // Projected closest point on segment A
    auto pB = b0 + (_B * t1); // Projected closest point on segment B

    // Clamp projections
    if (t0 < 0.0f)
    {
        pA = a0;
    }
    else if (t0 > magA)
    {
        pA = a1;
    }

    if (t1 < 0)
    {
        pB = b0;
    }
    else if (t1 > magB)
    {
        pB = b1;
    }

    // Clamp projection A
    if (t0 < 0.0f || t0 > magA)
    {
        float dot = simd_dot(_B, (pA - b0));
        if (dot < 0.0f)
        {
            dot = 0;
        }
        else if (dot > magB)
        {
            dot = magB;
        }
        pB = b0 + (_B * dot);
    }

    // Clamp projection B
    if (t1 < 0.0f || t1 > magB)
    {
        float dot = simd_dot(_A, (pB - a0));
        if (dot < 0.0f)
        {
            dot = 0;
        }
        else if (dot > magA)
        {
            dot = magA;
        }
        pA = a0 + (_A * dot);
    }

    return simd_length(pA - pB);
}

simd_float3 math::get_world_coordinates(
    simd_float4x4 projection_matrix,
    simd_float4x4 view_matrix,
    float clip_x,
    float clip_y,
    float monster_y
)
{
    const auto m = simd_mul(simd_inverse(view_matrix), simd_inverse(projection_matrix));

    const auto t1 = m.columns[0][3] * clip_x + m.columns[1][3] * clip_x + m.columns[3][3];
    const auto t2 = m.columns[0][1] * clip_x + m.columns[1][1] * clip_y + m.columns[3][1] - monster_y * t1;
    const float t = t2 / (m.columns[2][3] * monster_y - m.columns[2][1]);

    const float s = 1.0f / (m.columns[0][3] * clip_x + m.columns[1][3] * clip_y + m.columns[2][3] * t + m.columns[3][3]);
    const float us = clip_x * s;
    const float vs = clip_y * s;
    const float ts = t * s;

    simd_float3 r;
    r.x = m.columns[0][0] * us + m.columns[1][0] * vs + m.columns[2][0] * ts + m.columns[3][0] * s;
    r.y = 0.0f;
    r.z = m.columns[0][2] * us + m.columns[1][2] * vs + m.columns[2][2] * ts + m.columns[3][2] * s;

    return r;
}
