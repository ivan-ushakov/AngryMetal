//
//  game_over_screen.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 10.02.2022.
//

#include "game_over_screen.hpp"

#include <imgui.h>

#include "input_component.hpp"
#include "on_exit.hpp"
#include "scene.hpp"

using namespace angry::ui;

void GameOverScreen::reset()
{
    _total_time = 0.0f;
}

void GameOverScreen::update(const Context& context, Scene& scene, float delta_time)
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
    
    constexpr float space = 10.0f;

    const auto title = "GAME OVER";
    ImVec2 title_size;

    ImGui::PushFont(context.big_font);
    title_size = ImGui::CalcTextSize(title);
    ImGui::PopFont();

    const auto subtitle = "Tap to continue";
    ImVec2 subtitle_size;

    ImGui::PushFont(context.default_font);
    subtitle_size = ImGui::CalcTextSize(subtitle);
    ImGui::PopFont();

    const auto title_x = floor((viewport->Size.x - title_size.x) / 2.0f);
    const auto title_y = floor((viewport->Size.y - (title_size.y + space + subtitle_size.y)) / 2.0f);
    ImGui::SetCursorPos(ImVec2(title_x, title_y));

    ImGui::PushFont(context.big_font);
    ImGui::TextUnformatted(title);
    ImGui::PopFont();

    const auto subtitle_x = floor((viewport->Size.x - subtitle_size.x) / 2.0f);
    const auto subtitle_y = title_y + title_size.y + space;
    ImGui::SetCursorPos(ImVec2(subtitle_x, subtitle_y));

    ImGui::PushFont(context.default_font);
    ImGui::TextUnformatted(subtitle);
    ImGui::PopFont();

    _total_time += delta_time;
    if (_total_time < pause_time)
    {
        return;
    }

    if (!context.touches.empty())
    {
        auto& input_component = scene.get_registry().get<InputComponent>(scene.get_player());
        input_component.is_restarting = true;
    }
}
