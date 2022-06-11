//
//  collider_component.hpp
//  AngryMetal
//
//  Created by  Ivan Ushakov on 20.07.2021.
//

#pragma once

namespace angry
{

struct Capsule
{
    float height = 0;
    float radius = 0;
};

struct ColliderComponent
{
    Capsule capsule;
};

}
