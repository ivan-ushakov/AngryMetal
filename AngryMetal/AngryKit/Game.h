//
//  Game.h
//  AngryKit
//
//  Created by  Ivan Ushakov on 15.05.2021.
//

#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface Game : NSObject

- (instancetype)initWithAssetsURL:(NSURL *)assetsURL;
- (BOOL)setup:(id<MTLDevice>)device error:(NSError **)error;
- (void)processTouch:(CGPoint)touch withTapCount:(NSUInteger)tapCount;
- (void)draw:(MTKView *)view;

@end

NS_ASSUME_NONNULL_END
