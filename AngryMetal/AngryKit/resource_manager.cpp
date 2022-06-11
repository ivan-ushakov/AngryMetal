//
//  resource_manager.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 27.06.2021.
//

#include "resource_manager.hpp"

using namespace angry;

ResourceManager::ResourceManager(BufferManagerInterface& buffer_manager,
                                 InstancedMeshManager& instanced_mesh_manager,
                                 TextureManagerInterface& texture_manager)
    : _buffer_manager(buffer_manager),
    _instanced_mesh_manager(instanced_mesh_manager),
    _texture_manager(texture_manager)
{
}
