//
//  entity_pool.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 16.07.2021.
//

#pragma once

#include <optional>
#include <vector>

#include <entt/entt.hpp>

namespace angry
{

class EntityPool final
{
public:
    EntityPool(entt::registry& registry, size_t size);

    std::optional<entt::entity> get_entity();

    const std::vector<entt::entity>& get_idle() const;

    template<class T>
    void release_if(T function)
    {
        auto i = _active.begin();
        while (i != _active.end())
        {
            auto entity = *i;
            if (function(entity))
            {
                _idle.push_back(entity);
                i = _active.erase(i);
            }
            else
            {
                i++;
            }
        }
    }

    template<class T>
    std::optional<entt::entity> release_first_if(T function)
    {
        auto i = _active.begin();
        while (i != _active.end())
        {
            auto entity = *i;
            if (function(entity))
            {
                _idle.push_back(entity);
                i = _active.erase(i);
                return entity;
            }
            else
            {
                i++;
            }
        }
        return std::nullopt;
    }

    template<class T>
    std::optional<entt::entity> find_first_if(T function)
    {
        for (auto entity : _active)
        {
            if (function(entity))
            {
                return entity;
            }
        }
        return std::nullopt;
    }

private:
    std::vector<entt::entity> _idle;
    std::vector<entt::entity> _active;
};

}
