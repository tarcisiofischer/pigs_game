#ifndef __COLLISION_REGION_HPP
#define __COLLISION_REGION_HPP

#include <Vector2D.hpp>

struct CollisionRegionInformation {
    CollisionRegionInformation(Vector2D<double> position, Vector2D<double> old_position, Vector2D<int> collision_size)
        : collision_region { position.x, position.y, double(collision_size.x), double(collision_size.y) }
        , old_collision_region { old_position.x, old_position.y, double(collision_size.x), double(collision_size.y) }
    {
    }

    const Region2D<double> collision_region;
    const Region2D<double> old_collision_region;
};

#endif
