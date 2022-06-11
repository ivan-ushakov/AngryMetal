//
//  buffer_manager_interface.hpp
//  AngryMetal
//
//  Created by  Ivan Ushakov on 26.05.2021.
//

#pragma once

#include <cstdint>
#include <memory>

namespace angry
{

struct BufferManagerInterface
{
    template<class T>
    struct View
    {
        T* data = nullptr;
        size_t size = 0;
    };

    virtual ~BufferManagerInterface() = default;

    virtual size_t create_buffer(size_t size) = 0;
    virtual size_t create_buffer(const uint8_t* data, size_t size) = 0;

    template<class T>
    View<T> get_buffer_view(size_t index)
    {
        auto data = reinterpret_cast<T*>(get_buffer_data(index));
        return View<T>{data, get_buffer_size(index)};
    }

protected:
    virtual void* get_buffer_data(size_t index) = 0;
    virtual size_t get_buffer_size(size_t index) = 0;
};

}
