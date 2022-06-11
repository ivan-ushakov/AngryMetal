//
//  render_pass.h
//  AngryMetal
//
//  Created by  Ivan Ushakov on 23.05.2021.
//

#pragma once

#import <Metal/Metal.h>

#include <unordered_map>
#include <vector>

#include "instanced_mesh_manager.hpp"
#include "mesh.hpp"
#include "render_pass_attribute.h"
#include "render_pass_type.h"

namespace angry
{

using AttributeArray = std::vector<render::AttributeType>;
using TextureArray = std::vector<MaterialTexture>;
using InstanceBufferAttributes = std::unordered_map<InstanceBufferType, NSUInteger>;

class RenderPass
{
public:
    explicit RenderPass(RenderPassType type) : _type(type) {}

    virtual ~RenderPass() = default;

    RenderPassType get_type() const
    {
        return _type;
    }

    const AttributeArray& get_attributes() const
    {
        return _attributes;
    }

    const TextureArray& get_textures() const
    {
        return _textures;
    }

    NSUInteger get_attribute_index(InstanceBufferType type) const
    {
        return _instance_buffer_attributes.at(type);
    }

    virtual void set_attribute(const render::AttributeVariant& attribute) = 0;
    virtual void encode(id<MTLRenderCommandEncoder> command_encoder) = 0;

protected:
    RenderPassType _type;
    AttributeArray _attributes;
    TextureArray _textures;
    InstanceBufferAttributes _instance_buffer_attributes;
};

}
