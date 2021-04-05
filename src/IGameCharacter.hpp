#ifndef __GAME_CHARACTER_INTERFACE_HPP
#define __GAME_CHARACTER_INTERFACE_HPP

#include <Vector2D.hpp>
#include <CollisionRegion.hpp>

class IGameCharacter
{
public:
    virtual void update(double elapsedTime) = 0;
    virtual void run_animation(double elapsedTime) = 0;
    virtual void set_position(double x, double y) = 0;
    virtual Vector2D<double> get_position() const = 0;
    virtual Vector2D<double> get_velocity() const = 0;
    virtual void set_velocity(double x, double y) = 0;
    virtual void handle_collision(CollisionType const& type, CollisionSide const& side) = 0;
    virtual CollisionRegionInformation get_collision_region_information() const = 0;
    virtual void on_after_collision() = 0;
};

#endif
