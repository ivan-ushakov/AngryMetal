//
//  mesh.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 22.06.2021.
//

#pragma once

#include <unordered_map>

#include "render_pass_type.h"

namespace angry
{

enum class MaterialTexture
{
    diffuse, normal, specular, shadow
};

struct Material
{
    std::unordered_map<MaterialTexture, size_t> textures;
};

enum class VertexAttribute
{
    position, normal, uv
};

struct Mesh
{
    std::unordered_map<VertexAttribute, size_t> vertex_buffer;
    size_t vertex_count = 0;

    size_t index_buffer = 0;
    size_t index_count = 0;

    RenderPassType render_pass_type = RenderPassType::none;
    Material material;
};

}
