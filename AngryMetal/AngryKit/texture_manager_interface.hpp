//
//  texture_manager_interface.h
//  AngryMetal
//
//  Created by  Ivan Ushakov on 12.06.2021.
//

#pragma once

#include <filesystem>

namespace angry
{

struct TextureManagerInterface
{
    virtual ~TextureManagerInterface() = default;

    virtual size_t create_texture(const std::filesystem::path& file_path) = 0;
};

}
