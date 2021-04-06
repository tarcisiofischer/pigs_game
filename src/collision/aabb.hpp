#ifndef __AABB_COLLISION_HPP
#define __AABB_COLLISION_HPP

#include <Vector2D.hpp>

inline bool check_aabb_collision(Region2D<double> const& a, Region2D<double> const& b)
{
    return (
        a.x       < b.x + b.w &&
        a.x + a.w > b.x       &&
        a.y       < b.y + b.h &&
        a.y + a.h > b.y
    );
}

#endif
