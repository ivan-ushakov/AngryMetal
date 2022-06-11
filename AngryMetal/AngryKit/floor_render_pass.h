//
//  floor_render_pass.h
//  AngryKit
//
//  Created by  Ivan Ushakov on 22.05.2021.
//

#pragma once

#import <Metal/Metal.h>

#include <filesystem>

#include "objc_ref.h"
#include "render_pass.h"
#include "shader_common.h"

namespace angry
{

class FloorRenderPass final : public RenderPass
{
public:
    FloorRenderPass(id<MTLDevice> device, id<MTLLibrary> library);

    FloorRenderPass(const FloorRenderPass&) = delete;
    FloorRenderPass(FloorRenderPass&&) = delete;
    FloorRenderPass& operator=(const FloorRenderPass&) = delete;
    FloorRenderPass& operator=(FloorRenderPass&&) = delete;

    void set_attribute(const render::AttributeVariant& attribute) override;
    void encode(id<MTLRenderCommandEncoder> command_encoder) override;

private:
    void setup_state(id<MTLDevice> device, id<MTLLibrary> library);

private:
    objc::Ref<id<MTLRenderPipelineState>> _render_state;

    BasicUniforms _vertex_uniforms;
    CommonFragmentUniforms _fragment_uniforms;

    const float _light_factor = 0.35f;
    const float _non_blue = 0.7f;
    const simd_float3 _light_color{_light_factor * 1.0f * simd_float3{_non_blue * 0.406f, _non_blue * 0.723f, 1.0f}};
    const simd_float3 _ambient_color{_light_factor * 0.50f * simd_float3{_non_blue * 0.7f, _non_blue * 0.7f, 0.7f}};
};

}
