//
//  screen.hpp
//  AngryMetal
//
//  Created by  Ivan Ushakov on 10.02.2022.
//

#pragma once

#include <simd/simd.h>
#include <vector>

class ImFont;

namespace angry
{

class Scene;

namespace ui
{

struct ScreenTouch
{
    simd_float2 coordinates;
    size_t tap_count;
};

struct Context
{
    ImFont* default_font;
    ImFont* big_font;
    simd_float2 padding;
    std::vector<ScreenTouch> touches;
};

struct Screen
{
    virtual ~Screen() = default;

    virtual void reset() = 0;
    virtual void update(const Context& context, Scene& scene, float delta_time) = 0;
};

}

}
