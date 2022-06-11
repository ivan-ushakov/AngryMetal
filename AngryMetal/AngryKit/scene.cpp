//
//  scene.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 16.05.2021.
//

#include "scene.hpp"

#include <array>
#include <sstream>
#include <stdexcept>

#include <assimp/postprocess.h>

#include "animation_component.hpp"
#include "camera_component.hpp"
#include "collider_component.hpp"
#include "health_component.hpp"
#include "input_component.hpp"
#include "instanced_mesh_component.hpp"
#include "look_component.hpp"
#include "math.hpp"
#include "mesh_component.hpp"
#include "movement_component.hpp"
#include "score_component.hpp"
#include "time_component.hpp"
#include "transform_component.hpp"

namespace angry
{

const aiScene* load_scene(Assimp::Importer& importer, const std::filesystem::path& file_path)
{
    const aiScene* scene = importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::stringstream t;
        t << "load_scene() failed to load " << file_path;
        throw std::runtime_error(t.str());
    }
    return scene;
}

}

using namespace angry;

Scene::Scene(ResourceManager* resource_manager)
    : _resource_manager(resource_manager), _enemy_pool(_registry, _max_enemy_count),
    _bullet_pool(_registry, _max_bullet_count)
{
}

void Scene::load(const std::filesystem::path& assets_path)
{
    _camera_entity = _registry.create();
    _registry.emplace<CameraComponent>(_camera_entity);

    load_floor(assets_path);
    load_player(assets_path);
    load_enemy(assets_path);
    load_bullet(assets_path);
}

entt::registry& Scene::get_registry()
{
    return _registry;
}

entt::entity Scene::get_camera() const
{
    return _camera_entity;
}

entt::entity Scene::get_player() const
{
    return _player_entity;
}

entt::entity Scene::get_gun() const
{
    return _gun_entity;
}

EntityPool& Scene::get_enemy_pool()
{
    return _enemy_pool;
}

EntityPool& Scene::get_bullet_pool()
{
    return _bullet_pool;
}

void Scene::load_player(const std::filesystem::path& assets_path)
{
    const std::filesystem::path player_path = assets_path / "Player";
    auto source_scene = load_scene(_importer1, player_path / "Player.fbx");

    {
        _player_entity = _registry.create();

        auto& animation_component = _registry.emplace<AnimationComponent>(_player_entity);
        animation_component.animation = source_scene->mAnimations[0];
        animation_component.root_node = source_scene->mRootNode;
        animation_component.global_inv = source_scene->mRootNode->mTransformation.Inverse();

        auto& mesh_component = _registry.emplace<MeshComponent>(_player_entity);
        mesh_component.source_mesh = source_scene->mMeshes[0];
        mesh_component.has_shadow = true;
        mesh_component.is_visible = true;
        mesh_component.mesh.render_pass_type = RenderPassType::player;

        const auto textures_path = player_path / "Textures";
        auto& textures = mesh_component.mesh.material.textures;
        TextureManagerInterface& texture_manager = _resource_manager->get_texture_manager();
        textures[MaterialTexture::diffuse] = texture_manager.create_texture(textures_path / "Player_D.tga");
        textures[MaterialTexture::specular] = texture_manager.create_texture(textures_path / "Player_M.tga");

        auto& movement_component = _registry.emplace<MovementComponent>(_player_entity);
        movement_component.speed = 1.5f;

        auto& transform_component = _registry.emplace<TransformComponent>(_player_entity);
        transform_component.position = simd_float3{0.0f, 0.0f, 0.0f};
        transform_component.scale = {0.0044f, 0.0044f, 0.0044f};
        transform_component.euler_angles = {0.0f, 0.0f, 0.0f};

        auto& health_component = _registry.emplace<HealthComponent>(_player_entity);
        health_component.health = 100;

        auto& look_component = _registry.emplace<LookComponent>(_player_entity);
        look_component.direction = {0.0f, 0.0f};

        _registry.emplace<InputComponent>(_player_entity);
        _registry.emplace<ScoreComponent>(_player_entity);
    }

    {
        _gun_entity = _registry.create();

        auto& mesh_component = _registry.emplace<MeshComponent>(_gun_entity);
        mesh_component.source_mesh = source_scene->mMeshes[1];
        mesh_component.has_shadow = true;
        mesh_component.is_visible = true;
        mesh_component.mesh.render_pass_type = RenderPassType::player;

        const auto textures_path = player_path / "Textures";
        auto& textures = mesh_component.mesh.material.textures;
        TextureManagerInterface& texture_manager = _resource_manager->get_texture_manager();
        textures[MaterialTexture::diffuse] = texture_manager.create_texture(textures_path / "Gun_D.tga");
        textures[MaterialTexture::specular] = texture_manager.create_texture(textures_path / "Gun_M.tga");

        auto& movement_component = _registry.emplace<MovementComponent>(_gun_entity);
        movement_component.speed = 1.5f;

        auto& transform_component = _registry.emplace<TransformComponent>(_gun_entity);
        transform_component.position = simd_float3{0.0f, 0.0f, 0.0f};
        transform_component.scale = {0.0044f, 0.0044f, 0.0044f};
        transform_component.euler_angles = {0.0f, 0.0f, 0.0f};
    }
}

