//
//  shooting_system.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 16.09.2021.
//

#pragma once

namespace angry
{

class Scene;
class Timer;

class ShootingSystem final
{
public:
    ShootingSystem() = default;
    ~ShootingSystem() = default;

    ShootingSystem(const ShootingSystem&) = delete;
    ShootingSystem(ShootingSystem&&) = delete;
    ShootingSystem& operator=(const ShootingSystem&) = delete;
    ShootingSystem& operator=(ShootingSystem&&) = delete;

    void update(Scene& scene, const Timer& timer);

private:
    float _last_fire_time = 0;
    const float _fire_interval = 0.1f;
    const float _bullet_lifetime = 1.0f;
};

}
