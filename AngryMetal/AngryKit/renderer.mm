//
//  renderer.mm
//  AngryKit
//
//  Created by  Ivan Ushakov on 22.05.2021.
//

#include "renderer.h"

#import "Game.h"

#include <stdexcept>

#include "bullet_render_pass.h"
#include "camera_component.hpp"
#include "enemy_render_pass.h"
#include "floor_render_pass.h"
#include "instanced_mesh_component.hpp"
#include "matrix.hpp"
#include "mesh_component.hpp"
#include "movement_component.hpp"
#include "player_render_pass.h"
#include "scene.hpp"
#include "shadow_map_manager.h"
#include "transform_component.hpp"

using namespace angry;

using RenderPassPtr = std::unique_ptr<RenderPass>;

Renderer::Renderer(BufferManager* buffer_manager,
                   InstancedMeshManager* instanced_mesh_manager,
                   TextureManager* texture_mananger)
    : _buffer_manager(buffer_manager),
    _instanced_mesh_manager(instanced_mesh_manager),
    _texture_manager(texture_mananger)
{
}

Renderer::~Renderer()
{
}

void Renderer::setup(id<MTLDevice> device)
{
    setup_depth_state(device);

    NSBundle* bundle = [NSBundle bundleForClass:[Game class]];
    objc::Ref<id<MTLLibrary>> library([device newDefaultLibraryWithBundle:bundle error:nil]);
    if (!library)
    {
        throw std::runtime_error("library");
    }

    _passes.emplace_back(std::make_unique<FloorRenderPass>(device, library.get()));
    _passes.emplace_back(std::make_unique<PlayerRenderPass>(device, library.get()));
    _passes.emplace_back(std::make_unique<EnemyRenderPass>(device, library.get()));
    _passes.emplace_back(std::make_unique<BulletRenderPass>(device, library.get()));

    _shadow_map_manager = std::make_unique<ShadowMapManager>(_buffer_manager, device, library.get());
}

