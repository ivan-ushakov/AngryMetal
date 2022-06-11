//
//  hud.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 22.07.2021.
//

#include "hud.h"

#include <backends/imgui_impl_metal.h>
#include <imgui.h>
#include <optional>

#include "game_over_screen.hpp"
#include "health_component.hpp"
#include "input_component.hpp"
#include "math.hpp"
#include "play_screen.hpp"
#include "scene.hpp"

using namespace angry;

HUD::HUD(id<MTLDevice> device, TextureManager* texture_manager, const std::filesystem::path& assets_path)
{
    const std::filesystem::path hud_path = assets_path / "HUD";

    _screens[ScreenIndex::play] = std::make_unique<ui::PlayScreen>(*texture_manager, hud_path);
    _screens[ScreenIndex::game_over] = std::make_unique<ui::GameOverScreen>();

    _context.touches.reserve(2);

    {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO();
        const auto font_path = hud_path / "Roboto-Medium.ttf";
        _context.default_font = io.Fonts->AddFontFromFileTTF(font_path.c_str(), 16.0f);
        _context.big_font = io.Fonts->AddFontFromFileTTF(font_path.c_str(), 32.0f);

        ImGui_ImplMetal_Init(device);
    }
}

HUD::~HUD()
{
    ImGui_ImplMetal_Shutdown();
    ImGui::DestroyContext();
}

void HUD::add_touch(simd_float2 location, size_t tap_count)
{
    if (_context.touches.size() < _context.touches.capacity())
    {
        _context.touches.emplace_back(ui::ScreenTouch{location, tap_count});
    }
}

void HUD::render(
    MetalContext& context,
    simd_float2 display_size,
    simd_float2 padding,
    float framebuffer_scale,
    Scene& scene,
    float delta_time
)
{
    auto& io = ImGui::GetIO();
    io.DisplaySize.x = display_size.x;
    io.DisplaySize.y = display_size.y;

    io.DisplayFramebufferScale = ImVec2(framebuffer_scale, framebuffer_scale);
    io.DeltaTime = delta_time;

    ImGui_ImplMetal_NewFrame(context.render_pass_descriptor);
    ImGui::NewFrame();

    const auto screen_index = get_screen_index(scene);
    if (_current_screen_index != screen_index)
    {
        _current_screen_index = screen_index;
        _screens[_current_screen_index]->reset();
    }

    _context.padding = padding;
    _screens[_current_screen_index]->update(_context, scene, delta_time);

    // rendering
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();

    auto render_encoder = context.get_render_encoder();
    [render_encoder pushDebugGroup:@"Dear ImGui rendering"];
    ImGui_ImplMetal_RenderDrawData(draw_data, context.command_buffer, render_encoder);
    [render_encoder popDebugGroup];

    _context.touches.clear();
}

HUD::ScreenIndex HUD::get_screen_index(Scene& scene)
{
    const auto& player_health_component = scene.get_registry().get<HealthComponent>(scene.get_player());
    if (player_health_component.health == 0)
    {
        return ScreenIndex::game_over;
    }
    else
    {
        return ScreenIndex::play;
    }
}
