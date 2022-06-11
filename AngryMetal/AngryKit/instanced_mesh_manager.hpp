//
//  instanced_mesh_manager.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 26.06.2021.
//

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "mesh.hpp"

namespace angry
{

enum class InstanceBufferType
{
    transform,
    aim_rotation
};

struct InstancedMesh
{
    size_t count = 0;
    size_t max_count = 0;
    Mesh mesh;
    std::unordered_map<InstanceBufferType, size_t> buffers;
};

class InstancedMeshManager final
{
public:
    using MeshArray = std::vector<std::unique_ptr<InstancedMesh>>;

public:
    size_t create();
    InstancedMesh& get_mesh(size_t index);
    const MeshArray& get_all() const;

private:
    MeshArray _meshes;
};

};
