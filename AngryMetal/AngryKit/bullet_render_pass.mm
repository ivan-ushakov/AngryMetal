//
//  bullet_render_pass.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 16.07.2021.
//

#include "bullet_render_pass.h"

#include <array>
#include <stdexcept>

using namespace angry;

BulletRenderPass::BulletRenderPass(id<MTLDevice> device, id<MTLLibrary> library)
    : RenderPass(RenderPassType::bullet)
{
    _attributes = {
        render::AttributeType::projection_view_matrix
    };

    _textures = {
        MaterialTexture::diffuse
    };

    _instance_buffer_attributes = {
        {InstanceBufferType::transform, 3}
    };

    setup_state(device, library);
}

void BulletRenderPass::set_attribute(const render::AttributeVariant& attribute)
{
    if (const auto& a = std::get_if<render::PVMatrixAttribute>(&attribute))
    {
        _vertex_uniforms.pv = a->value;
    }
}

void BulletRenderPass::encode(id<MTLRenderCommandEncoder> command_encoder)
{
    [command_encoder setRenderPipelineState:_render_state.get()];

    [command_encoder setVertexBytes:&_vertex_uniforms length:sizeof(BulletUniforms) atIndex:2];
}

void BulletRenderPass::setup_state(id<MTLDevice> device, id<MTLLibrary> library)
{
    objc::Ref<MTLVertexDescriptor*> vertex_descriptor([MTLVertexDescriptor new]);
    vertex_descriptor.get().attributes[0].format = MTLVertexFormatFloat3;
    vertex_descriptor.get().attributes[0].bufferIndex = 0;
    vertex_descriptor.get().attributes[0].offset = 0;

    vertex_descriptor.get().attributes[1].format = MTLVertexFormatFloat2;
    vertex_descriptor.get().attributes[1].bufferIndex = 1;
    vertex_descriptor.get().attributes[1].offset = 0;

    vertex_descriptor.get().layouts[0].stride = 12;
    vertex_descriptor.get().layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

    vertex_descriptor.get().layouts[1].stride = 8;
    vertex_descriptor.get().layouts[1].stepFunction = MTLVertexStepFunctionPerVertex;

    objc::Ref<id<MTLFunction>> vertex_function([library newFunctionWithName:@"bullet_vertex_shader"]);
    if (!vertex_function)
    {
        throw std::runtime_error("vertex function");
    }

    objc::Ref<id<MTLFunction>> fragment_function([library newFunctionWithName:@"bullet_fragment_shader"]);
    if (!fragment_function)
    {
        throw std::runtime_error("fragment function");
    }

    objc::Ref<MTLRenderPipelineDescriptor*> pipeline_descriptor([MTLRenderPipelineDescriptor new]);
    pipeline_descriptor.get().vertexFunction = vertex_function.get();
    pipeline_descriptor.get().vertexDescriptor = vertex_descriptor.get();
    pipeline_descriptor.get().fragmentFunction = fragment_function.get();
    pipeline_descriptor.get().colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    pipeline_descriptor.get().depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

    pipeline_descriptor.get().colorAttachments[0].blendingEnabled = YES;
    pipeline_descriptor.get().colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
    pipeline_descriptor.get().colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    pipeline_descriptor.get().colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    pipeline_descriptor.get().colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

    NSError* error = nil;
    _render_state = [device newRenderPipelineStateWithDescriptor:pipeline_descriptor.get() error:&error];
    if (!_render_state)
    {
        throw std::runtime_error("render pipeline state");
    }
}
