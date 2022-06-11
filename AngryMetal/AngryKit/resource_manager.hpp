//
//  resource_manager.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 26.06.2021.
//

#pragma once

#include "buffer_manager_interface.hpp"
#include "instanced_mesh_manager.hpp"
#include "texture_manager_interface.hpp"

namespace angry
{

class ResourceManager final
{
public:
    ResourceManager(BufferManagerInterface& buffer_manager,
                    InstancedMeshManager& instanced_mesh_manager,
                    TextureManagerInterface& texture_manager);

    ~ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    BufferManagerInterface& get_buffer_manager() const
    {
        return _buffer_manager;
    }

    InstancedMeshManager& get_instanced_mesh_manager() const
    {
        return _instanced_mesh_manager;
    }

    TextureManagerInterface& get_texture_manager() const
    {
        return _texture_manager;
    }

private:
    BufferManagerInterface& _buffer_manager;
    InstancedMeshManager& _instanced_mesh_manager;
    TextureManagerInterface& _texture_manager;
};

}
