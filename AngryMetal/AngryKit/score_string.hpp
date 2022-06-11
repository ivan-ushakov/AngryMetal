//
//  score_string.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 15.01.2022.
//

#pragma once

#include <string>

namespace angry
{

class ScoreString final
{
public:
    ScoreString();

    void update(int score);
    void reset();
    const char *get() const;

private:
    std::string _buffer;
    size_t _prefix_size;
};

}
