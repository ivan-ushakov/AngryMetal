//
//  player_input_system.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 05.06.2021.
//

#pragma once

namespace angry
{

class Scene;

class PlayerInputSystem final
{
public:
    PlayerInputSystem() = default;
    ~PlayerInputSystem() = default;

    PlayerInputSystem(const PlayerInputSystem&&) = delete;
    PlayerInputSystem(PlayerInputSystem&&) = delete;
    PlayerInputSystem& operator=(const PlayerInputSystem&) = delete;
    PlayerInputSystem& operator=(PlayerInputSystem&&) = delete;

    void update(Scene& scene, float delta_time);
};

}
