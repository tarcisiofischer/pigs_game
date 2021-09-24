#ifndef __IGAMELEVEL_HPP
#define __IGAMELEVEL_HPP

#include <characters/IGameCharacter.hpp>
#include <GameMap.hpp>
#include <functional>
#include <vector>

class IGameLevel {
public:
    virtual GameMap& get_map() = 0;
    virtual std::vector<IGameCharacter*>& get_characters() = 0;
    virtual std::function<void()> get_collision_callback(int callback_collision_id, IGameCharacter* character) = 0;
};

#endif