void Scene::load_floor(const std::filesystem::path& assets_path)
{
    _floor_entity = _registry.create();

    auto& mesh_component = _registry.emplace<MeshComponent>(_floor_entity);
    mesh_component.is_visible = true;

    const float floor_size = 100.0f;
    const float tile_size = 1.0f;
    const float num_tile_wraps = floor_size / tile_size;
    const size_t vertex_count = 6;

    {
        std::array<float, 3 * vertex_count> position_buffer{
            -floor_size / 2, 0.0f, -floor_size / 2,
            -floor_size / 2, 0.0f, floor_size / 2,
            floor_size / 2,  0.0f, floor_size / 2,
            -floor_size / 2, 0.0f, -floor_size / 2,
            floor_size / 2,  0.0f, floor_size / 2,
            floor_size / 2,  0.0f, -floor_size / 2
        };

        auto data = reinterpret_cast<const uint8_t*>(position_buffer.data());
        BufferManagerInterface& buffer_manager = _resource_manager->get_buffer_manager();
        auto index = buffer_manager.create_buffer(data, vertex_count * 3 * sizeof(float));
        mesh_component.mesh.vertex_buffer[VertexAttribute::position] = index;
    }

    {
        std::array<float, 2 * vertex_count> uv_buffer{
            0.0f, 0.0f,
            num_tile_wraps, 0.0f,
            num_tile_wraps, num_tile_wraps,
            0.0f, 0.0f,
            num_tile_wraps, num_tile_wraps,
            0.0f, num_tile_wraps
        };

        auto data = reinterpret_cast<const uint8_t*>(uv_buffer.data());
        BufferManagerInterface& buffer_manager = _resource_manager->get_buffer_manager();
        auto index = buffer_manager.create_buffer(data, vertex_count * 2 * sizeof(float));
        mesh_component.mesh.vertex_buffer[VertexAttribute::uv] = index;
    }
    mesh_component.mesh.vertex_count = vertex_count;
    mesh_component.mesh.render_pass_type = RenderPassType::floor;

    const auto floor_path = assets_path / "Floor";
    auto& textures = mesh_component.mesh.material.textures;
    TextureManagerInterface& texture_manager = _resource_manager->get_texture_manager();
    textures[MaterialTexture::diffuse] = texture_manager.create_texture(floor_path / "Floor_D.psd");
    textures[MaterialTexture::normal] = texture_manager.create_texture(floor_path / "Floor_N.psd");
    textures[MaterialTexture::specular] = texture_manager.create_texture(floor_path / "Floor_M.psd");

    auto& transform_component = _registry.emplace<TransformComponent>(_floor_entity);
    transform_component.position = simd_float3{0.0f, 0.0f, 0.0f};
    transform_component.scale = {1.0f, 1.0f, 1.0f};
    transform_component.euler_angles = {0.0f, 45.0f * math::radians, 0.0f};
}

