//
//  bullet_render_pass.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 16.07.2021.
//

#pragma once

#import <Metal/Metal.h>

#include "objc_ref.h"
#include "render_pass.h"
#include "shader_common.h"

namespace angry
{

class BulletRenderPass final : public RenderPass
{
public:
    BulletRenderPass(id<MTLDevice> device, id<MTLLibrary> library);

    BulletRenderPass(const BulletRenderPass&) = delete;
    BulletRenderPass(BulletRenderPass&&) = delete;
    BulletRenderPass& operator=(const BulletRenderPass&) = delete;
    BulletRenderPass& operator=(BulletRenderPass&&) = delete;

    void set_attribute(const render::AttributeVariant& attribute) override;
    void encode(id<MTLRenderCommandEncoder> command_encoder) override;

private:
    void setup_state(id<MTLDevice> device, id<MTLLibrary> library);

private:
    objc::Ref<id<MTLRenderPipelineState>> _render_state;

    BulletUniforms _vertex_uniforms;
};

}