void Renderer::draw(MetalContext& context, Scene& scene, const Timer& timer)
{
    _shadow_map_manager->update(scene, context.command_buffer);
    auto light_space_matrix = _shadow_map_manager->get_light_space_matrix();
    auto shadow_map = _shadow_map_manager->get_shadow_map();

    auto command_encoder = context.get_render_encoder();
    [command_encoder pushDebugGroup:@"Scene rendering"];

    [command_encoder setDepthStencilState:_depth_state.get()];
    [command_encoder setFrontFacingWinding:MTLWindingCounterClockwise];
    [command_encoder setCullMode:MTLCullModeBack];

    auto& camera_component = scene.get_registry().get<CameraComponent>(scene.get_camera());
    auto f = [&](const Mesh& mesh, entt::entity* entity)
    {
        auto* render_pass = find_render_pass(mesh.render_pass_type);
        if (render_pass == nullptr)
        {
            return;
        }

        for (const auto& type : render_pass->get_attributes())
        {
            switch (type)
            {
                case render::AttributeType::model_matrix:
                {
                    const auto& transform_component = scene.get_registry().get<TransformComponent>(*entity);
                    auto m = transform_component.get_matrix();
                    render_pass->set_attribute(render::ModelMatrixAttribute(m));
                    break;
                }

                case render::AttributeType::projection_view_matrix:
                {
                    auto m = camera_component.projection_view_matrix;
                    render_pass->set_attribute(render::PVMatrixAttribute(m));
                    break;
                }

                case render::AttributeType::view_position:
                {
                    render_pass->set_attribute(render::ViewPositionAttribute(camera_component.position));
                    break;
                }

                case render::AttributeType::light_space_matrix:
                {
                    render_pass->set_attribute(render::LightSpaceMatrixAttribute(light_space_matrix));
                    break;
                }

                case render::AttributeType::aim_rotation:
                {
                    const auto& transform_component = scene.get_registry().get<TransformComponent>(*entity);
                    auto m = transform_component.get_rotation_matrix();
                    render_pass->set_attribute(render::AimRotationMatrixAttribute(m));
                    break;
                }

                case render::AttributeType::time:
                {
                    render_pass->set_attribute(render::TimeAttribute(timer.get_time_since_start()));
                    break;
                }
            }
        }
        render_pass->encode(command_encoder);

        const auto& material = mesh.material;
        NSUInteger texture_index = 0;
        for (auto t : render_pass->get_textures())
        {
            if (t == MaterialTexture::shadow)
            {
                [command_encoder setFragmentTexture:shadow_map atIndex:texture_index];
            }
            else
            {
                auto index = material.textures.at(t);
                id<MTLTexture> texture = _texture_manager->get_texture(index);
                [command_encoder setFragmentTexture:texture atIndex:texture_index];
            }
            texture_index += 1;
        }

        std::array<VertexAttribute, 3> attributes = {
            VertexAttribute::position, VertexAttribute::normal, VertexAttribute::uv
        };
        NSUInteger attribute_index = 0;
        for (const auto& attribute : attributes)
        {
            auto p = mesh.vertex_buffer.find(attribute);
            if (p != mesh.vertex_buffer.end())
            {
                id<MTLBuffer> buffer = _buffer_manager->get_buffer(p->second);
                [command_encoder setVertexBuffer:buffer offset:0 atIndex:attribute_index];
                attribute_index += 1;
            }
        }
    };

    // mesh rendering
    auto v1 = scene.get_registry().view<MeshComponent>();
    for (auto entity : v1)
    {
        const auto& mesh_component = v1.get<MeshComponent>(entity);
        if (!mesh_component.is_visible)
        {
            continue;
        }

        const auto& mesh = mesh_component.mesh;
        f(mesh, &entity);

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

    // instanced mesh prepare
    auto v2 = scene.get_registry().view<InstancedMeshComponent>();
    for (auto entity : v2)
    {
        const auto& mesh_component = v2.get<InstancedMeshComponent>(entity);
        if (!mesh_component.is_visible)
        {
            continue;
        }

        auto& instanced_mesh = _instanced_mesh_manager->get_mesh(mesh_component.instanced_mesh);

        const auto& transform_component = scene.get_registry().get<TransformComponent>(entity);
        for (auto entry : instanced_mesh.buffers)
        {
            switch (entry.first)
            {
                case InstanceBufferType::transform:
                {
                    auto buffer = _buffer_manager->get_buffer_view<simd_float4x4>(entry.second);
                    buffer.data[instanced_mesh.count] = transform_component.get_matrix();
                    break;
                }

                case InstanceBufferType::aim_rotation:
                {
                    auto buffer = _buffer_manager->get_buffer_view<simd_float4x4>(entry.second);
                    buffer.data[instanced_mesh.count] = transform_component.get_rotation_matrix();
                    break;
                }

                default:
                    break;
            }
            
        }

        instanced_mesh.count += 1;
    }

    // instanced mesh rendering
    for (auto& instanced_mesh : _instanced_mesh_manager->get_all())
    {
        if (instanced_mesh->count == 0)
        {
            continue;
        }

        const auto& mesh = instanced_mesh->mesh;
        f(mesh, nullptr);

        auto* render_pass = find_render_pass(mesh.render_pass_type);
        if (render_pass == nullptr)
        {
            continue;
        }

        for (auto entry : instanced_mesh->buffers)
        {
            auto buffer_index = instanced_mesh->buffers[entry.first];
            auto buffer = _buffer_manager->get_buffer(buffer_index);
            auto attribute_index = render_pass->get_attribute_index(entry.first);
            [command_encoder setVertexBuffer:buffer offset:0 atIndex:attribute_index];
        }

        if (mesh.index_count > 0)
        {
            id<MTLBuffer> index_buffer = _buffer_manager->get_buffer(mesh.index_buffer);
            [command_encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                        indexCount:mesh.index_count
                                         indexType:MTLIndexTypeUInt32
                                       indexBuffer:index_buffer
                                 indexBufferOffset:0
                                     instanceCount:instanced_mesh->count];
        }
        else
        {
            [command_encoder drawPrimitives:MTLPrimitiveTypeTriangle
                                vertexStart:0
                                vertexCount:mesh.vertex_count
                              instanceCount:instanced_mesh->count];
        }

        instanced_mesh->count = 0;
    }

    [command_encoder popDebugGroup];
}

void Renderer::setup_depth_state(id<MTLDevice> device)
{
    objc::Ref<MTLDepthStencilDescriptor*> descriptor([MTLDepthStencilDescriptor new]);
    descriptor.get().depthWriteEnabled = YES;
    descriptor.get().depthCompareFunction = MTLCompareFunctionLess;

    _depth_state = [device newDepthStencilStateWithDescriptor:descriptor.get()];
    if (!_depth_state)
    {
        throw std::runtime_error("setup_depth_state");
    }
}

RenderPass* Renderer::find_render_pass(RenderPassType type) const
{
    for (auto& e : _passes)
    {
        if (e->get_type() == type)
        {
            return e.get();
        }
    }
    return nullptr;
}
