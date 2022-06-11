//
//  metal_context.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 31.07.2021.
//

#pragma once

#import <Metal/Metal.h>

#include <simd/simd.h>

namespace angry
{

struct MetalContext
{
    id<MTLCommandBuffer> command_buffer = nullptr;
    MTLRenderPassDescriptor* render_pass_descriptor = nullptr;

    id<MTLRenderCommandEncoder> get_render_encoder()
    {
        if (_render_encoder == nullptr)
        {
            _render_encoder = [command_buffer renderCommandEncoderWithDescriptor:render_pass_descriptor];
        }
        return _render_encoder;
    }

private:
    id<MTLRenderCommandEncoder> _render_encoder = nullptr;
};

}
