//
//  shader_common.h
//  AngryKit
//
//  Created by  Ivan Ushakov on 22.05.2021.
//

#ifndef shader_common_h
#define shader_common_h

#include <simd/simd.h>

using namespace simd;

struct DepthOnlyUniforms
{
    float4x4 model_matrix;
    float4x4 light_space_matrix;
};

struct BasicUniforms
{
    float4x4 model_matrix;
    float4x4 pv;
    float4x4 light_space_matrix;
};

struct Uniforms
{
    float4x4 model_matrix;
    float4x4 pv;
    float4x4 aim_rotation;
    float4x4 light_space_matrix;
};

struct EnemyUniforms
{
    float4x4 pv;
    float4x4 light_space_matrix;

    float3 nose_position;
    float time;
};

struct EnemyInstance
{
    float4x4 model_matrix;
    float4x4 aim_rotation;
};

struct BulletUniforms
{
    float4x4 pv;
};

struct BulletInstance
{
    float4x4 model_matrix;
};

struct DirectionLight
{
    float3 direction;
    float3 color;
};

struct PointLight
{
    float3 world_position;
    float3 color;
};

struct CommonFragmentUniforms
{
    DirectionLight direction_light;
    PointLight point_light;
    bool use_point_light;
    bool use_light;
    bool use_specular;
    float3 ambient_color;
    float3 view_position;
};

#endif /* shader_common_h */
