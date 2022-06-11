//
//  play_screen.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 10.02.2022.
//

#include "play_screen.hpp"

#include <imgui.h>

#include "camera_component.hpp"
#include "input_component.hpp"
#include "math.hpp"
#include "on_exit.hpp"
#include "scene.hpp"
#include "score_component.hpp"
#include "texture_manager.h"
#include "transform_component.hpp"

namespace angry
{

static bool contains_point(ImVec2 origin, ImVec2 size, ImVec2 point)
{
    const auto dx = point.x - origin.x;
    const auto dy = point.y - origin.y;
    return dx > 0 && dx < size.x && dy > 0 && dy < size.y;
}

}

using namespace angry::ui;

PlayScreen::PlayScreen(TextureManager& texture_manager, const std::filesystem::path& assets_path) :
    _texture_manager(texture_manager)
{
    _button_texture = texture_manager.create_texture(assets_path / "Button.png");
}

void PlayScreen::reset()
{
    _score_string.reset();
}

void PlayScreen::update(const Context& context, Scene& scene, float delta_time)
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoBackground
        | ImGuiWindowFlags_NoNav;
    const auto* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGui::Begin("HUD", NULL, window_flags);
    OnExit guard([](){ ImGui::End(); });

    {
        ImGui::PushFont(context.default_font);
        const auto& score_component = scene.get_registry().get<ScoreComponent>(scene.get_player());
        _score_string.update(score_component.score);
        const auto text = _score_string.get();
        const auto text_size = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(ImVec2(viewport->Size.x - text_size.x - context.padding.x, context.padding.y));
        ImGui::TextUnformatted(text);
        ImGui::PopFont();
    }

    const auto control_area_size = floor((viewport->Size.y - 2.0f * context.padding.y) / 3.0f);
    auto& input_component = scene.get_registry().get<InputComponent>(scene.get_player());

    ImVec2 uv1(0.0f, 0.0f);
    ImVec2 uv2(1.0f, 1.0f);
    ImVec4 tint_color(1.0f, 1.0f, 1.0f, 0.25f);
    {
        ImTextureID texture_id = _texture_manager.get_texture(_button_texture);
        const auto size = floor(control_area_size / 2.5f);
        ImVec2 button_size(size, size);
        {
            const auto x = viewport->Size.x - size - context.padding.x;
            ImVec2 origin(x, viewport->Size.y - context.padding.y - control_area_size);
            ImGui::SetCursorPos(origin);
            ImGui::Image(texture_id, button_size, uv1, uv2, tint_color);

            const auto p = std::find_if(context.touches.cbegin(), context.touches.cend(), [&](const auto &touch) {
                return contains_point(origin, button_size, ImVec2(touch.coordinates.x, touch.coordinates.y));
            });
            input_component.is_moving = p != context.touches.cend();
        }

        {
            const auto x = viewport->Size.x - context.padding.x - control_area_size;
            ImVec2 origin(x, viewport->Size.y - context.padding.y - size);
            ImGui::SetCursorPos(origin);
            ImGui::Image(texture_id, button_size, uv1, uv2, tint_color);

            const auto p = std::find_if(context.touches.cbegin(), context.touches.cend(), [&](const auto &touch) {
                return contains_point(origin, button_size, ImVec2(touch.coordinates.x, touch.coordinates.y));
            });
            input_component.is_shooting = p != context.touches.cend();
        }
    }

    ImVec2 origin(viewport->Size.x - control_area_size - context.padding.x, viewport->Size.y - context.padding.y - control_area_size);
    ImVec2 size(control_area_size, control_area_size);
    const auto p = std::find_if(context.touches.cbegin(), context.touches.cend(), [&](const auto &touch) {
        return !contains_point(origin, size, ImVec2(touch.coordinates.x, touch.coordinates.y));
    });
    if (p == context.touches.cend())
    {
        return;
    }

    const auto coordinates = p->coordinates;
    const auto clip_x = -1.0f + 2.0f * coordinates.x / viewport->Size.x;
    const auto clip_y = 1.0f - 2.0f * coordinates.y / viewport->Size.y;
    if (fabs(clip_x) < 0.005f && fabs(clip_y) < 0.005f)
    {
        input_component.theta = 0.0f;
    }
    else
    {
        const auto& camera_component = scene.get_registry().get<CameraComponent>(scene.get_camera());

        const auto& player_transform_component = scene.get_registry().get<TransformComponent>(scene.get_player());
        const float player_model_gun_height = 120.0f;
        const auto monster_y = player_transform_component.scale.x * player_model_gun_height;

        const auto world_coordinates = math::get_world_coordinates(
            camera_component.projection_matrix,
            camera_component.view_matrix,
            clip_x,
            clip_y,
            monster_y
        );

        const auto dx = world_coordinates.x - player_transform_component.position.x;
        const auto dz = world_coordinates.z - player_transform_component.position.z;

        if (dz != 0)
        {
            input_component.theta = atanf(dx / dz) + (dz < 0.0f ? 180.0f * math::radians : 0.0f);
        }
        else
        {
            input_component.theta = 0.0f;
        }
    }

    input_component.direction = simd_float2{
        sinf(input_component.theta),
        cosf(input_component.theta)
    };
}
