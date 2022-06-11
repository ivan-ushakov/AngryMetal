//
//  game_restart_system.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 04.02.2022.
//

#pragma once

namespace angry
{

class Scene;

class GameRestartSystem final
{
public:
    GameRestartSystem() = default;
    ~GameRestartSystem() = default;

    GameRestartSystem(const GameRestartSystem&) = delete;
    GameRestartSystem(GameRestartSystem&&) = delete;
    GameRestartSystem& operator=(const GameRestartSystem&) = delete;
    GameRestartSystem& operator=(GameRestartSystem&&) = delete;

    void update(Scene& scene, float aspect);
};

}
