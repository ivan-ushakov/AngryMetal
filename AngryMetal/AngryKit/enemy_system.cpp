//
//  enemy_system.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 14.06.2021.
//

#include "enemy_system.hpp"

#include "collider_component.hpp"
#include "health_component.hpp"
#include "instanced_mesh_component.hpp"
#include "math.hpp"
#include "movement_component.hpp"
#include "scene.hpp"
#include "transform_component.hpp"

using namespace angry;

EnemySystem::EnemySystem()
    : _countdown(_spawns_per_interval)
{
}

void EnemySystem::update(Scene& scene, float delta_time)
{
    auto& player_health_component = scene.get_registry().get<HealthComponent>(scene.get_player());
    if (player_health_component.health == 0)
    {
        return;
    }

    auto& enemy_pool = scene.get_enemy_pool();

    auto& player_transform_component = scene.get_registry().get<TransformComponent>(scene.get_player());
    const auto player_position = player_transform_component.position;

    const float player_model_gun_height = 120.0f;
    float monster_y = player_transform_component.scale.x * player_model_gun_height;

    _countdown -= delta_time;
    if (_countdown <= 0.0f)
    {
        for (int i = 0; i < _spawns_per_interval; i++)
        {
            auto entity = enemy_pool.get_entity();
            if (!entity)
            {
                break;
            }

            const float theta = rand() % 360 * math::radians;
            const float x = player_position.x + sin(theta) * _spawn_radius;
            const float z = player_position.z + cos(theta) * _spawn_radius;

            auto& transform_component = scene.get_registry().get<TransformComponent>(*entity);
            transform_component.position = {x, monster_y, z};
            
            auto& mesh_component = scene.get_registry().get<InstancedMeshComponent>(*entity);
            mesh_component.is_visible = true;
        }
        _countdown += _spawn_interval;
    }

    const simd_float3 player_collision_position{player_position.x, monster_y, player_position.z};

    auto r = enemy_pool.find_first_if([&](auto enemy_entity) {
        auto& transform_component = scene.get_registry().get<TransformComponent>(enemy_entity);
        auto direction = player_transform_component.position - transform_component.position;
        direction.y = 0.0f;
        direction = simd_normalize(direction);
        transform_component.position += direction * _monster_speed * delta_time;

        auto& movement_component = scene.get_registry().get<MovementComponent>(enemy_entity);
        movement_component.direction = direction.xz;

        float theta = atanf(direction.x / direction.z) + (direction.z < 0.0f ? 0.0f : 180.0f * math::radians);
        transform_component.euler_angles.y = theta;

        // collision
        const auto& collider_component = scene.get_registry().get<ColliderComponent>(enemy_entity);
        const auto& capsule = collider_component.capsule;

        const auto enemy_position = transform_component.position;
        const auto p1 = enemy_position - direction * (capsule.height / 2.0f);
        const auto p2 = enemy_position + direction * (capsule.height / 2.0f);
        const float d = math::distance_between_point_and_line_segment(player_collision_position, p1, p2);
        return d <= (_player_collision_radius + capsule.radius);
    });

    if (r)
    {
        player_health_component.health = 0;
    }
}
