#ifndef __CANNONBALL_HPP
#define __CANNONBALL_HPP

#include <characters/IGameCharacter.hpp>
#include <Vector2D.hpp>

#include <sdl_wrappers.hpp>

#include <tuple>
#include <vector>

class CannonBall : public IGameCharacter {
public:
    enum class CannonBallState {
        active = 0,
        exploding = 1,
        finished = 2
    };
    
    static auto constexpr IDLE_ANIMATION = 0;

    static auto constexpr collision_offset_x = 23.;
    static auto constexpr collision_offset_y = 13.;
    static auto constexpr collision_size_x = 12.;
    static auto constexpr collision_size_y = 12.;

    static auto constexpr collision_size = Vector2D<int>{20, 20};

    static auto constexpr ball_exit_offset_x = 8.;
    static auto constexpr ball_exit_offset_y = 2.;

    CannonBall(SDL_Renderer* renderer, double pos_x, double pos_y)
        : animations()
        , boom_animation(nullptr, {}, 0, 0, 100.)
        , position{pos_x, pos_y}
        , old_position{pos_x, pos_y}
        , velocity{0.0, 0.0}
        , state(CannonBallState::active)
        , renderer(renderer)
        , spritesheet(load_media("assets/sprites/cannonball44x28.png", renderer))
        , boom_spritesheet(load_media("assets/sprites/boom80x80.png", renderer))
    {
        auto register_animation = [&](int id, std::vector<std::tuple<int, int>> const& frames, double time) {
            this->animations.insert(std::make_pair(id, Animation(this->spritesheet, frames, 44, 28, time)));
        };
        register_animation(
            CannonBall::IDLE_ANIMATION,
            {
                {0, 0},
            },
            1000.
        );
        
        this->boom_animation = Animation(
            this->boom_spritesheet,
            {
                {0, 0},
                {1, 0},
                {2, 0},
                {3, 0},
                {4, 0},
                {5, 0},
            },
            80,
            80,
            100.
        );
        
        this->boom_animation.set_on_finish_animation_callback([this](){
            this->state = CannonBallState::finished;
        });
    }

    void update(double elapsedTime) override
    {
        this->old_position = this->position;
        this->position += this->velocity * elapsedTime;
    }
    
    void run_animation(double elapsedTime) override {
        if (this->state == CannonBallState::active) {
            this->animations.at(IDLE_ANIMATION).run(this->renderer, elapsedTime, +1, this->position.as_int(), Vector2D<int>{20, 0}, camera_offset);
        } else if (this->state == CannonBallState::exploding) {
            this->boom_animation.run(this->renderer, elapsedTime, +1, this->position.as_int(), Vector2D<int>{30, 27}, camera_offset);
        }
    }

    void set_position(double x, double y) override {}

    Vector2D<double> get_position() const override {
        return this->position;
    }

    Vector2D<double> get_velocity() const override {
        return this->velocity;
    }

    void set_velocity(double x, double y) override {
        this->velocity.x = x;
        this->velocity.y = y;
    }
    
    void handle_collision(CollisionType const& type, CollisionSide const& side) override {
        if (type == CollisionType::TILEMAP_COLLISION) {
            this->state = CannonBallState::exploding;
        }
    }

    CollisionRegionInformation get_collision_region_information() const override {
        if (this->state == CannonBallState::active || this->state == CannonBallState::exploding) {
            return CollisionRegionInformation(this->position, this->old_position, this->collision_size);
        } else {
            return CollisionRegionInformation({0, 0}, {0, 0}, {0, 0});
        }
    }

    void on_after_collision() override {}

public:
    std::map<int, Animation> animations;
    Animation boom_animation;
    Vector2D<double> position;
    Vector2D<double> old_position;
    Vector2D<double> velocity;
    CannonBallState state;
    SDL_Renderer* renderer;
    SDL_Texture* spritesheet;
    SDL_Texture* boom_spritesheet;
};

#endif
