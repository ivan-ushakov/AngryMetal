//
//  player_animation_system.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 25.05.2021.
//

#pragma once

#include "buffer_manager_interface.hpp"

namespace angry
{

class Scene;

class PlayerAnimationSystem final
{
public:
    explicit PlayerAnimationSystem(BufferManagerInterface& buffer_manager);
    ~PlayerAnimationSystem() = default;

    PlayerAnimationSystem(const PlayerAnimationSystem&&) = delete;
    PlayerAnimationSystem(PlayerAnimationSystem&&) = delete;
    PlayerAnimationSystem& operator=(const PlayerAnimationSystem&) = delete;
    PlayerAnimationSystem& operator=(PlayerAnimationSystem&&) = delete;

    void update(Scene& scene, float time);

private:
    BufferManagerInterface& _buffer_manager;
};

}
