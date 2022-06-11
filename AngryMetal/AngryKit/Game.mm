//
//  Game.mm
//  AngryKit
//
//  Created by  Ivan Ushakov on 15.05.2021.
//

#import "Game.h"

#include <filesystem>
#include <memory>
#include <stdexcept>

#include "buffer_manager.h"
#include "bullet_system.hpp"
#include "camera_system.hpp"
#include "enemy_system.hpp"
#include "game_restart_system.hpp"
#include "hud.h"
#include "instanced_mesh_manager.hpp"
#include "metal_context.h"
#include "objc_ref.h"
#include "player_animation_system.hpp"
#include "player_input_system.hpp"
#include "renderer.h"
#include "resource_manager.hpp"
#include "scene.hpp"
#include "shooting_system.hpp"
#include "texture_manager.h"
#include "timer.hpp"

@implementation Game
{
    std::filesystem::path assets_path;
    std::unique_ptr<angry::BufferManager> buffer_manager;
    std::unique_ptr<angry::InstancedMeshManager> instanced_mesh_manager;
    std::unique_ptr<angry::TextureManager> texture_manager;
    std::unique_ptr<angry::ResourceManager> resource_manager;

    std::unique_ptr<angry::PlayerInputSystem> player_input_system;
    std::unique_ptr<angry::CameraSystem> camera_system;
    std::unique_ptr<angry::PlayerAnimationSystem> player_animation_system;
    std::unique_ptr<angry::EnemySystem> enemy_system;
    std::unique_ptr<angry::BulletSystem> bullet_system;
    std::unique_ptr<angry::ShootingSystem> shooting_system;
    std::unique_ptr<angry::GameRestartSystem> game_restart_system;

    std::unique_ptr<angry::Renderer> renderer;
    std::unique_ptr<angry::Scene> scene;
    std::unique_ptr<angry::HUD> hud;

    angry::Timer _timer;
    objc::Ref<id<MTLCommandQueue>> command_queue;
}

- (instancetype)initWithAssetsURL:(NSURL *)assetsURL
{
    self = [super init];
    if (self != nil)
    {
        assets_path = assetsURL.path.UTF8String;
    }
    return self;
}

- (BOOL)setup:(id<MTLDevice>)device error:(NSError **)error
{
    using namespace angry;

    buffer_manager = std::make_unique<BufferManager>(device);
    instanced_mesh_manager = std::make_unique<InstancedMeshManager>();
    texture_manager = std::make_unique<TextureManager>(device);
    resource_manager = std::make_unique<ResourceManager>(*buffer_manager,
                                                         *instanced_mesh_manager,
                                                         *texture_manager);

    player_input_system = std::make_unique<PlayerInputSystem>();
    camera_system = std::make_unique<CameraSystem>();
    player_animation_system = std::make_unique<PlayerAnimationSystem>(*buffer_manager);
    enemy_system = std::make_unique<EnemySystem>();
    bullet_system = std::make_unique<BulletSystem>();
    shooting_system = std::make_unique<ShootingSystem>();
    game_restart_system = std::make_unique<GameRestartSystem>();

    try
    {
        scene = std::make_unique<Scene>(resource_manager.get());
        scene->load(assets_path);

        renderer = std::make_unique<Renderer>(buffer_manager.get(),
                                              instanced_mesh_manager.get(),
                                              texture_manager.get());
        renderer->setup(device);

        hud = std::make_unique<HUD>(device, texture_manager.get(), assets_path);
    }
    catch (const std::runtime_error& e)
    {
        NSLog(@"ERROR: %s", e.what());
        *error = [NSError errorWithDomain:@"AngryKit" code:0 userInfo:nil];
        return NO;
    }

    command_queue = [device newCommandQueue];

    return YES;
}

- (void)processTouch:(CGPoint)touch withTapCount:(NSUInteger)tapCount
{
    hud->add_touch(simd_float2{float(touch.x), float(touch.y)}, tapCount);
}

- (void)draw:(MTKView *)view
{
    if (!scene || !renderer)
    {
        return;
    }

    // system order is important
    game_restart_system->update(*scene, _timer.get_delta_time());
    player_input_system->update(*scene, _timer.get_delta_time());
    shooting_system->update(*scene, _timer);
    enemy_system->update(*scene, _timer.get_delta_time());
    camera_system->update(*scene, static_cast<float>(view.bounds.size.width / view.bounds.size.height));
    player_animation_system->update(*scene, _timer.get_time_since_start());
    bullet_system->update(*scene, _timer);

    [self render:view];

    _timer.update();
}

- (void)render:(MTKView *)view
{
    id<MTLCommandBuffer> command_buffer = [command_queue.get() commandBuffer];
    MTLRenderPassDescriptor* render_pass_descriptor = view.currentRenderPassDescriptor;
    if (render_pass_descriptor == nil)
    {
        [command_buffer commit];
        return;
    }

    angry::MetalContext metal_context;
    metal_context.command_buffer = command_buffer;
    metal_context.render_pass_descriptor = render_pass_descriptor;

    renderer->draw(metal_context, *scene, _timer);

    UIEdgeInsets safe_area_insets = view.safeAreaInsets;
    CGFloat left_inset = fmax(safe_area_insets.left, safe_area_insets.right);
    CGFloat top_inset = fmax(safe_area_insets.top, safe_area_insets.bottom);
    hud->render(
        metal_context,
        simd_float2{float(view.bounds.size.width), float(view.bounds.size.height)},
        simd_float2{float(left_inset), float(top_inset)},
        float(view.window.screen.scale),
        *scene,
        _timer.get_delta_time() == 0 ? 1.0f / view.preferredFramesPerSecond : _timer.get_delta_time()
    );

    [metal_context.get_render_encoder() endEncoding];

    // present
    [command_buffer presentDrawable:view.currentDrawable];
    [command_buffer commit];
}

@end
