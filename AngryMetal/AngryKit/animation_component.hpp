//
//  animation_component.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 30.05.2021.
//

#pragma once

#include <assimp/scene.h>

namespace angry
{

struct AnimationComponent
{
    aiAnimation* animation = nullptr;
    aiNode* root_node = nullptr;
    aiMatrix4x4 global_inv;

    float transition_time = 0.2f;
    float last_anim_time = 0.0f;
    float death_time = -1.0f;

    float prev_idle_weight = 0.0f;
    float prev_right_weight = 0.0f;
    float prev_forward_weight = 0.0f;
    float prev_back_weight = 0.0f;
    float prev_left_weight = 0.0f;
};

}
