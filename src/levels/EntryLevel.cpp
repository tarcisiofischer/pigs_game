#include <levels/EntryLevel.hpp>

#include <characters/builder.hpp>
#include <characters/IGameCharacter.hpp>
#include <characters/Pig.hpp>

#include <TransitionAnimation.hpp>
#include <io.hpp>
#include <GameHandler.hpp>

EntryLevel::EntryLevel(GameHandler& game_handler)
    : map(load_map("maps/entry_level.map"))
    , characters(build_game_characters(game_handler.get_renderer(), map))
{
}

GameMap& EntryLevel::get_map()
{
    return this->map;
}

std::vector<IGameCharacter*>& EntryLevel::get_characters()
{
    return this->characters;
}
