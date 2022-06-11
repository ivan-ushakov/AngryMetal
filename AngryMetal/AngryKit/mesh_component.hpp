//
//  mesh_component.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 16.05.2021.
//

#pragma once

#include "mesh.hpp"

namespace angry
{

struct MeshComponent
{
    class aiMesh* source_mesh = nullptr;
    Mesh mesh;
    bool has_shadow = false;
    bool is_visible = false;
};

}
