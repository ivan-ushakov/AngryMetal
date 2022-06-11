//
//  texture_manager.mm
//  AngryKit
//
//  Created by  Ivan Ushakov on 12.06.2021.
//

#include "texture_manager.h"

#import <Accelerate/Accelerate.h>

#include <vector>
#include <sstream>
#include <stdexcept>

#include "image.hpp"

using namespace angry;

TextureManager::TextureManager(id<MTLDevice> device) : _device(device)
{
}

size_t TextureManager::create_texture(const std::filesystem::path& file_path)
{
    Image image(file_path);

    MTLPixelFormat pixel_format = MTLPixelFormatInvalid;
    std::vector<unsigned char> buffer;
    NSUInteger bytes_per_row = 0;
    switch (image.components)
    {
        case 1:
        {
            // grey

            pixel_format = MTLPixelFormatR8Unorm;
            bytes_per_row = 1 * image.width;
            break;
        }

        case 3:
        {
            // red, green, blue

            vImage_Buffer source;
            source.data = image.data;
            source.height = image.height;
            source.width = image.width;
            source.rowBytes = 3 * image.width;

            vImage_Buffer target;
            buffer.resize(4 * image.width * image.height);
            target.data = buffer.data();
            target.height = image.height;
            target.width = image.width;
            target.rowBytes = 4 * image.width;

            if (vImageConvert_RGB888toBGRA8888(&source, NULL, 0, &target, NO, kvImageNoFlags) != kvImageNoError)
            {
                throw std::runtime_error("create_texture() fail to convert from RGB888 to BGRA8888");
            }

            pixel_format = MTLPixelFormatBGRA8Unorm;
            bytes_per_row = 4 * image.width;
            break;
        }

        case 4:
        {
            // red, green, blue, alpha

            vImage_Buffer source;
            source.data = image.data;
            source.height = image.height;
            source.width = image.width;
            source.rowBytes = 4 * image.width;

            vImage_Buffer target;
            buffer.resize(4 * image.width * image.height);
            target.data = buffer.data();
            target.height = image.height;
            target.width = image.width;
            target.rowBytes = 4 * image.width;

            // from RGBA (0, 1, 2, 3) to BGRA
            const uint8_t permute_map[4] = {2, 1, 0, 3};
            if (vImagePermuteChannels_ARGB8888(&source, &target, permute_map, kvImageNoFlags) != kvImageNoError)
            {
                throw std::runtime_error("create_texture() fail to permute channels");
            }

            pixel_format = MTLPixelFormatBGRA8Unorm;
            bytes_per_row = 4 * image.width;
            break;
        }

        default:
        {
            std::stringstream t;
            t << "create_texture() unsupported number of components " << image.components;
            throw std::runtime_error(t.str());
        }
    }

    auto* descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixel_format
                                                                          width:image.width
                                                                         height:image.height
                                                                      mipmapped:NO];
    descriptor.storageMode = MTLStorageModeShared;
    TextureRef texture([_device newTextureWithDescriptor:descriptor]);

    MTLRegion region = MTLRegionMake2D(0, 0, image.width, image.height);
    const void* pixel_bytes = buffer.empty() ? image.data : buffer.data();
    [texture.get() replaceRegion:region mipmapLevel:0 withBytes:pixel_bytes bytesPerRow:bytes_per_row];

    size_t index = _textures.size();
    _textures.push_back(texture);

    return index;
}

id<MTLTexture> TextureManager::get_texture(size_t index)
{
    return _textures[index].get();
}
