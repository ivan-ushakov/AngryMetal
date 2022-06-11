//
//  scene.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 16.05.2021.
//

#pragma once

#include <simd/simd.h>

#include <filesystem>
#include <memory>
#include <vector>

#include <assimp/Importer.hpp>
#include <entt/entt.hpp>

#include "entity_pool.hpp"
#include "resource_manager.hpp"

namespace angry
{

class Scene final
{
public:
    explicit Scene(ResourceManager* resource_manager);
    ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene(Scene&&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) = delete;

    void load(const std::filesystem::path& assets_path);
    entt::registry& get_registry();

    entt::entity get_camera() const;
    entt::entity get_player() const;
    entt::entity get_gun() const;

    EntityPool& get_enemy_pool();
    EntityPool& get_bullet_pool();

private:
    void load_floor(const std::filesystem::path& assets_path);
    void load_player(const std::filesystem::path& assets_path);
    void load_enemy(const std::filesystem::path& assets_path);
    void load_bullet(const std::filesystem::path& assets_path);

private:
    const int _max_enemy_count = 16;
    const int _max_bullet_count = 16;

    ResourceManager* _resource_manager;

    Assimp::Importer _importer1;
    Assimp::Importer _importer2;

    entt::registry _registry;
    entt::entity _camera_entity;
    entt::entity _floor_entity;
    entt::entity _player_entity;
    entt::entity _gun_entity;

    EntityPool _enemy_pool;
    EntityPool _bullet_pool;
};

}
