#ifndef __IGAMELEVEL_HPP
#define __IGAMELEVEL_HPP

#include <GameMap.hpp>
#include <characters/IGameCharacter.hpp>
#include <functional>
#include <vector>
#include <memory>

class IGameLevel {
public:
    virtual GameMap& get_map() = 0;
    virtual std::vector<std::unique_ptr<IGameCharacter>>& get_characters() = 0;
    virtual std::function<void()> get_collision_callback(int callback_collision_id, IGameCharacter* character) = 0;
};

#endif
