#ifndef __TILEMAP_COLLISION_HPP
#define __TILEMAP_COLLISION_HPP

class IGameLevel;
class GameMap;
class IGameCharacter;

void compute_tilemap_collisions(GameMap const& map, IGameCharacter* c, IGameLevel& level);

#endif
