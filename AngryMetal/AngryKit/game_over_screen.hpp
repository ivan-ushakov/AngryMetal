//
//  game_over_screen.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 10.02.2022.
//

#pragma once

#include "screen.hpp"

namespace angry::ui
{

class GameOverScreen final : public Screen
{
public:
    void reset() override;
    void update(const Context& context, Scene& scene, float delta_time) override;
private:
    constexpr static float pause_time = 3.0f;
    float _total_time;
};

}
