//
//  render_pass_attribute.h
//  AngryMetal
//
//  Created by  Ivan Ushakov on 25.05.2021.
//

#pragma once

#include <variant>

#include <simd/simd.h>

namespace angry::render
{

enum class AttributeType
{
    model_matrix,
    projection_view_matrix,
    view_position,
    aim_rotation,
    light_space_matrix,
    time
};

template<class T, AttributeType type>
struct Attribute
{
    const AttributeType Type = type;
    T value;

    explicit Attribute(T value) : value(value) {}
};

using ModelMatrixAttribute = Attribute<simd_float4x4, AttributeType::model_matrix>;
using PVMatrixAttribute = Attribute<simd_float4x4, AttributeType::projection_view_matrix>;
using ViewPositionAttribute = Attribute<simd_float3, AttributeType::view_position>;
using AimRotationMatrixAttribute = Attribute<simd_float4x4, AttributeType::aim_rotation>;
using LightSpaceMatrixAttribute = Attribute<simd_float4x4, AttributeType::light_space_matrix>;
using TimeAttribute = Attribute<float, AttributeType::time>;

using AttributeVariant = std::variant<
    ModelMatrixAttribute,
    PVMatrixAttribute,
    ViewPositionAttribute,
    AimRotationMatrixAttribute,
    LightSpaceMatrixAttribute,
    TimeAttribute>;

}
