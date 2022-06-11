//
//  player_input_system.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 05.06.2021.
//

#include "player_input_system.hpp"

#include <array>

#include "health_component.hpp"
#include "input_component.hpp"
#include "look_component.hpp"
#include "movement_component.hpp"
#include "scene.hpp"
#include "transform_component.hpp"

using namespace angry;

void PlayerInputSystem::update(Scene& scene, float delta_time)
{
    auto& health_component = scene.get_registry().get<HealthComponent>(scene.get_player());
    if (health_component.health == 0)
    {
        return;
    }

    auto& input_component = scene.get_registry().get<InputComponent>(scene.get_player());

    auto& look_component = scene.get_registry().get<LookComponent>(scene.get_player());
    look_component.direction = input_component.direction;

    const auto view = std::array<entt::entity, 2>{scene.get_player(), scene.get_gun()};
    for (auto entity : view)
    {
        auto& movement_component = scene.get_registry().get<MovementComponent>(entity);
        movement_component.direction = input_component.is_moving ? input_component.direction : 0;

        auto& transform_component = scene.get_registry().get<TransformComponent>(entity);
        transform_component.euler_angles.y = input_component.theta;
        if (input_component.is_moving)
        {
            simd_float3 d{input_component.direction.x, 0.0f, input_component.direction.y};
            transform_component.position += d * movement_component.speed * delta_time;
        }
    }
}
