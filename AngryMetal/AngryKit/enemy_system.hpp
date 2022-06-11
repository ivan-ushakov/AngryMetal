//
//  enemy_system.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 14.06.2021.
//

#pragma once

namespace angry
{

class Scene;

class EnemySystem final
{
public:
    EnemySystem();
    ~EnemySystem() = default;

    EnemySystem(const EnemySystem&) = delete;
    EnemySystem(EnemySystem&&) = delete;
    EnemySystem& operator=(const EnemySystem&) = delete;
    EnemySystem& operator=(EnemySystem&&) = delete;

    void update(Scene& scene, float delta_time);
    
private:
    const float _spawn_interval = 1.0f; // seconds
    const int _spawns_per_interval = 1;
    const float _spawn_radius = 10.0f;  // from player
    const float _monster_speed = 0.6f;
    const float _player_collision_radius = 0.35f;

    float _countdown = 0;
};

}