void Scene::load_enemy(const std::filesystem::path& assets_path)
{
    auto& instanced_mesh_manager = _resource_manager->get_instanced_mesh_manager();
    auto enemy_instanced_mesh = instanced_mesh_manager.create();

    auto& instanced_mesh = instanced_mesh_manager.get_mesh(enemy_instanced_mesh);
    instanced_mesh.mesh.render_pass_type = RenderPassType::enemy;
    auto& buffer_manager = _resource_manager->get_buffer_manager();
    const auto buffer_size = sizeof(simd_float4x4) * _max_enemy_count;
    instanced_mesh.buffers[InstanceBufferType::transform] = buffer_manager.create_buffer(buffer_size);
    instanced_mesh.buffers[InstanceBufferType::aim_rotation] = buffer_manager.create_buffer(buffer_size);
    instanced_mesh.count = 0;
    instanced_mesh.max_count = _max_enemy_count;

    const std::filesystem::path enemy_path = assets_path / "Enemy";
    auto source_scene = load_scene(_importer2, enemy_path / "Enemy.fbx");

    auto* source = source_scene->mMeshes[0];
    auto& mesh = instanced_mesh.mesh;

    const auto vertex_count = source->mNumVertices;
    const auto position_index = buffer_manager.create_buffer(vertex_count * 3 * sizeof(float));
    mesh.vertex_buffer[VertexAttribute::position] = position_index;

    const auto normal_index = buffer_manager.create_buffer(vertex_count * 3 * sizeof(float));
    mesh.vertex_buffer[VertexAttribute::normal] = normal_index;

    const auto uv_index = buffer_manager.create_buffer(vertex_count * 2 * sizeof(float));
    mesh.vertex_buffer[VertexAttribute::uv] = uv_index;

    auto position = buffer_manager.get_buffer_view<float>(position_index).data;
    auto normal = buffer_manager.get_buffer_view<float>(normal_index).data;
    auto uv = buffer_manager.get_buffer_view<float>(uv_index).data;
    for (unsigned int i = 0; i < vertex_count; i++)
    {
        position[3 * i] = source->mVertices[i].x;
        position[3 * i + 1] = source->mVertices[i].y;
        position[3 * i + 2] = source->mVertices[i].z;

        normal[3 * i] = source->mNormals[i].x;
        normal[3 * i + 1] = source->mNormals[i].y;
        normal[3 * i + 2] = source->mNormals[i].z;

        if (source->mTextureCoords[0])
        {
            uv[2 * i] = source->mTextureCoords[0][i].x;
            uv[2 * i + 1] = source->mTextureCoords[0][i].y;
        }
        else
        {
            uv[2 * i] = 0;
            uv[2 * i + 1] = 0;
        }
    }
    mesh.vertex_count = vertex_count;

    const auto index_count = 3 * source->mNumFaces;
    const auto index = buffer_manager.create_buffer(index_count * sizeof(uint32_t));
    mesh.index_buffer = index;
    mesh.index_count = index_count;

    auto view = buffer_manager.get_buffer_view<uint32_t>(index);
    size_t p = 0;
    for (unsigned int i = 0; i < source->mNumFaces; i++)
    {
        aiFace& face = source->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            const unsigned int vertex_index = face.mIndices[j];
            view.data[p++] = vertex_index;
        }
    }

    const auto textures_path = enemy_path / "Textures";
    auto& textures = mesh.material.textures;
    TextureManagerInterface& texture_manager = _resource_manager->get_texture_manager();
    textures[MaterialTexture::diffuse] = texture_manager.create_texture(textures_path / "Enemy_D.png");

    for (int i = 0; i < _max_enemy_count; i++)
    {
        auto entity = _enemy_pool.get_idle()[i];

        auto& transform_component = _registry.emplace<TransformComponent>(entity);
        transform_component.position = {0.0f, 0.0f, 0.0f};
        transform_component.scale = {0.01f, 0.01f, 0.01f};
        transform_component.euler_angles = {90.0f * math::radians, 0.0f, 180.0f * math::radians};

        auto& instanced_mesh_component = _registry.emplace<InstancedMeshComponent>(entity);
        instanced_mesh_component.instanced_mesh = enemy_instanced_mesh;
        instanced_mesh_component.is_visible = false;

        auto& collider_component = _registry.emplace<ColliderComponent>(entity);
        collider_component.capsule = {0.4f, 0.08f};

        _registry.emplace<MovementComponent>(entity);
    }
}

