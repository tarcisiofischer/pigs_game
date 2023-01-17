#ifndef __CHARACTER_BUILDER_HPP
#define __CHARACTER_BUILDER_HPP

#include <vector>
#include <memory>

class IGameCharacter;
class SDL_Renderer;
class GameMap;

std::vector<std::unique_ptr<IGameCharacter>> build_game_characters(SDL_Renderer* renderer, GameMap const& map);

#endif
