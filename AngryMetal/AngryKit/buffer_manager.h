//
//  buffer_manager.h
//  AngryKit
//
//  Created by  Ivan Ushakov on 26.05.2021.
//

#pragma once

#import <Metal/Metal.h>

#include <vector>

#include "buffer_manager_interface.hpp"
#include "objc_ref.h"

namespace angry
{

class BufferManager final : public BufferManagerInterface
{
public:
    explicit BufferManager(id<MTLDevice> device);

    BufferManager(const BufferManager&) = delete;
    BufferManager(BufferManager&&) = delete;
    BufferManager& operator=(const BufferManager&) = delete;
    BufferManager& operator=(BufferManager&&) = delete;

    size_t create_buffer(size_t size) override;
    size_t create_buffer(const uint8_t* data, size_t size) override;

    id<MTLBuffer> get_buffer(size_t index);

private:
    void* get_buffer_data(size_t size) override;
    size_t get_buffer_size(size_t index) override;

private:
    id<MTLDevice> _device;

    using BufferRef = objc::Ref<id<MTLBuffer>>;
    std::vector<BufferRef> _buffers;
};

}
