#include <characters/builder.hpp>

#include <GameMap.hpp>

#include <characters/IGameCharacter.hpp>
#include <characters/King.hpp>
#include <characters/Pig.hpp>

std::vector<IGameCharacter*> build_game_characters(SDL_Renderer* renderer, GameMap const& map)
{
    // TODO: Avoid raw pointers
    auto game_characters = std::vector<IGameCharacter*>();
    auto player = ([&map, &renderer]() -> IGameCharacter* {
        for (auto const& info : map.interactables) {
            if (info.id == 0) {
                return new King(renderer, info.position.x, info.position.y);
            }
        }
        return nullptr;
    })();
    if (player) {
        game_characters.push_back(player);
    }

    for (auto const& info : map.interactables) {
        if (info.id == 1) {
            auto pig = new Pig(renderer, info.position.x, info.position.y);
            game_characters.push_back(pig);
        }
    }

    return game_characters;
}
