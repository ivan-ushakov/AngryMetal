//
//  instanced_mesh_component.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 22.06.2021.
//

#pragma once

#include <cstddef>

namespace angry
{

struct InstancedMeshComponent
{
    size_t instanced_mesh = 0;
    bool is_visible = false;
};

}
