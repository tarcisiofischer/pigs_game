#ifndef __LIV_HPP
#define __LIV_HPP

#include <Animation.hpp>
#include <StateTimeout.hpp>
#include <Vector2D.hpp>
#include <characters/IGameCharacter.hpp>
#include <functional>
#include <optional>
#include <random.hpp>
#include <sdl_wrappers.hpp>
#include <GameController.hpp>

extern Vector2D<int> camera_offset;

class Liv : public IGameCharacter {
public:
    static auto constexpr IDLE_ANIMATION = 0;
    static auto constexpr RUNNING_ANIMATION = 1;
    static auto constexpr JUMPING_ANIMATION = 2;
    static auto constexpr FALLING_ANIMATION = 3;
    static auto constexpr ATTACKING_ANIMATION = 4;
    static auto constexpr JUST_TOUCHED_GROUND_ANIMATION = 5;
    static auto constexpr TAKING_DAMAGE_ANIMATION = 6;
    static auto constexpr DYING_ANIMATION = 7;
    static auto constexpr DEAD_ANIMATION = 8;
    static auto constexpr DASHING_ANIMATION = 9;
    static auto constexpr FRAME_SIZE_X = 23;
    static auto constexpr FRAME_SIZE_Y = 26;
    static auto constexpr collision_size = Vector2D<int> { 12, 20 };
    static auto constexpr spritesheet_offset = Vector2D<int> { 6, 2 };
    static auto constexpr walk_speed = 0.1;
    static auto constexpr dash_speed = 0.25;
    static auto constexpr jump_speed = 0.3;
    static auto constexpr double_jump_speed = 0.25;
    static auto constexpr reset_dash_timeout = 200.0;
    static auto constexpr reset_no_dash_timeout = 500.0;

public:
    Liv(SDL_Renderer* renderer, double pos_x, double pos_y);
    void set_position(double x, double y) override;
    [[nodiscard]] Vector2D<double> get_position() const override;
    [[nodiscard]] Vector2D<double> get_velocity() const override;
    void set_velocity(double x, double y) override;
    [[nodiscard]] CollisionRegionInformation get_collision_region_information() const override;
    void handle_collision(CollisionType const& type, CollisionSide const& side) override;
    void on_after_collision() override;
    void handle_controller(GameController const& controller);
    void register_on_dead_callback(std::function<void()> const& f);
    void update(double elapsedTime) override;
    void start_taking_damage();
    void run_animation(double elapsedTime) override;
    [[nodiscard]] Region2D<double> attack_region() const;

public:
    int running_side;
    std::map<int, Animation> animations;
    std::vector<std::function<void(SDL_Renderer*, IGameCharacter*, double)>> on_after_run_animation_callbacks;
    StateTimeout after_taking_damage_timeout;
    int face;
    int life;
    Vector2D<double> old_position;
    Vector2D<double> position;
    Vector2D<double> velocity;
    SDL_Renderer* renderer;
    SDL_Texture* spritesheet;
    bool is_jumping;
    bool is_falling;
    bool start_jumping;
    bool is_grounded;
    bool just_touched_ground;
    bool is_taking_damage;
    bool after_taking_damage;
    bool is_dying;
    bool is_dead;
    bool start_dashing;
    double dashing_timeout;
    double no_dash_timeout;
    int jump_count;
    std::optional<std::function<void()>> on_dead_callback;
    std::optional<std::function<void()>> on_start_taking_damage;
    std::optional<std::function<void()>> on_start_dashing;
};

#endif
