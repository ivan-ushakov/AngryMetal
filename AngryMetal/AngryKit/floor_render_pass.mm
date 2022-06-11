//
//  floor_render_pass.mm
//  AngryKit
//
//  Created by  Ivan Ushakov on 22.05.2021.
//

#include "floor_render_pass.h"

#import "Game.h"

#include <array>
#include <stdexcept>

#include "matrix.hpp"

using namespace angry;

FloorRenderPass::FloorRenderPass(id<MTLDevice> device, id<MTLLibrary> library)
    : RenderPass(RenderPassType::floor)
{
    _attributes = {
        render::AttributeType::model_matrix,
        render::AttributeType::projection_view_matrix,
        render::AttributeType::view_position,
        render::AttributeType::light_space_matrix
    };

    _textures = {
        MaterialTexture::diffuse,
        MaterialTexture::normal,
        MaterialTexture::specular,
        MaterialTexture::shadow
    };

    setup_state(device, library);

    _fragment_uniforms.use_light = true;
    _fragment_uniforms.use_point_light = false;
    _fragment_uniforms.use_specular = true;

    _fragment_uniforms.direction_light.direction = simd_normalize(simd_float3{-0.8f, 0.0f, -1.0f});
    _fragment_uniforms.direction_light.color = _light_color;

    _fragment_uniforms.ambient_color = _ambient_color;
}

void FloorRenderPass::set_attribute(const render::AttributeVariant& attribute)
{
    if (const auto& a = std::get_if<render::ModelMatrixAttribute>(&attribute))
    {
        _vertex_uniforms.model_matrix = a->value;
    }
    else if (const auto& a = std::get_if<render::PVMatrixAttribute>(&attribute))
    {
        _vertex_uniforms.pv = a->value;
    }
    else if (const auto& a = std::get_if<render::LightSpaceMatrixAttribute>(&attribute))
    {
        _vertex_uniforms.light_space_matrix = a->value;
    }
    else if (const auto& a = std::get_if<render::ViewPositionAttribute>(&attribute))
    {
        _fragment_uniforms.view_position = a->value;
    }
}

void FloorRenderPass::encode(id<MTLRenderCommandEncoder> command_encoder)
{
    [command_encoder setRenderPipelineState:_render_state.get()];

    [command_encoder setVertexBytes:&_vertex_uniforms length:sizeof(BasicUniforms) atIndex:2];
    [command_encoder setFragmentBytes:&_fragment_uniforms length:sizeof(CommonFragmentUniforms) atIndex:0];
}

void FloorRenderPass::setup_state(id<MTLDevice> device, id<MTLLibrary> library)
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

    objc::Ref<id<MTLFunction>> vertex_function([library newFunctionWithName:@"basic_vertex_shader"]);
    if (!vertex_function)
    {
        throw std::runtime_error("vertex function");
    }

    objc::Ref<id<MTLFunction>> fragment_function([library newFunctionWithName:@"floor_fragment_shader"]);
    if (!fragment_function)
    {
        throw std::runtime_error("fragment function");
    }

    objc::Ref<MTLRenderPipelineDescriptor*> pipeline_descriptor([MTLRenderPipelineDescriptor new]);
    pipeline_descriptor.get().vertexFunction = vertex_function.get();
    pipeline_descriptor.get().vertexDescriptor = vertex_descriptor.get();
    pipeline_descriptor.get().fragmentFunction = fragment_function.get();
    pipeline_descriptor.get().colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

    pipeline_descriptor.get().colorAttachments[0].blendingEnabled = YES;
    pipeline_descriptor.get().colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
    pipeline_descriptor.get().colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    pipeline_descriptor.get().colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    pipeline_descriptor.get().colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

    pipeline_descriptor.get().depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

    NSError* error = nil;
    _render_state = [device newRenderPipelineStateWithDescriptor:pipeline_descriptor.get() error:&error];
    if (!_render_state)
    {
        throw std::runtime_error("render pipeline state");
    }
}
