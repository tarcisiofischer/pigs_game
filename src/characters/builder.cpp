#include <GameMap.hpp>
#include <characters/IGameCharacter.hpp>
#include <characters/Liv.hpp>
#include <characters/Pig.hpp>
#include <items/Key.hpp>
#include <characters/builder.hpp>

std::vector<std::unique_ptr<IGameCharacter>> build_game_characters(SDL_Renderer* renderer, GameMap const& map)
{
    auto game_characters = std::vector<std::unique_ptr<IGameCharacter>>();
    auto player = ([&map, &renderer]() -> std::unique_ptr<IGameCharacter> {
        for (auto const& info : map.interactables) {
            if (info.id == 0) {
                return std::make_unique<Liv>(renderer, info.position.x, info.position.y);
            }
        }
        return nullptr;
    })();
    if (player) {
        game_characters.push_back(std::move(player));
    }

    for (auto const& info : map.interactables) {
        if (info.id == 1) {
            game_characters.push_back(std::make_unique<Pig>(renderer, info.position.x, info.position.y));
        }
        if (info.id == 4) {
            game_characters.push_back(std::make_unique<Key>(renderer, info.position.x, info.position.y));
        }
    }

    return game_characters;
}
