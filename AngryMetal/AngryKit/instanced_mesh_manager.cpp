//
//  instanced_mesh_manager.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 01.07.2021.
//

#include "instanced_mesh_manager.hpp"

using namespace angry;

size_t InstancedMeshManager::create()
{
    auto index = _meshes.size();
    _meshes.emplace_back(std::make_unique<InstancedMesh>());
    return index;
}

InstancedMesh& InstancedMeshManager::get_mesh(size_t index)
{
    return *_meshes[index];
}

const InstancedMeshManager::MeshArray& InstancedMeshManager::get_all() const
{
    return _meshes;
}
