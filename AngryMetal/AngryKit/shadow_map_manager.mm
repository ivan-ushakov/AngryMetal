//
//  shadow_map_manager.mm
//  AngryKit
//
//  Created by  Ivan Ushakov on 07.06.2021.
//

#include "shadow_map_manager.h"

#include "buffer_manager.h"
#include "matrix.hpp"
#include "mesh_component.hpp"
#include "scene.hpp"
#include "transform_component.hpp"

using namespace angry;

ShadowMapManager::ShadowMapManager(BufferManager* buffer_manager, id<MTLDevice> device, id<MTLLibrary> library) 
    : _buffer_manager(buffer_manager)
{
    objc::Ref<MTLVertexDescriptor*> vertex_descriptor([MTLVertexDescriptor new]);
    vertex_descriptor.get().attributes[0].format = MTLVertexFormatFloat3;
    vertex_descriptor.get().attributes[0].bufferIndex = 0;
    vertex_descriptor.get().attributes[0].offset = 0;

    vertex_descriptor.get().layouts[0].stride = 12;
    vertex_descriptor.get().layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

    objc::Ref<id<MTLFunction>> vertex_function([library newFunctionWithName:@"depth_only_vertex_shader"]);
    if (!vertex_function)
    {
        throw std::runtime_error("vertex function");
    }

    objc::Ref<id<MTLFunction>> fragment_function([library newFunctionWithName:@"dummy_fragment_shader"]);
    if (!fragment_function)
    {
        throw std::runtime_error("fragment function");
    }

    objc::Ref<MTLRenderPipelineDescriptor*> pipeline_descriptor([MTLRenderPipelineDescriptor new]);
    pipeline_descriptor.get().label = @"DepthOnlyPipelineState";
    pipeline_descriptor.get().sampleCount = 1;
    pipeline_descriptor.get().vertexFunction = vertex_function.get();
    pipeline_descriptor.get().vertexDescriptor = vertex_descriptor.get();
    pipeline_descriptor.get().fragmentFunction = fragment_function.get();
    pipeline_descriptor.get().depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

    NSError* error = nil;
    _render_state = [device newRenderPipelineStateWithDescriptor:pipeline_descriptor.get() error:&error];
    if (!_render_state)
    {
        throw std::runtime_error("render pipeline state");
    }
    
    objc::Ref<MTLDepthStencilDescriptor*> descriptor([MTLDepthStencilDescriptor new]);
    descriptor.get().depthWriteEnabled = YES;
    descriptor.get().depthCompareFunction = MTLCompareFunctionLess;

    _depth_state = [device newDepthStencilStateWithDescriptor:descriptor.get()];
    if (!_depth_state)
    {
        throw std::runtime_error("setup_depth_state");
    }

    NSUInteger shadow_map_size = 1024;
    MTLTextureDescriptor* texture_descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                                  width:shadow_map_size
                                                                                                 height:shadow_map_size
                                                                                              mipmapped:NO];
    texture_descriptor.textureType = MTLTextureType2D;
    texture_descriptor.storageMode = MTLStorageModePrivate;
    texture_descriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;

    _texture = [device newTextureWithDescriptor:texture_descriptor];
    _texture.get().label = @"ShadowMap";

    _render_pass_descriptor = [[MTLRenderPassDescriptor renderPassDescriptor] retain];
    _render_pass_descriptor.get().depthAttachment.texture = _texture.get();
    _render_pass_descriptor.get().depthAttachment.slice = 0;
    _render_pass_descriptor.get().depthAttachment.clearDepth = 1.0f;
    _render_pass_descriptor.get().depthAttachment.loadAction = MTLLoadActionClear;
    _render_pass_descriptor.get().depthAttachment.storeAction = MTLStoreActionStore;

    const float near_plane = 1.0f;
    const float far_plane = 50.0f;
    const float ortho_size = 10.0f;
    _light_projection_matrix = matrix::make_ortho_projection(-ortho_size, ortho_size, -ortho_size, ortho_size, near_plane, far_plane);
}

void ShadowMapManager::update(Scene& scene, id<MTLCommandBuffer> command_buffer)
{
    auto& player_transform_component = scene.get_registry().get<TransformComponent>(scene.get_player());
    const simd_float3 player_light_direction = simd_normalize(simd_float3{-1.0f, -1.0f, -1.0f});
    simd_float3 eye = player_transform_component.position - 20.0f * player_light_direction;
    simd_float3 target = player_transform_component.position;
    const auto light_view_matrix = matrix::make_look_at(eye, target, simd_float3{0.0f, 1.0f, 0.0f});
    _light_space_matrix = simd_mul(_light_projection_matrix, light_view_matrix);

    id<MTLRenderCommandEncoder> command_encoder = [command_buffer renderCommandEncoderWithDescriptor:_render_pass_descriptor.get()];
    command_encoder.label = @"ShadowPass";
    [command_encoder setDepthBias:0.0f slopeScale:2.0f clamp:0.0f];

    [command_encoder setDepthStencilState:_depth_state.get()];
    [command_encoder setFrontFacingWinding:MTLWindingCounterClockwise];
    [command_encoder setCullMode:MTLCullModeBack];

    [command_encoder setRenderPipelineState:_render_state.get()];

    auto entity_view = scene.get_registry().view<MeshComponent>();
    for (auto entity : entity_view)
    {
        auto& mesh_component = entity_view.get<MeshComponent>(entity);
        if (!mesh_component.has_shadow)
        {
            continue;
        }

        auto& transform_component = scene.get_registry().get<TransformComponent>(entity);
        _vertex_uniforms.light_space_matrix = _light_space_matrix;
        _vertex_uniforms.model_matrix = transform_component.get_matrix();
        [command_encoder setVertexBytes:&_vertex_uniforms length:sizeof(DepthOnlyUniforms) atIndex:1];

        auto& mesh = mesh_component.mesh;
        auto p = mesh.vertex_buffer.find(VertexAttribute::position);
        if (p != mesh.vertex_buffer.end())
        {
            id<MTLBuffer> buffer = _buffer_manager->get_buffer(p->second);
            [command_encoder setVertexBuffer:buffer offset:0 atIndex:0];
        }

        if (mesh.index_count > 0)
        {
            id<MTLBuffer> index_buffer = _buffer_manager->get_buffer(mesh.index_buffer);
            [command_encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                        indexCount:mesh.index_count
                                         indexType:MTLIndexTypeUInt32
                                       indexBuffer:index_buffer
                                 indexBufferOffset:0];
        }
        else
        {
            [command_encoder drawPrimitives:MTLPrimitiveTypeTriangle
                                vertexStart:0
                                vertexCount:mesh.vertex_count];
        }
    }

    [command_encoder endEncoding];
}

simd_float4x4 ShadowMapManager::get_light_space_matrix() const
{
    return _light_space_matrix;
}

id<MTLTexture> ShadowMapManager::get_shadow_map() const
{
    return _texture.get();
}
