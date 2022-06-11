//
//  bullet_system.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 07.07.2021.
//

#pragma once

namespace angry
{

class Scene;
class Timer;

class BulletSystem final
{
public:
    BulletSystem() = default;
    ~BulletSystem() = default;

    BulletSystem(const BulletSystem&) = delete;
    BulletSystem(BulletSystem&&) = delete;
    BulletSystem& operator=(const BulletSystem&) = delete;
    BulletSystem& operator=(BulletSystem&&) = delete;

    void update(Scene& scene, const Timer& timer);

private:
    const float _bullet_lifetime = 1.0f;
    const float _bullet_speed = 15.0f;
};

}
