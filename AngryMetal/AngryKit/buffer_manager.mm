//
//  buffer_manager.mm
//  AngryKit
//
//  Created by  Ivan Ushakov on 26.05.2021.
//

#include "buffer_manager.h"

#include <stdexcept>

using namespace angry;

BufferManager::BufferManager(id<MTLDevice> device) : _device(device)
{
}

size_t BufferManager::create_buffer(size_t size)
{
    BufferRef buffer([_device newBufferWithLength:size options:MTLStorageModeShared]);
    if (!buffer)
    {
        throw std::runtime_error("BufferManager::create_buffer");
    }

    size_t index = _buffers.size();
    _buffers.push_back(buffer);

    return index;
}

size_t BufferManager::create_buffer(const uint8_t* data, size_t size)
{
    BufferRef buffer([_device newBufferWithBytes:data length:size options:MTLStorageModeShared]);
    if (!buffer)
    {
        throw std::runtime_error("BufferManager::create_buffer");
    }

    size_t index = _buffers.size();
    _buffers.push_back(buffer);

    return index;
}

id<MTLBuffer> BufferManager::get_buffer(size_t index)
{
    return _buffers[index].get();
}

void* BufferManager::get_buffer_data(size_t index)
{
    return _buffers[index].get().contents;
}

size_t BufferManager::get_buffer_size(size_t index)
{
    return _buffers[index].get().length;
}
