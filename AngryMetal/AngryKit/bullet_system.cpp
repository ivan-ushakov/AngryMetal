//
//  bullet_system.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 07.07.2021.
//

#include "bullet_system.hpp"

#include "collider_component.hpp"
#include "health_component.hpp"
#include "instanced_mesh_component.hpp"
#include "look_component.hpp"
#include "math.hpp"
#include "movement_component.hpp"
#include "scene.hpp"
#include "score_component.hpp"
#include "timer.hpp"
#include "time_component.hpp"
#include "transform_component.hpp"

using namespace angry;

void BulletSystem::update(Scene& scene, const Timer& timer)
{
    auto& player_health_component = scene.get_registry().get<HealthComponent>(scene.get_player());
    if (player_health_component.health == 0)
    {
        return;
    }

    auto& bullet_pool = scene.get_bullet_pool();
    auto& registry = scene.get_registry();

    const float delta_pos_magnitude = timer.get_delta_time() * _bullet_speed;
    bullet_pool.release_if([&](auto bullet_entity) {
        auto& time_component = registry.get<TimeComponent>(bullet_entity);
        time_component.time -= timer.get_delta_time();
        if (time_component.time <= 0.0f)
        {
            time_component.time = 0;

            auto& mesh_component = registry.get<InstancedMeshComponent>(bullet_entity);
            mesh_component.is_visible = false;

            return true;
        }

        auto& movement_component = registry.get<MovementComponent>(bullet_entity);
        simd_float3 direction{movement_component.direction.x, 0.0f, movement_component.direction.y};

        auto& transform_component = registry.get<TransformComponent>(bullet_entity);
        transform_component.position += direction * delta_pos_magnitude;

        // collision
        const auto& capsule = registry.get<ColliderComponent>(bullet_entity).capsule;
        auto r = scene.get_enemy_pool().release_first_if([&](auto enemy_entity) {
            auto& enemy_transform_component = registry.get<TransformComponent>(enemy_entity);
            auto enemy_position = enemy_transform_component.position;
            auto& enemy_capsule = registry.get<ColliderComponent>(enemy_entity).capsule;

            float max_collision_d = capsule.height / 2.0f + capsule.radius
                + enemy_capsule.height / 2.0f + enemy_capsule.radius;
            if (simd_distance(transform_component.position, enemy_position) > max_collision_d)
            {
                return false;
            }

            auto a0 = transform_component.position - direction * (capsule.height / 2.0f);
            auto a1 = transform_component.position + direction * (capsule.height / 2.0f);

            auto& enemy_movement_component = registry.get<MovementComponent>(enemy_entity);
            auto enemy_direction = simd_float3{
                enemy_movement_component.direction.x,
                0.0f,
                enemy_movement_component.direction.y
            };

            auto b0 = enemy_position - enemy_direction * (enemy_capsule.height / 2.0f);
            auto b1 = enemy_position + enemy_direction * (enemy_capsule.height / 2.0f);

            const float closest_d = math::distance_between_line_segments(a0, a1, b0, b1);
            return closest_d <= (capsule.radius + enemy_capsule.radius);
        });

        if (r)
        {
            {
                time_component.time = 0;

                auto& mesh_component = registry.get<InstancedMeshComponent>(bullet_entity);
                mesh_component.is_visible = false;
            }

            {
                auto& mesh_component = registry.get<InstancedMeshComponent>(*r);
                mesh_component.is_visible = false;
            }

            {
                auto& score_component = registry.get<ScoreComponent>(scene.get_player());
                score_component.score += 1;
            }

            return true;
        }

        return false;
    });
}
