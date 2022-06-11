//
//  camera_system.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 06.06.2021.
//

#pragma once

namespace angry
{

class Scene;

class CameraSystem final
{
public:
    CameraSystem() = default;
    ~CameraSystem() = default;

    CameraSystem(const CameraSystem&) = delete;
    CameraSystem(CameraSystem&&) = delete;
    CameraSystem& operator=(const CameraSystem&) = delete;
    CameraSystem& operator=(CameraSystem&&) = delete;

    void update(Scene& scene, float aspect);
};

}
