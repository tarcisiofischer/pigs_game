#ifndef __CHARACTERS_COLLISION
#define __CHARACTERS_COLLISION

#include <collision/aabb.hpp>
#include <vector>

class King;
class Pig;
class Cannon;
class CannonBall;
class IGameCharacter;

void pig_king_collision(Pig* pig_ptr, King* king);
void cannon_king_collision(Cannon* cannon_ptr, King* king);
void cannonball_king_collision(CannonBall* cannon_ptr, King* king);
void cannonball_pig_collision(CannonBall* cannon_ptr, Pig* pig_ptr);

void compute_characters_collisions(std::vector<IGameCharacter*>& game_characters);

#endif
