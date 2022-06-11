//
//  entity_pool.cpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 16.07.2021.
//

#include "entity_pool.hpp"

#include <algorithm>

using namespace angry;

EntityPool::EntityPool(entt::registry& registry, size_t size)
{
    _idle.reserve(size);
    for (size_t i = 0; i < size; i++)
    {
        _idle.push_back(registry.create());
    }

    _active.reserve(size);
}

std::optional<entt::entity> EntityPool::get_entity()
{
    if (_idle.empty())
    {
        return std::nullopt;
    }

    auto entity = _idle.back();
    _idle.pop_back();
    _active.push_back(entity);

    return entity;
}

const std::vector<entt::entity>& EntityPool::get_idle() const
{
    return _idle;
}