void Scene::load_bullet(const std::filesystem::path& assets_path)
{
    auto& instanced_mesh_manager = _resource_manager->get_instanced_mesh_manager();
    auto mesh_index = instanced_mesh_manager.create();
    auto& instanced_mesh = instanced_mesh_manager.get_mesh(mesh_index);

    const auto buffer_size = sizeof(simd_float4x4) * _max_bullet_count;
    BufferManagerInterface& buffer_manager = _resource_manager->get_buffer_manager();
    instanced_mesh.buffers[InstanceBufferType::transform] = buffer_manager.create_buffer(buffer_size);
    instanced_mesh.count = 0;
    instanced_mesh.max_count = _max_bullet_count;

    const float bullet_scale = 0.3f;
    const size_t vertex_count = 4;

    {
        std::array<float, 3 * vertex_count> buffer{
            bullet_scale * (-0.243f), 0.0f, bullet_scale * (-0.5f),
            bullet_scale * (-0.243f), 0.0f, bullet_scale * 0.5f,
            bullet_scale * 0.243f,  0.0f, bullet_scale * 0.5f,
            bullet_scale * 0.243f, 0.0f, bullet_scale * (-0.5f)
        };

        auto data = reinterpret_cast<const uint8_t*>(buffer.data());
        auto index = buffer_manager.create_buffer(data, buffer.size() * sizeof(float));
        instanced_mesh.mesh.vertex_buffer[VertexAttribute::position] = index;
    }

    {
        std::array<float, 2 * vertex_count> buffer{
            1.0f, 0.0f,
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f
        };

        auto data = reinterpret_cast<const uint8_t*>(buffer.data());
        auto index = buffer_manager.create_buffer(data, buffer.size() * sizeof(float));
        instanced_mesh.mesh.vertex_buffer[VertexAttribute::uv] = index;
    }

    {
        std::array<uint32_t, 6> buffer{
            0, 1, 2,
            0, 2, 3
        };

        auto data = reinterpret_cast<const uint8_t*>(buffer.data());
        auto index = buffer_manager.create_buffer(data, buffer.size() * sizeof(uint32_t));
        instanced_mesh.mesh.index_buffer = index;
        instanced_mesh.mesh.index_count = buffer.size();
    }

    instanced_mesh.mesh.vertex_count = vertex_count;
    instanced_mesh.mesh.render_pass_type = RenderPassType::bullet;

    const std::filesystem::path bullet_path = assets_path / "Bullet";
    auto& mesh = instanced_mesh.mesh;
    auto& textures = mesh.material.textures;
    TextureManagerInterface& texture_manager = _resource_manager->get_texture_manager();
    textures[MaterialTexture::diffuse] = texture_manager.create_texture(bullet_path / "Bullet_D.png");

    for (int i = 0; i < _max_bullet_count; i++)
    {
        auto entity = _bullet_pool.get_idle()[i];

        auto& transform_component = _registry.emplace<TransformComponent>(entity);
        transform_component.position = {0.0f, 0.0f, 0.0f};
        transform_component.scale = {1.0f, 1.0f, 1.0f};
        transform_component.euler_angles = {0.0f, 0.0f, 0.0f};

        auto& instanced_mesh_component = _registry.emplace<InstancedMeshComponent>(entity);
        instanced_mesh_component.instanced_mesh = mesh_index;
        instanced_mesh_component.is_visible = false;

        auto& collider_component = _registry.emplace<ColliderComponent>(entity);
        collider_component.capsule = {0.3f, 0.03f};

        _registry.emplace<TimeComponent>(entity);
        _registry.emplace<MovementComponent>(entity);
    }
}
