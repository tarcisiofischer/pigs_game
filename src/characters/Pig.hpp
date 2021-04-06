#ifndef __PIG_HPP
#define __PIG_HPP

#include <characters/IGameCharacter.hpp>

#include <Animation.hpp>
#include <Vector2D.hpp>

#include <random.hpp>
#include <sdl_wrappers.hpp>

// TODO: Remove this
extern Vector2D<int> camera_offset;

class Pig : public IGameCharacter {
public:
    static auto constexpr IDLE_ANIMATION = 0;
    static auto constexpr RUNNING_ANIMATION = 1;
    static auto constexpr TAKING_DAMAGE_ANIMATION = 2;
    static auto constexpr DYING_ANIMATION = 3;

    static auto constexpr collision_offset_x = 30.;
    static auto constexpr collision_offset_y = 30.;
    static auto constexpr collision_size = Vector2D<int>{18, 18};
    static auto constexpr spritesheet_offset = Vector2D<int>{31, 33};

    Pig(Pig const& other)
    {
        this->running_side = other.running_side;

        // Force callbacks re-connection, so that the lambda functions are up-to-date in relation to the lambda captures
        this->animations = other.animations;
        this->connect_callbacks();

        this->face = other.face;
        this->position = other.position;
        this->old_position = other.old_position;
        this->velocity = other.velocity;
        this->renderer = other.renderer;
        this->spritesheet = other.spritesheet;
        this->think_timeout = other.think_timeout;
        this->is_taking_damage = other.is_taking_damage;
        this->life = other.life;
        this->is_dying = other.is_dying;
        this->is_dead = other.is_dead;
    }
    
    virtual ~Pig() {} 

    Pig& operator=(Pig const& other)
    {
        this->running_side = other.running_side;

        // Force callbacks re-connection, so that the lambda functions are up-to-date in relation to the lambda captures
        this->animations = other.animations;
        this->connect_callbacks();

        this->face = other.face;
        this->position = other.position;
        this->old_position = other.old_position;
        this->velocity = other.velocity;
        this->renderer = other.renderer;
        this->spritesheet = other.spritesheet;
        this->think_timeout = other.think_timeout;
        this->is_taking_damage = other.is_taking_damage;
        this->life = other.life;
        this->is_dying = other.is_dying;
        this->is_dead = other.is_dead;

        return *this;
    }

    Pig(SDL_Renderer* renderer, double pos_x, double pos_y)
        : running_side(0)
        , position{pos_x, pos_y}
        , old_position{pos_x, pos_y}
        , velocity{0.0, 0.0}
        , renderer(renderer)
        , spritesheet(load_media("assets/sprites/pig80x80.png", renderer))
        , think_timeout(1000.)
        , is_taking_damage(false)
        , life(2)
        , is_dying(false)
        , is_dead(false)
    {
        auto register_animation = [&](int id, std::vector<std::tuple<int, int>> const& frames, double time) {
            this->animations.insert(std::make_pair(id, Animation(this->spritesheet, frames, 80, 80, time)));
        };
        
        register_animation(
            Pig::IDLE_ANIMATION, 
            {
                {3, 3},
                {4, 3},
                {5, 3},
                {0, 4},
                {1, 4},
            },
            100.
        );
        register_animation(
            Pig::RUNNING_ANIMATION,
            {
                {2, 2},
                {3, 2},
                {4, 2},
                {5, 2},
                {0, 3},
                {1, 3},
                {2, 3},
            },
            100.
        );
        register_animation(
            Pig::TAKING_DAMAGE_ANIMATION,
            {
                {5, 0},
                {6, 0},
                {0, 1},
                {6, 0},
                {0, 1},
            },
            100.
        );
        register_animation(
            Pig::DYING_ANIMATION,
            {
                {3, 0},
                {2, 0},
                {1, 0},
                {1, 0},
                {1, 0},
            },
            100.
        );

        this->connect_callbacks();
    }

    void set_position(double x, double y) override
    {
        this->position.x = x;
        this->position.y = y;
    }

    Vector2D<double> get_position() const override
    {
        return this->position;
    }
    
    Vector2D<double> get_velocity() const override
    {
        return this->velocity;
    }
    
    void set_velocity(double x, double y) override
    {
        this->velocity.y = x;
        this->velocity.y = y;
    }

    CollisionRegionInformation get_collision_region_information() const override
    {
        return CollisionRegionInformation(this->position, this->old_position, this->collision_size);
    }
    
    void handle_collision(CollisionType const& type, CollisionSide const& side) override {}
    void on_after_collision() override {}
    
    void update(double elapsedTime) override
    {
        // velocity x setup
        if (!this->is_taking_damage && !this->is_dead && !this->is_dying) {
            this->think(elapsedTime);
        } else {
            this->running_side = 0;
        }

        if (this->running_side == +1) this->velocity.x = +0.05;
        else if (this->running_side == -1) this->velocity.x = -0.05;
        else this->velocity.x = 0.0;
        this->velocity.y = velocity.y + gravity * elapsedTime;

        // Position setup
        this->old_position = this->position;
        this->position += this->velocity * elapsedTime;
    }
    
    void start_taking_damage()
    {
        this->velocity.x = 0.05;
        this->velocity.y = -0.1;
        this->is_taking_damage = true;
        if (this->on_start_taking_damage) {
            (*this->on_start_taking_damage)();
        }
    }
    
    void run_animation(double elapsedTime) override
    {
        auto current_animation = ([this]() {
            if (this->is_dying) {
                return DYING_ANIMATION;
            }
            if (this->is_taking_damage) {
                return TAKING_DAMAGE_ANIMATION;
            }
            if (this->running_side != 0) {
                return RUNNING_ANIMATION;
            }
            return IDLE_ANIMATION;
        })();
        this->animations.at(current_animation).run(
            this->renderer,
            elapsedTime,
            -this->face,
            Vector2D<int>{int(this->position.x), int(this->position.y)},
            this->spritesheet_offset,
            camera_offset
        );
    }

    void think(double elapsedTime)
    {
        this->think_timeout -= elapsedTime;
        if (this->think_timeout <= 0.) {
            switch (random_int(0, 2)) {
                case 0: {
                    this->running_side = -1;
                    this->face = -1;
                    break;
                };
                case 1: {
                    this->running_side = 0;
                    break;
                };
                case 2: {
                    this->running_side = +1;
                    this->face = +1;
                    break;
                };
            }
            this->think_timeout = 1000.;
        }
    }

private:
    void connect_callbacks()
    {
        this->animations.at(TAKING_DAMAGE_ANIMATION).set_on_finish_animation_callback([this]() {
            this->is_taking_damage = false;
            this->life -= 1;
            if (this->life <= 0) {
                this->is_dying = true;
            }
        });
        this->animations.at(DYING_ANIMATION).set_on_finish_animation_callback([this]() {
            this->is_dead = true;
        });
    }

public:
    int running_side;
    std::map<int, Animation> animations;
    int face;
    Vector2D<double> position;
    Vector2D<double> old_position;
    Vector2D<double> velocity;
    SDL_Renderer* renderer;
    SDL_Texture* spritesheet;
    double think_timeout;
    bool is_taking_damage;
    int life;
    bool is_dying;
    bool is_dead;
    std::optional<std::function<void()>> on_start_taking_damage;
};

#endif
