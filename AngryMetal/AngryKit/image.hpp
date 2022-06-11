//
//  image.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 22.05.2021.
//

#pragma once

#include <filesystem>

namespace angry
{

struct Image
{
    int width = 0;
    int height = 0;
    int components = 0;
    unsigned char* data = nullptr;

    explicit Image(const std::filesystem::path& file_path);
    ~Image();

    Image(const Image&) = delete;
    Image(Image&&) = delete;
    Image& operator=(const Image&) = delete;
    Image& operator=(Image&&) = delete;

};

}
