//
//  image.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 22.05.2021.
//

#include "image.hpp"

#include <sstream>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace angry;

Image::Image(const std::filesystem::path& file_path)
{
    data = stbi_load(file_path.c_str(), &width, &height, &components, 0);
    if (data == nullptr)
    {
        std::stringstream t;
        t << "Image() fail to load image " << file_path;
        throw std::runtime_error(t.str());
    }
}

Image::~Image()
{
    if (data != nullptr)
    {
        stbi_image_free(data);
    }
}
