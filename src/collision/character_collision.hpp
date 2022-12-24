#ifndef __CHARACTERS_COLLISION
#define __CHARACTERS_COLLISION

#include <collision/aabb.hpp>
#include <vector>

void compute_characters_collisions(std::vector<IGameCharacter*>& game_characters);

#endif
