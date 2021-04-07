#ifndef __PIG_WITH_MATCHES_HPP
#define __PIG_WITH_MATCHES_HPP

#include <characters/IGameCharacter.hpp>
#include <characters/Cannon.hpp>

#include <Animation.hpp>
#include <Vector2D.hpp>

#include <random.hpp>
#include <sdl_wrappers.hpp>

extern Vector2D<int> camera_offset;

class PigWithMatches : public IGameCharacter {
public:
    static auto constexpr IDLE_ANIMATION = 0;
    static auto constexpr ACTIVATE_CANNON = 1;
    static auto constexpr PREPARE_NEXT_MATCH = 2;
    static auto constexpr MATCH_ON_HAND = 3;

    static auto constexpr DEFAULT_THINK_TIMEOUT = 500.0;

    static auto constexpr collision_size = Vector2D<int>{18, 18};

    PigWithMatches(SDL_Renderer* renderer, double pos_x, double pos_y, int face, Cannon& cannon);

    void set_position(double x, double y) override;
    Vector2D<double> get_position() const override;
    void set_velocity(double x, double y) override;
    Vector2D<double> get_velocity() const override;
    CollisionRegionInformation get_collision_region_information() const override;
    void handle_collision(CollisionType const& type, CollisionSide const& side) override;
    void on_after_collision() override;
    void update(double elapsedTime) override;
    void run_animation(double elapsedTime) override;
    void think(double elapsedTime);
    
public:
    std::map<int, Animation> animations;
    int face;
    Vector2D<double> position;
    Vector2D<double> old_position;
    Vector2D<double> velocity;
    SDL_Renderer* renderer;
    SDL_Texture* spritesheet;
    double think_timeout;
    bool start_attack;
    bool preparing_next_match;
    Cannon& cannon;
};

#endif
