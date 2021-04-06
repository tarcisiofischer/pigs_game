#ifndef __TILEMAP_COLLISION_HPP
#define __TILEMAP_COLLISION_HPP

#include <characters/IGameCharacter.hpp>
#include <constants.hpp>
#include <collision/aabb.hpp>
#include <collision/common.hpp>
#include <collision/enums.hpp>

#include <array>

auto constexpr collision_tiles = std::array<int, 10>{1, 3, 12, 14, 15, 25, 27, 37, 38, 39};

void compute_tilemap_collisions(Tilemap const& tilemap, IGameCharacter* c)
{
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            auto tile_id = tilemap[i][j];
            for (auto&& collision_tile_id : collision_tiles) {
                if (tile_id == collision_tile_id) {
                    auto collision_region_info = c->get_collision_region_information();
                    auto const& collision_region = collision_region_info.collision_region;

                    auto tile_world_position = Vector2D<int>{TILE_SIZE * j, TILE_SIZE * (HEIGHT - i - 1)};
                    auto tile_region = Region2D<double>{double(tile_world_position.x), double(tile_world_position.y), double(TILE_SIZE), double(TILE_SIZE)};

                    if (check_aabb_collision(collision_region, tile_region)) {
                        auto const& old_collision_region = collision_region_info.old_collision_region;
                        auto current_position = c->get_position();
                        auto current_velocity = c->get_velocity();

                        if (
                            collision_region.x + collision_region.w > tile_region.x &&
                            old_collision_region.x + old_collision_region.w <= tile_region.x
                        ) {
                            c->set_position(tile_region.x - collision_region.w - 0.1, current_position.y);
                            c->set_velocity(0.0, current_velocity.y);

                            c->handle_collision(CollisionType::TILEMAP_COLLISION, CollisionSide::RIGHT_COLLISION);
                        } else if (
                            collision_region.x < tile_region.x + tile_region.w &&
                            old_collision_region.x >= tile_region.x + tile_region.w
                        ) {
                            c->set_position(tile_region.x + tile_region.w + 0.1, current_position.y);
                            c->set_velocity(0.0, current_velocity.y);

                            c->handle_collision(CollisionType::TILEMAP_COLLISION, CollisionSide::LEFT_COLLISION);
                        } else if (
                            collision_region.y < tile_region.y + tile_region.h &&
                            old_collision_region.y >= tile_region.y + tile_region.h
                        ) {
                            c->set_position(current_position.x, tile_region.y + tile_region.h + 0.1);
                            c->set_velocity(current_velocity.x, 0.0);

                            c->handle_collision(CollisionType::TILEMAP_COLLISION, CollisionSide::BOTTOM_COLLISION);
                        } else if (
                            collision_region.y + collision_region.h > tile_region.y &&
                            old_collision_region.y + old_collision_region.h <= tile_region.y
                        ) {
                            c->set_position(current_position.x, tile_region.y - collision_region.h - 0.1);
                            c->set_velocity(current_velocity.x, 0.0);
                            
                            c->handle_collision(CollisionType::TILEMAP_COLLISION, CollisionSide::TOP_COLLISION);
                        }
                    }
                }
            }
        }
    }
    
    c->on_after_collision();
}

#endif
