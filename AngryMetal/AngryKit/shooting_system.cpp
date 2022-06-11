//
//  shooting_system.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 16.09.2021.
//

#include "shooting_system.hpp"

#include "collider_component.hpp"
#include "health_component.hpp"
#include "input_component.hpp"
#include "instanced_mesh_component.hpp"
#include "look_component.hpp"
#include "math.hpp"
#include "movement_component.hpp"
#include "on_exit.hpp"
#include "scene.hpp"
#include "timer.hpp"
#include "time_component.hpp"
#include "transform_component.hpp"

using namespace angry;

void ShootingSystem::update(Scene& scene, const Timer& timer)
{
    auto& input_component = scene.get_registry().get<InputComponent>(scene.get_player());

    OnExit guard([&input_component]{
        input_component.is_shooting = false;
    });

    auto& player_health_component = scene.get_registry().get<HealthComponent>(scene.get_player());
    if (player_health_component.health == 0)
    {
        return;
    }

    auto& bullet_pool = scene.get_bullet_pool();
    auto& registry = scene.get_registry();

    if (input_component.is_shooting && (_last_fire_time + _fire_interval) < timer.get_time_since_start())
    {
        auto entity = bullet_pool.get_entity();
        if (entity)
        {
            auto player_transform_component = registry.get<TransformComponent>(scene.get_player());
            const float player_model_gun_height = 120.0f;
            const float player_model_gun_muzzle_offset = 100.0f;
            auto muzzle_point = simd_float4{-20.0f, player_model_gun_height, player_model_gun_muzzle_offset, 1.0f};
            auto spawn_point = simd_mul(player_transform_component.get_matrix(), muzzle_point);

            auto& transform_component = registry.get<TransformComponent>(*entity);
            transform_component.position = spawn_point.xyz;
            transform_component.euler_angles.y = player_transform_component.euler_angles.y;

            auto& mesh_component = registry.get<InstancedMeshComponent>(*entity);
            mesh_component.is_visible = true;

            auto& time_component = registry.get<TimeComponent>(*entity);
            time_component.time = _bullet_lifetime;

            auto& player_look_component = registry.get<LookComponent>(scene.get_player());
            const auto x = simd_normalize(simd_float3{0.0f, 0.0f, 1.0f});
            const auto y = simd_normalize(simd_float3{player_look_component.direction.x, 0.0f, player_look_component.direction.y});
            const float theta = math::oriented_angle(x, y, simd_float3{0.0f, 1.0f, 0.0f});

            auto& movement_component = registry.get<MovementComponent>(*entity);
            movement_component.direction = simd_mul(matrix::make_rotation(simd_float3{0.0f, 1.0f, 0.0f}, theta), simd_float4{0.0f, 0.0f, 1.0f, 1.0f}).xz;
        }
        _last_fire_time = timer.get_time_since_start();
    }
}
