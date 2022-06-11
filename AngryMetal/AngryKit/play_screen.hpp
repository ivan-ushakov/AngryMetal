//
//  play_screen.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 10.02.2022.
//

#pragma once

#include <filesystem>

#include "score_string.hpp"
#include "screen.hpp"

namespace angry
{

class TextureManager;

namespace ui
{

class PlayScreen final : public Screen
{
public:
    PlayScreen(TextureManager& texture_manager, const std::filesystem::path& assets_path);

    void reset() override;
    void update(const Context& context, Scene& scene, float delta_time) override;

private:
    TextureManager& _texture_manager;
    size_t _button_texture = 0;
    ScoreString _score_string;
};

}

}
