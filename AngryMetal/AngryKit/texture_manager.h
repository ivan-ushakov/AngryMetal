//
//  texture_manager.h
//  AngryKit
//
//  Created by  Ivan Ushakov on 12.06.2021.
//

#pragma once

#import <Metal/Metal.h>

#include <vector>

#include "texture_manager_interface.hpp"
#include "objc_ref.h"

namespace angry
{

class TextureManager final : public TextureManagerInterface
{
public:
    explicit TextureManager(id<MTLDevice> device);

    TextureManager(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager& operator=(TextureManager&&) = delete;

    size_t create_texture(const std::filesystem::path& file_path) override;

    id<MTLTexture> get_texture(size_t index);

private:
    id<MTLDevice> _device;

    using TextureRef = objc::Ref<id<MTLTexture>>;
    std::vector<TextureRef> _textures;
};

}
