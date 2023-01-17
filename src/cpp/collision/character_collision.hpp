#ifndef __CHARACTERS_COLLISION
#define __CHARACTERS_COLLISION

#include <collision/aabb.hpp>
#include <vector>
#include <memory>

void compute_characters_collisions(std::vector<std::unique_ptr<IGameCharacter>>& game_characters);

#endif
