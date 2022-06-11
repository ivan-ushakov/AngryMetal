//
//  camera_system.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 06.06.2021.
//

#include "camera_system.hpp"

#include "camera_component.hpp"
#include "math.hpp"
#include "matrix.hpp"
#include "scene.hpp"
#include "transform_component.hpp"

using namespace angry;

void CameraSystem::update(Scene& scene, float aspect)
{
    auto player_position = scene.get_registry().get<TransformComponent>(scene.get_player()).position;

    const simd_float3 camera_follow{-4.0f, 4.3f, 0.0f};
    const simd_float3 camera_up{0.0f, 1.0f, 0.0f};

    auto& camera_component = scene.get_registry().get<CameraComponent>(scene.get_camera());
    camera_component.projection_matrix = matrix::make_perspective_projection(45.0f * math::radians, aspect, 0.1f, 10.0f);
    camera_component.position = player_position + camera_follow;
    camera_component.view_matrix = matrix::make_look_at(camera_component.position, player_position, camera_up);
    camera_component.projection_view_matrix = simd_mul(camera_component.projection_matrix, camera_component.view_matrix);
}
