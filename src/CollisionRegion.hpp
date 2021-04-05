#ifndef __COLLISION_REGION_HPP
#define __COLLISION_REGION_HPP

#include <Vector2D.hpp>

enum class CollisionType {
    TILEMAP_COLLISION = 0
};

enum class CollisionSide {
    LEFT_COLLISION = 0,
    RIGHT_COLLISION = 1,
    TOP_COLLISION = 2,
    BOTTOM_COLLISION = 3
};

struct CollisionRegionInformation
{
    CollisionRegionInformation(
        Vector2D<double> position,
        Vector2D<double> old_position,
        Vector2D<int> collision_size
    )
        : collision_region{
            position.x,
            position.y,
            double(collision_size.x),
            double(collision_size.y)
        }
        , old_collision_region{
            old_position.x,
            old_position.y,
            double(collision_size.x),
            double(collision_size.y)
        }
        {}

    const Region2D<double> collision_region;
    const Region2D<double> old_collision_region;
};

#endif
