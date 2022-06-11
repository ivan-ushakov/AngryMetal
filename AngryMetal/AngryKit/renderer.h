//
//  renderer.h
//  AngryKit
//
//  Created by  Ivan Ushakov on 16.05.2021.
//

#pragma once

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include <memory>
#include <vector>

#include "buffer_manager.h"
#include "instanced_mesh_manager.hpp"
#include "metal_context.h"
#include "render_pass.h"
#include "objc_ref.h"
#include "texture_manager.h"
#include "timer.hpp"

namespace angry
{

class Scene;
class ShadowMapManager;

using RenderPassPtr = std::unique_ptr<RenderPass>;

class Renderer final
{
public:
    Renderer(BufferManager* buffer_manager,
             InstancedMeshManager* instanced_mesh_manager,
             TextureManager* texture_manager);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    void setup(id<MTLDevice> device);
    void draw(MetalContext& context, Scene& scene, const Timer& timer);
    
private:
    void setup_depth_state(id<MTLDevice> device);
    RenderPass* find_render_pass(RenderPassType type) const;

private:
    BufferManager* _buffer_manager;
    InstancedMeshManager* _instanced_mesh_manager;
    TextureManager* _texture_manager;
    std::unique_ptr<ShadowMapManager> _shadow_map_manager;

    objc::Ref<id<MTLDepthStencilState>> _depth_state;

    using RenderPassPtr = std::unique_ptr<RenderPass>;
    std::vector<RenderPassPtr> _passes;
};

}
