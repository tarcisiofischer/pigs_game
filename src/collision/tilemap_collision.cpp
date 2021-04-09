#include <collision/tilemap_collision.hpp>
#include <cmath>

namespace {
    struct TileCollisionInformation {
        Region2D<double> tile_region;
        bool is_collideable;
        CollisionType collision_type;
    };

    TileCollisionInformation tile_info_from_position(
        Tilemap const& tilemap,
        Tilemap const& foreground,
        Vector2D<double> const& position
    )
    {
        int j = std::floor(position.x / TILE_SIZE);
        int i = std::floor(position.y / TILE_SIZE);

        auto tile_world_position = Vector2D<int>{TILE_SIZE * j, TILE_SIZE * i};
        auto tile_region = Region2D<double>{
            double(tile_world_position.x),
            double(tile_world_position.y),
            double(TILE_SIZE),
            double(TILE_SIZE)
        };

        auto is_collideable = false;
        auto collision_type = CollisionType::NO_COLLISION;

        auto tile_id = tilemap[HEIGHT - i - 1][j];
        for (auto&& collision_tile_id : collision_tiles) {
            if (tile_id == collision_tile_id) {
                is_collideable = true;
                collision_type = CollisionType::TILEMAP_COLLISION;
            }
        }

        // Only check foreground collision if no background collision is present
        if (!is_collideable) {
            auto foreground_id = foreground[HEIGHT - i - 1][j];
            for (auto&& collision_tile_id : foreground_collision_tiles) {
                if (foreground_id == collision_tile_id) {
                    is_collideable = true;
                    collision_type = CollisionType::FOREGROUND_COLLISION;
                }
            }
        }

        return {tile_region, is_collideable, collision_type};
    }

    void compute_single_collission(TileCollisionInformation const& info, IGameCharacter* character)
    {
        auto const& tile_region = info.tile_region;
        auto const& collision_type = info.collision_type;

        auto collision_region_info = character->get_collision_region_information();
        auto const& collision_region = collision_region_info.collision_region;
        auto const& old_collision_region = collision_region_info.old_collision_region;
        auto const& current_position = character->get_position();
        auto const& current_velocity = character->get_velocity();

        if (check_aabb_collision(collision_region, tile_region)) {
            if (
                collision_region.x + collision_region.w > tile_region.x &&
                old_collision_region.x + old_collision_region.w <= tile_region.x
            ) {
                if (collision_type == CollisionType::TILEMAP_COLLISION) {
                    character->set_position(tile_region.x - collision_region.w - 0.1, current_position.y);
                    character->set_velocity(0.0, current_velocity.y);
                }
                character->handle_collision(collision_type, CollisionSide::RIGHT_COLLISION);
            } else if (
                collision_region.x < tile_region.x + tile_region.w &&
                old_collision_region.x >= tile_region.x + tile_region.w
            ) {
                if (collision_type == CollisionType::TILEMAP_COLLISION) {
                    character->set_position(tile_region.x + tile_region.w + 0.1, current_position.y);
                    character->set_velocity(0.0, current_velocity.y);
                }
                character->handle_collision(collision_type, CollisionSide::LEFT_COLLISION);
            } else if (
                collision_region.y < tile_region.y + tile_region.h &&
                old_collision_region.y >= tile_region.y + tile_region.h
            ) {
                character->set_position(current_position.x, tile_region.y + tile_region.h + 0.1);
                character->set_velocity(current_velocity.x, 0.0);
                character->handle_collision(collision_type, CollisionSide::BOTTOM_COLLISION);
            } else if (
                collision_region.y + collision_region.h > tile_region.y &&
                old_collision_region.y + old_collision_region.h <= tile_region.y
            ) {
                if (collision_type == CollisionType::TILEMAP_COLLISION) {
                    character->set_position(current_position.x, tile_region.y - collision_region.h - 0.1);
                    character->set_velocity(current_velocity.x, 0.0);
                }
                character->handle_collision(collision_type, CollisionSide::TOP_COLLISION);
            }
        }
    }
}

void compute_tilemap_collisions(Tilemap const& tilemap, Tilemap const& foreground, IGameCharacter* character)
{
    auto collision_region_info = character->get_collision_region_information();
    auto const& collision_region = collision_region_info.collision_region;
    auto const& old_collision_region = collision_region_info.old_collision_region;

    // Left-bottom
    {
        auto tile_collision_info = tile_info_from_position(tilemap, foreground, {collision_region.x, collision_region.y});
        if (tile_collision_info.is_collideable) {
            compute_single_collission(tile_collision_info, character);
        }
    }

    // Right-bottom
    {
        auto tile_collision_info = tile_info_from_position(tilemap, foreground, {collision_region.x + collision_region.w, collision_region.y});
        if (tile_collision_info.is_collideable) {
            compute_single_collission(tile_collision_info, character);
        }
    }

    // Left-top
    {
        auto tile_collision_info = tile_info_from_position(tilemap, foreground, {collision_region.x, collision_region.y + collision_region.h});
        if (tile_collision_info.is_collideable) {
            compute_single_collission(tile_collision_info, character);
        }
    }

    // Left-top
    {
        auto tile_collision_info = tile_info_from_position(tilemap, foreground, {collision_region.x + collision_region.w, collision_region.y + collision_region.h});
        if (tile_collision_info.is_collideable) {
            compute_single_collission(tile_collision_info, character);
        }
    }
    
    character->on_after_collision();
}

