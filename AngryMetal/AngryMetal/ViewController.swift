//
//  ViewController.swift
//  AngryMetal
//
//  Created by  Ivan Ushakov on 15.05.2021.
//

import UIKit
import MetalKit
import AngryKit

final class ViewController: UIViewController {

    private lazy var metalView: MTKView = {
        let view = MTKView()
        view.device = MTLCreateSystemDefaultDevice()
        view.delegate = self
        view.clearColor = MTLClearColor(red: 1.0, green: 1.0, blue: 1.0, alpha: 1.0)
        view.depthStencilPixelFormat = .depth32Float
        view.isPaused = true
        view.isMultipleTouchEnabled = true
        return view
    }()

    private lazy var game: Game = {
        return Game(assetsURL: Bundle.main.bundleURL.appendingPathComponent("Assets"))
    }()

    private var touchState: [UITouch: CGPoint] = [:]

    override func loadView() {
        view = metalView
    }

    override func viewDidLoad() {
        super.viewDidLoad()

        DispatchQueue.global().async {
            self.setup()
        }
    }

    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        super.touchesBegan(touches, with: event)
        touches.forEach { touch in
            touchState[touch] = touch.location(in: view)
        }
    }

    override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
        super.touchesMoved(touches, with: event)
        touches.forEach { touch in
            touchState[touch] = touch.location(in: view)
        }
    }

    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        super.touchesEnded(touches, with: event)
        touches.forEach { touch in
            touchState.removeValue(forKey: touch)
        }
    }

    override func touchesCancelled(_ touches: Set<UITouch>, with event: UIEvent?) {
        super.touchesCancelled(touches, with: event)
        touches.forEach { touch in
            touchState.removeValue(forKey: touch)
        }
    }

    private func setup() {
        guard let device = metalView.device else {
            return
        }

        do {
            try game.setup(device)

            DispatchQueue.main.async {
                self.start()
            }
        } catch {
            print("ERROR: \(error)")
        }
    }

    private func start() {
        metalView.isPaused = false
    }
}

extension ViewController: MTKViewDelegate {
    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
        print("DEBUG: drawable size = \(size)")
    }

    func draw(in view: MTKView) {
        touchState.forEach {
            game.processTouch($1, withTapCount: UInt($0.tapCount))
        }
        game.draw(view)
    }
}
