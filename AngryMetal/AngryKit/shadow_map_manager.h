//
//  shadow_map_manager.h
//  AngryKit
//
//  Created by  Ivan Ushakov on 07.06.2021.
//

#pragma once

#import <Metal/Metal.h>

#include <simd/simd.h>

#include "objc_ref.h"
#include "shader_common.h"

namespace angry
{

class BufferManager;
class Scene;

class ShadowMapManager final
{
public:
    ShadowMapManager(BufferManager* buffer_manager, id<MTLDevice> device, id<MTLLibrary> library);
    ~ShadowMapManager() = default;

    ShadowMapManager(const ShadowMapManager&&) = delete;
    ShadowMapManager(ShadowMapManager&&) = delete;
    ShadowMapManager& operator=(const ShadowMapManager&) = delete;
    ShadowMapManager& operator=(ShadowMapManager&&) = delete;

    void update(Scene& scene, id<MTLCommandBuffer> command_buffer);
    simd_float4x4 get_light_space_matrix() const;
    id<MTLTexture> get_shadow_map() const;

private:
    BufferManager* _buffer_manager;

    objc::Ref<id<MTLRenderPipelineState>> _render_state;
    objc::Ref<id<MTLDepthStencilState>> _depth_state;
    objc::Ref<id<MTLTexture>> _texture;
    objc::Ref<MTLRenderPassDescriptor*> _render_pass_descriptor;

    DepthOnlyUniforms _vertex_uniforms;
    simd_float4x4 _light_projection_matrix;
    simd_float4x4 _light_space_matrix;
};

}
