//
//  game_restart_system.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 04.02.2022.
//

#include "game_restart_system.hpp"

#include "animation_component.hpp"
#include "camera_component.hpp"
#include "health_component.hpp"
#include "input_component.hpp"
#include "instanced_mesh_component.hpp"
#include "look_component.hpp"
#include "movement_component.hpp"
#include "scene.hpp"
#include "score_component.hpp"
#include "time_component.hpp"
#include "transform_component.hpp"

using namespace angry;

void GameRestartSystem::update(Scene& scene, float aspect)
{
    auto& registry = scene.get_registry();

    auto& input_component = registry.get<InputComponent>(scene.get_player());
    if (!input_component.is_restarting)
    {
        return;
    }
    input_component.is_restarting = false;

    {
        auto entity = scene.get_player();

        auto& animation_component = registry.get<AnimationComponent>(entity);
        animation_component.transition_time = 0.2f;
        animation_component.last_anim_time = 0.0f;
        animation_component.death_time = -1.0f;

        animation_component.prev_idle_weight = 0.0f;
        animation_component.prev_right_weight = 0.0f;
        animation_component.prev_forward_weight = 0.0f;
        animation_component.prev_back_weight = 0.0f;
        animation_component.prev_left_weight = 0.0f;

        auto& movement_component = registry.get<MovementComponent>(entity);
        movement_component.speed = 1.5f;
        movement_component.direction = {0.0f, 0.0f};

        auto& transform_component = registry.get<TransformComponent>(entity);
        transform_component.position = simd_float3{0.0f, 0.0f, 0.0f};
        transform_component.euler_angles = {0.0f, 0.0f, 0.0f};

        auto& health_component = registry.get<HealthComponent>(entity);
        health_component.health = 100;

        auto& look_component = registry.get<LookComponent>(entity);
        look_component.direction = {0.0f, 0.0f};

        auto& score_component = registry.get<ScoreComponent>(entity);
        score_component.score = 0;
    }

    {
        auto entity = scene.get_gun();

        auto& movement_component = registry.get<MovementComponent>(entity);
        movement_component.speed = 1.5f;
        movement_component.direction = {0.0f, 0.0f};

        auto& transform_component = registry.get<TransformComponent>(entity);
        transform_component.position = simd_float3{0.0f, 0.0f, 0.0f};
        transform_component.euler_angles = {0.0f, 0.0f, 0.0f};
    }

    {
        auto& pool = scene.get_enemy_pool();
        pool.release_if([&registry](auto entity) {
            auto& mesh_component = registry.get<InstancedMeshComponent>(entity);
            mesh_component.is_visible = false;

            return true;
        });
    }

    {
        auto& pool = scene.get_bullet_pool();
        pool.release_if([&registry](auto entity) {
            auto& time_component = registry.get<TimeComponent>(entity);
            time_component.time = 0;

            auto& mesh_component = registry.get<InstancedMeshComponent>(entity);
            mesh_component.is_visible = false;

            return true;
        });
    }
}
