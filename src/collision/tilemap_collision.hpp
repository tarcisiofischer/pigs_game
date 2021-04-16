#ifndef __TILEMAP_COLLISION_HPP
#define __TILEMAP_COLLISION_HPP

#include <characters/IGameCharacter.hpp>
#include <GameMap.hpp>
#include <constants.hpp>
#include <collision/aabb.hpp>
#include <collision/enums.hpp>

#include <array>

void compute_tilemap_collisions(GameMap const& map, IGameCharacter* c);

#endif
