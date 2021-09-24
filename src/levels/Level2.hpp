#ifndef __LVL_2_HPP
#define __LVL_2_HPP

#include <GameMap.hpp>
#include <levels/IGameLevel.hpp>

#include <SDL.h>

#include <map>
#include <vector>
#include <string>

class IGameCharacter;
class TransitionAnimation;
class GameHandler;

class Level2 : public IGameLevel
{
public:
    Level2(GameHandler& game_handler);

    GameMap& get_map() override;
    std::vector<IGameCharacter*>& get_characters() override;
    std::function<void()> get_collision_callback(int callback_collision_id, IGameCharacter* character) override;

private:
    GameMap map;
    std::vector<IGameCharacter*> characters;
    GameHandler& game_handler;
};

#endif
