#ifndef __CANNON_HPP
#define __CANNON_HPP

#include <Animation.hpp>
#include <Vector2D.hpp>
#include <characters/IGameCharacter.hpp>
#include <sdl_wrappers.hpp>
#include <tuple>
#include <vector>

class Cannon : public IGameCharacter {
public:
    static auto constexpr IDLE_ANIMATION = 0;
    static auto constexpr ATTACKING_ANIMATION = 1;

    static auto constexpr collision_offset_x = 35.;
    static auto constexpr collision_offset_y = 43.;
    static auto constexpr collision_size = Vector2D<int> { 24, 21 };

    static auto constexpr SPRITESHEET_OFFSET = Vector2D<int> { 37, 32 };

    Cannon(SDL_Renderer* renderer, double pos_x, double pos_y, int face);
    void set_on_before_fire(std::function<void()> const& f);
    void update(double elapsedTime) override;
    void set_position(double x, double y) override;
    Vector2D<double> get_position() const override;
    Vector2D<double> get_velocity() const override;
    void set_velocity(double x, double y) override;
    CollisionRegionInformation get_collision_region_information() const override;
    void handle_collision(CollisionType const& type, CollisionSide const& side) override;
    void on_after_collision() override;
    void trigger_attack();
    void run_animation(double elapsedTime, Vector2D<int> const& camera_offset) override;

public:
    std::map<int, Animation> animations;
    int face;
    Vector2D<double> position;
    bool is_attacking;
    SDL_Renderer* renderer;
    SDL_Texture* spritesheet;
    std::optional<std::function<void()>> on_before_fire;
};

#endif
