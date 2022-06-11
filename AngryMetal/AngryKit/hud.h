//
//  hud.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 22.07.2021.
//

#pragma once

#import <Metal/Metal.h>

#include <filesystem>
#include <simd/simd.h>
#include <vector>
#include <sstream>

#include "enum_array.hpp"
#include "metal_context.h"
#include "screen.hpp"
#include "texture_manager.h"

namespace angry
{

class Scene;

class HUD final
{
public:
    HUD(id<MTLDevice> device, TextureManager* texture_manager, const std::filesystem::path& assets_path);
    ~HUD();

    HUD(const HUD&) = delete;
    HUD(HUD&&) = delete;
    HUD& operator=(const HUD&) = delete;
    HUD& operator=(HUD&&) = delete;

    void add_touch(simd_float2 location, size_t tap_count);
    void render(
        MetalContext& context,
        simd_float2 display_size,
        simd_float2 padding,
        float framebuffer_scale,
        Scene& scene,
        float delta_time
    );

private:
    enum class ScreenIndex
    {
        none, play, game_over
    };

    ScreenIndex get_screen_index(Scene& scene);

private:
    EnumArray<ScreenIndex, std::unique_ptr<ui::Screen>, 3> _screens;
    ui::Context _context;
    ScreenIndex _current_screen_index = ScreenIndex::none;
};

}
