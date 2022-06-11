//
//  score_string.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 15.01.2022.
//

#include "score_string.hpp"

#include <charconv>

using namespace angry;

ScoreString::ScoreString() : _buffer("SCORE ")
{
    _prefix_size = _buffer.size();
    _buffer.resize(_prefix_size + 5);
}

void ScoreString::update(int score)
{
    std::to_chars(_buffer.data() + _prefix_size, _buffer.data() + _buffer.size(), score);
}

void ScoreString::reset()
{
    std::fill(_buffer.begin() + _prefix_size, _buffer.end(), 0);
}

const char *ScoreString::get() const
{
    return _buffer.data();
}
