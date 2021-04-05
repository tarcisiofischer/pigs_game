#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <algorithm>
#include <iostream>
#include <functional>
#include <optional>
#include <vector>
#include <tuple>
#include <memory>
#include <random>
#include <map>
#include <variant>

#include <Animation.hpp>
#include <IGameCharacter.hpp>
#include <Vector2D.hpp>
#include <constants.hpp>

std::vector<std::string> debug_messages;

using Tilemap = std::array<std::array<int, WIDTH>, HEIGHT>;

// TODO: Initialize the camera on main (avoid global)
auto camera_offset = Vector2D<int>{0, 0};

class TransitionAnimation
{
public:
    static auto constexpr BLACKING = 0;
    static auto constexpr WAITING = 1;
    static auto constexpr CLEARING = 2;
    static auto constexpr FINISHED = 3;

    TransitionAnimation()
        : animation_state(FINISHED)
        , wait_timeout(0.0)
        , transition_acceleration(0.0)
        , transition_velocity(0.0)
        , transition_width(0.0)
    {
    }

    void reset()
    {
        this->animation_state = BLACKING;
        this->wait_timeout = 500.0;
        this->transition_acceleration = 0.01;
        this->transition_velocity = 0.0;
        this->transition_width = 0.0;
    }
    
    void register_transition_callback(std::function<void()> const& f)
    {
        this->transition_callback = f;
    }
    
    void run(SDL_Renderer* renderer, double elapsedTime)
    {
        if (this->animation_state == BLACKING) {
            this->transition_velocity += this->transition_acceleration * elapsedTime;
            this->transition_width += this->transition_velocity * elapsedTime;

            if (this->transition_width >= SCREEN_WIDTH) {
                this->transition_width = SCREEN_WIDTH;
                this->transition_velocity = 0.0;
                this->animation_state = WAITING;
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            auto rect = SDL_Rect{0, 0, int(this->transition_width), SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &rect);
        } else if (this->animation_state == WAITING) {
            if (this->transition_callback) {
                (*this->transition_callback)();
            }

            this->wait_timeout -= elapsedTime;
            if (this->wait_timeout <= 0.0) {
                this->animation_state = CLEARING;
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            auto rect = SDL_Rect{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &rect);
        } else if (this->animation_state == CLEARING) {
            this->transition_velocity += this->transition_acceleration * elapsedTime;
            this->transition_width -= this->transition_velocity * elapsedTime;
            if (this->transition_width <= 0.0) {
                this->transition_width = 0.0;
                this->transition_velocity = 0.0;
                this->animation_state = FINISHED;
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            auto rect = SDL_Rect{SCREEN_WIDTH - int(this->transition_width), 0, int(this->transition_width), SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &rect);
        }

    }

private:
    int animation_state;
    double wait_timeout;
    double transition_acceleration;
    double transition_velocity;
    double transition_width;
    std::optional<std::function<void()>> transition_callback;
};

inline int random_int(int a, int b)
{
    auto random_device = std::random_device();
    auto g = std::mt19937(random_device());
    std::uniform_int_distribution<> distrib(a, b);
    return distrib(g);
}

inline bool check_aabb_collision(Region2D<double> const& a, Region2D<double> const& b)
{
    return (
        a.x       < b.x + b.w &&
        a.x + a.w > b.x       &&
        a.y       < b.y + b.h &&
        a.y + a.h > b.y
    );
}

void compute_tilemap_collisions(Tilemap const& tilemap, IGameCharacter* c)
{
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            auto tile_id = tilemap[i][j];
            for (auto&& collision_tile_id : collision_tiles) {
                if (tile_id == collision_tile_id) {
                    auto collision_region_info = c->get_collision_region_information();
                    auto const& collision_region = collision_region_info.collision_region;

                    auto tile_world_position = Vector2D<int>{TILE_SIZE * j, TILE_SIZE * (HEIGHT - i - 1)};
                    auto tile_region = Region2D<double>{double(tile_world_position.x), double(tile_world_position.y), double(TILE_SIZE), double(TILE_SIZE)};

                    if (check_aabb_collision(collision_region, tile_region)) {
                        auto const& old_collision_region = collision_region_info.old_collision_region;
                        auto current_position = c->get_position();
                        auto current_velocity = c->get_velocity();

                        if (
                            collision_region.x + collision_region.w > tile_region.x &&
                            old_collision_region.x + old_collision_region.w <= tile_region.x
                        ) {
                            c->set_position(tile_region.x - collision_region.w - 0.1, current_position.y);
                            c->set_velocity(0.0, current_velocity.y);

                            c->handle_collision(CollisionType::TILEMAP_COLLISION, CollisionSide::RIGHT_COLLISION);
                        } else if (
                            collision_region.x < tile_region.x + tile_region.w &&
                            old_collision_region.x >= tile_region.x + tile_region.w
                        ) {
                            c->set_position(tile_region.x + tile_region.w + 0.1, current_position.y);
                            c->set_velocity(0.0, current_velocity.y);

                            c->handle_collision(CollisionType::TILEMAP_COLLISION, CollisionSide::LEFT_COLLISION);
                        } else if (
                            collision_region.y < tile_region.y + tile_region.h &&
                            old_collision_region.y >= tile_region.y + tile_region.h
                        ) {
                            c->set_position(current_position.x, tile_region.y + tile_region.h + 0.1);
                            c->set_velocity(current_velocity.x, 0.0);

                            c->handle_collision(CollisionType::TILEMAP_COLLISION, CollisionSide::BOTTOM_COLLISION);
                        } else if (
                            collision_region.y + collision_region.h > tile_region.y &&
                            old_collision_region.y + old_collision_region.h <= tile_region.y
                        ) {
                            c->set_position(current_position.x, tile_region.y - collision_region.h - 0.1);
                            c->set_velocity(current_velocity.x, 0.0);
                            
                            c->handle_collision(CollisionType::TILEMAP_COLLISION, CollisionSide::TOP_COLLISION);
                        }
                    }
                }
            }
        }
    }
    
    c->on_after_collision();
}

SDL_Texture* load_media(std::string const& filename, SDL_Renderer* renderer)
{
    auto* surface = IMG_Load(filename.c_str());
    if (surface == nullptr) {
        std::cout << "WARNING: Unable to load image. SDL Error: " << SDL_GetError() << std::endl;
    }
    auto* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr){
        std::cout << "WARNING: Unable to create texture from image. SDL Error: " << SDL_GetError() << std::endl;
    }
    SDL_FreeSurface(surface);
    return texture;
}

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

class StateTimeout {
public:
    StateTimeout()
        : timeout(0.0)
        , function()
        , current_time(0.0)
        , started(false)
        {}
    
    StateTimeout(double timeout, std::function<void()> const& f)
        : timeout(timeout)
        , function(f)
        , current_time(0.0)
        , started(false)
        {}


    void restart()
    {
        this->current_time = timeout;
        this->started = true;
    }
    
    void update(double elapsedTime)
    {
        if (!this->started) {
            return;
        }

        this->current_time -= elapsedTime;
        if (this->current_time <= 0.0) {
            this->function();
            this->started = false;
        }
    }

private:
    double timeout;
    std::function<void()> function;
    double current_time;
    bool started;
};

class Cannon : public IGameCharacter {
public:
    static auto constexpr IDLE_ANIMATION = 0;
    static auto constexpr ATTACKING_ANIMATION = 1;

    static auto constexpr collision_offset_x = 35.;
    static auto constexpr collision_offset_y = 43.;
    static auto constexpr collision_size = Vector2D<int>{24, 21};
    
    static auto constexpr spritesheet_offset = Vector2D<int>{37, 32};

    Cannon(SDL_Renderer* renderer, double pos_x, double pos_y, int face)
        : face(face)
        , position{pos_x, pos_y}
        , is_attacking(false)
        , renderer(renderer)
        , spritesheet(load_media("assets/sprites/cannon96x96.png", renderer))
    {
        auto register_animation = [&](int id, std::vector<std::tuple<int, int>> const& frames, double time) {
            this->animations.insert(std::make_pair(id, Animation(this->spritesheet, frames, 96, 96, time)));
        };
        
        register_animation(
            Cannon::IDLE_ANIMATION, 
            {
                {0, 0},
            },
            100.
        );
        register_animation(
            Cannon::ATTACKING_ANIMATION,
            {
                {1, 0},
                {2, 0},
                {3, 0},
                {4, 0},
            },
            100.
        );
    }
    
    void set_on_before_fire(std::function<void()> const& f)
    {
        this->on_before_fire = f;
    }

    void update(double elapsedTime) override
    {
    }

    void set_position(double x, double y) override {}

    Vector2D<double> get_position() const override
    {
        return this->position;
    }

    Vector2D<double> get_velocity() const override
    {
        return {0.0, 0.0};
    }

    void set_velocity(double x, double y) override {}

    CollisionRegionInformation get_collision_region_information() const override
    {
        return CollisionRegionInformation(this->position, this->position, this->collision_size);
    }

    void handle_collision(CollisionType const& type, CollisionSide const& side) override {}
    
    void on_after_collision() override {}
    
    void trigger_attack()
    {
        if (!this->is_attacking) {
            this->is_attacking = true;
            if (this->on_before_fire) {
                (*this->on_before_fire)();
            }
            this->animations.at(ATTACKING_ANIMATION).set_on_finish_animation_callback([this]() {
                this->is_attacking = false;
            });
        }
    }
    
    void run_animation(double elapsedTime) override
    {
        auto current_animation = ([this]() {
            if (this->is_attacking) {
               return ATTACKING_ANIMATION;
            }
            return IDLE_ANIMATION;
        })();

        this->animations.at(current_animation).run(
            this->renderer,
            elapsedTime,
            this->face,
            Vector2D<int>{int(this->position.x), int(this->position.y)},
            this->spritesheet_offset,
            camera_offset
        );
    }

public:
    std::map<int, Animation> animations;
    int face;
    Vector2D<double> position;
    bool is_attacking;
    SDL_Renderer* renderer;
    SDL_Texture* spritesheet;
    std::optional<std::function<void()>> on_before_fire;
};

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

    static auto constexpr exploding_collision_offset_x = 10.;
    static auto constexpr exploding_collision_offset_y = -10.;
    static auto constexpr collision_size = Vector2D<int>{40, 40};
    static auto constexpr exploding_collision_size = Vector2D<int>{60, 60};

    static auto constexpr ball_exit_offset_x = 23.;
    static auto constexpr ball_exit_offset_y = 33.;

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
            this->animations.at(IDLE_ANIMATION).run(this->renderer, elapsedTime, +1, this->position.as_int(), Vector2D<int>{0, 0}, camera_offset);
        } else if (this->state == CannonBallState::exploding) {
            this->boom_animation.run(this->renderer, elapsedTime, +1, this->position.as_int(), Vector2D<int>{-8, -28}, camera_offset);
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
        if (this->state == CannonBallState::active) {
            return CollisionRegionInformation(this->position, this->old_position, this->collision_size);
        } else if (this->state == CannonBallState::exploding) {
            return CollisionRegionInformation(this->position, this->old_position, this->exploding_collision_size);
        } else {
            return CollisionRegionInformation(this->position, this->old_position, {0, 0});
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

/*
class PigWithMatches : public IGameCharacter {
public:
    static auto constexpr IDLE_ANIMATION = 0;
    static auto constexpr ACTIVATE_CANNON = 1;
    static auto constexpr PREPARE_NEXT_MATCH = 2;
    static auto constexpr MATCH_ON_HAND = 3;

    static auto constexpr DEFAULT_THINK_TIMEOUT = 5000.0;

    static auto constexpr collision_offset_x = 30.;
    static auto constexpr collision_offset_y = 30.;
    static auto constexpr collision_size_x = 18.;
    static auto constexpr collision_size_y = 18.;

    PigWithMatches(SDL_Renderer* renderer, double pos_x, double pos_y, int face, Cannon& cannon)
        : face(face)
        , pos_x(pos_x)
        , pos_y(pos_y)
        , velocity_x(0.0)
        , velocity_y(0.0)
        , renderer(renderer)
        , spritesheet(load_media("assets/sprites/pig_with_match96x96.png", renderer))
        , old_pos_x(pos_x)
        , old_pos_y(pos_y)
        , think_timeout(PigWithMatches::DEFAULT_THINK_TIMEOUT)
        , start_attack(false)
        , preparing_next_match(false)
        , cannon(cannon)
    {
        auto register_animation = [&](int id, std::vector<std::tuple<int, int>> const& frames, double time) {
            this->animations.insert(std::make_pair(id, Animation(this->spritesheet, frames, 96, 96, time)));
        };
        
        register_animation(
            PigWithMatches::IDLE_ANIMATION,
            {
                {0, 3},
                {1, 3},
                {2, 3},
                {1, 3},
            },
            200.
        );
        register_animation(
            PigWithMatches::ACTIVATE_CANNON, 
            {
                {0, 0},
                {1, 0},
                {2, 0},
                {0, 1},
                {1, 1},
                {2, 1},
                {0, 2},
                {1, 2},
                {1, 2},
                {1, 2},
                {1, 2},
                {2, 2},
            },
            100.
        );
        this->animations.at(PigWithMatches::ACTIVATE_CANNON).set_on_finish_animation_callback([this]() {
            this->start_attack = false;
            this->cannon.trigger_attack();
        });
    }

    void set_position(double x, double y) override
    {
        this->pos_x = x;
        this->pos_y = y;
    }

    Vector2D<double> get_position() const override
    {
        return {this->pos_x, this->pos_y};
    }
    
    Vector2D<double> get_velocity() const override
    {
        return {this->velocity_x, this->velocity_y};
    }
    
    void set_velocity(double x, double y) override
    {
        this->velocity_x = x;
        this->velocity_y = y;
    }

    CollisionRegionInformation get_collision_region_information() const override
    {
        return CollisionRegionInformation{
            {
                this->pos_x + this->collision_offset_x,
                this->pos_y + this->collision_offset_y,
                this->collision_size_x,
                this->collision_size_y
            },
            {
                this->old_pos_x + this->collision_offset_x,
                this->old_pos_y + this->collision_offset_y,
                this->collision_size_x,
                this->collision_size_y
            },
            {this->collision_offset_x, this->collision_offset_y}
        };
    }
    
    void handle_collision(CollisionType const& type, CollisionSide const& side) override {}
    void on_after_collision() override {}
    
    void update(double elapsedTime) override
    {
        this->think(elapsedTime);

        // Position setup
        this->old_pos_x = this->pos_x;
        this->old_pos_y = this->pos_y;
        this->pos_x += this->velocity_x * elapsedTime;
        this->pos_y += this->velocity_y * elapsedTime;        
    }

    void run_animation(double elapsedTime) override
    {
        auto current_animation = ([this]() {
            if (this->start_attack) {
                return ACTIVATE_CANNON;
            }
            if (this->preparing_next_match) {
                return PREPARE_NEXT_MATCH;
            }
            return IDLE_ANIMATION;
        })();
//         this->animations.at(current_animation).run(this->renderer, elapsedTime, -this->face, this->pos_x, this->pos_y);
    }

private:
    void think(double elapsedTime)
    {
        if (!this->start_attack && !this->preparing_next_match) {
            this->think_timeout -= elapsedTime;
            if (this->think_timeout <= 0.) {
                this->start_attack = true;
                this->preparing_next_match = false;
                this->think_timeout = PigWithMatches::DEFAULT_THINK_TIMEOUT;
            }
        }
    }
    
public:
    std::map<int, Animation> animations;
    int face;
    double pos_x;
    double pos_y;
    double velocity_x;
    double velocity_y;
    SDL_Renderer* renderer;
    SDL_Texture* spritesheet;
    double old_pos_x;
    double old_pos_y;
    double think_timeout;
    bool start_attack;
    bool preparing_next_match;
    Cannon& cannon;
};
*/

class King : public IGameCharacter {
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

    static auto constexpr collision_size = Vector2D<int>{20, 27};

    static auto constexpr attack_region_offset_x = 30.;
    static auto constexpr attack_region_offset_y = -10.;
    static auto constexpr attack_region_w = 10.;
    static auto constexpr attack_region_h = 20.;

    static auto constexpr reference_point = Vector2D<int>{41, 30};
    static auto constexpr spritesheet_offset = Vector2D<int>{38, 32};

public:
    King(SDL_Renderer* renderer, double pos_x, double pos_y)
        : running_side(0)
        , animations()
        , after_taking_damage_timeout()
        , face(+1)
        , life(2)
        , old_position{pos_x, pos_y}
        , position{pos_x, pos_y}
        , velocity{0.0, 0.0}
        , renderer(renderer)
        , spritesheet(load_media("assets/sprites/king96x96.png", renderer))
        , is_jumping(false)
        , is_falling(true)
        , start_jumping(false)
        , is_grounded(false)
        , is_attacking(false)
        , just_touched_ground(false)
        , is_taking_damage(false)
        , is_dying(false)
        , is_dead(false)
    {
        auto register_animation = [&](int id, std::vector<std::tuple<int, int>> const& frames, double time) {
            this->animations.insert(std::make_pair(id, Animation(this->spritesheet, frames, 96, 96, time)));
        };
        register_animation(
            King::IDLE_ANIMATION,
            {
                {1, 3},
                {2, 3},
                {3, 3},
                {4, 3},
                {5, 3},
                {6, 3},
            },
            100.
        );
        register_animation(
            King::RUNNING_ANIMATION,
            {
                {4, 4},
                {5, 4},
                {6, 4},
                {0, 5},
                {1, 5},
                {2, 5},
                {3, 5},
            },
            100.
        );
        register_animation(
            King::JUMPING_ANIMATION,
            {
                {0, 4},
            },
            100.
        );
        register_animation(
            King::FALLING_ANIMATION,
            {
                {1, 4},
            },
            100.
        );
        register_animation(
            King::ATTACKING_ANIMATION,
            {
                {3, 5},
                {4, 5},
                {5, 5},
            },
            100.
        );
        register_animation(
            King::JUST_TOUCHED_GROUND_ANIMATION,
            {
                {2, 4},
            },
            150.
        );
        register_animation(
            King::TAKING_DAMAGE_ANIMATION,
            {
                {1, 0},
                {2, 0},
                {4, 1},
                {2, 0},
                {4, 1},
            },
            100.
        );
        register_animation(
            King::DYING_ANIMATION,
            {
                {1, 0},
                {2, 0},
                {3, 0},
                {4, 0},
                {5, 0},
            },
            100.
        );
        register_animation(
            King::DEAD_ANIMATION,
            {
                {5, 0},
            },
            100.
        );

        this->animations.at(ATTACKING_ANIMATION).set_on_finish_animation_callback([this](){
            this->is_attacking = false;
        });
        this->animations.at(JUST_TOUCHED_GROUND_ANIMATION).set_on_finish_animation_callback([this](){
            this->just_touched_ground = false;
        });
        this->animations.at(TAKING_DAMAGE_ANIMATION).set_on_finish_animation_callback([this](){
            this->is_taking_damage = false;
            if (this->life > 0) {
                this->after_taking_damage = true;
                this->after_taking_damage_timeout.restart();
            } else {
                this->is_dying = true;
            }
        });
        this->after_taking_damage_timeout = StateTimeout(500., [this]() { 
            this->after_taking_damage = false;
        });
        this->animations.at(DYING_ANIMATION).set_on_finish_animation_callback([this](){
            this->is_taking_damage = false;
            this->after_taking_damage = false;
            this->is_dying = false;
            this->is_dead = true;
            this->is_attacking = false;
            if (this->on_dead_callback) {
                (*this->on_dead_callback)();
            }
        });
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
        this->velocity.x = x;
        this->velocity.y = y;
    }

    CollisionRegionInformation get_collision_region_information() const override
    {
        return CollisionRegionInformation(this->position, this->old_position, this->collision_size);
    }

    void handle_collision(CollisionType const& type, CollisionSide const& side) override {
        if (type == CollisionType::TILEMAP_COLLISION) {
            if (side == CollisionSide::TOP_COLLISION) {
                this->set_velocity(0.0, +0.01); // Force response
            } else if (side == CollisionSide::BOTTOM_COLLISION) {
                this->is_grounded = true;
            }
        }
    }
    
    void on_after_collision() override
    {
        this->is_falling = (!this->is_grounded && this->velocity.y < 0.0);
        this->is_jumping = (!this->is_grounded && this->velocity.y > 0.0);
        if (this->is_grounded && (this->position.y + 0.1) < this->old_position.y) {
            this->just_touched_ground = true;
        }
    }

    void handle_controller(const unsigned char* keystates)
    {
        // Player handling
        if (!this->is_taking_damage && !this->is_dying && !this->is_dead) {
            if (keystates[SDL_SCANCODE_LEFT]) {
                this->running_side = -1;
                this->face = -1;
            } else if (keystates[SDL_SCANCODE_RIGHT]) {
                this->running_side = +1;
                this->face = +1;
            } else {
                this->running_side = 0;
            }
            if (keystates[SDL_SCANCODE_UP]) {
                if (!this->is_jumping && !this->is_falling) {
                    this->start_jumping = true;
                }
            }
            if (keystates[SDL_SCANCODE_LCTRL]) {
                if (!this->is_attacking) {
                    this->is_attacking = true;
                }
            }
        }
    }
    
    void register_on_dead_callback(std::function<void()> const& f)
    {
        this->on_dead_callback = f;
    }
    
    void update(double elapsedTime) override
    {
        // velocity x setup
        if (!this->is_taking_damage && !this->is_dying && !this->is_dead) {
            if (this->running_side == +1) this->velocity.x = +0.2;
            else if (this->running_side == -1) this->velocity.x = -0.2;
            else this->velocity.x = 0.0;
            
            // velocity y setup
            if (this->start_jumping) {
                this->start_jumping = false;
                this->is_grounded = false;
                this->velocity.y += 0.4;
            }
        }
        if (this->is_dead) {
            this->velocity.x = 0.0;
        }
        this->velocity.y += gravity * elapsedTime;

        // Position setup
        this->old_position = this->position;
        this->position += this->velocity * elapsedTime;

        this->after_taking_damage_timeout.update(elapsedTime);
    }

    void start_taking_damage()
    {
        this->velocity.x = 0.05;
        this->velocity.y = 0.1;
        this->is_taking_damage = true;
        this->life -= 1;
        if (this->on_start_taking_damage) {
            (*this->on_start_taking_damage)();
        }
    }

    void run_animation(double elapsedTime) override
    {
        auto current_animation = ([this]() {
            if (this->is_dead) {
                return DEAD_ANIMATION;
            }
            if (this->is_dying) {
                return DYING_ANIMATION;
            }
            if (this->is_taking_damage) {
                return TAKING_DAMAGE_ANIMATION;
            }
            if (this->just_touched_ground) {
                return JUST_TOUCHED_GROUND_ANIMATION;
            }
            if (this->is_attacking) {
                return ATTACKING_ANIMATION;
            }
            if (this->is_falling) {
                return FALLING_ANIMATION;
            }
            if (this->is_jumping) {
                return JUMPING_ANIMATION;
            }
            if (this->running_side != 0) {
                return RUNNING_ANIMATION;
            }
            return IDLE_ANIMATION;
        })();
        this->animations.at(current_animation).run(
            this->renderer,
            elapsedTime,
            this->face,
            Vector2D<int>{int(this->position.x), int(this->position.y)},
            this->spritesheet_offset,
            camera_offset
        );
    }

    Region2D<double> attack_region() const {
        auto const& collision_region = this->get_collision_region_information().collision_region;
        return {
            collision_region.x + this->face * attack_region_offset_x,
            collision_region.y + attack_region_offset_y,
            collision_region.w + attack_region_w,
            collision_region.h + attack_region_h
        };
    }

    int running_side;
    std::map<int, Animation> animations;
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
    bool is_attacking;
    bool just_touched_ground;
    bool is_taking_damage;
    bool after_taking_damage;
    bool is_dying;
    bool is_dead;
    std::optional<std::function<void()>> on_dead_callback;
    std::optional<std::function<void()>> on_start_taking_damage;
};

void debug_text(std::vector<std::string> const& messages, SDL_Renderer* renderer, TTF_Font* font, int posx, int posy)
{
    for (auto const& message : messages) {
        SDL_Surface* message_surface = TTF_RenderText_Shaded(font, message.c_str(), {255, 255, 255}, {0, 0, 0});
        SDL_Texture* message_texture = SDL_CreateTextureFromSurface(renderer, message_surface);

        SDL_Rect rect{posx, posy, 0, 0};
        rect.w = message_surface->w;
        rect.h = message_surface->h;
        SDL_RenderCopy(renderer, message_texture, NULL, &rect);

        SDL_FreeSurface(message_surface);
        SDL_DestroyTexture(message_texture);

        posy += 10;
    }
}

void pig_king_collision(Pig* pig_ptr, King* king)
{
    auto& player = *king;
    auto& pig = *pig_ptr;

    auto const& pig_collision_region = pig.get_collision_region_information().collision_region;
    auto const& player_collision_region = player.get_collision_region_information().collision_region;

    if (
        player.is_attacking &&
        !player.is_taking_damage &&
        !pig.is_taking_damage &&
        !pig.is_dying &&
        !pig.is_dead
    ) {
        auto player_attack_region = player.attack_region();

        if (check_aabb_collision(player_attack_region, pig_collision_region)) {
            pig.start_taking_damage();
        }
    }

    if (
        !player.is_taking_damage &&
        !player.after_taking_damage &&
        !player.is_dying &&
        !player.is_dead &&
        !pig.is_taking_damage &&
        !pig.is_dying &&
        !pig.is_dead
    ) {
        if (check_aabb_collision(player_collision_region, pig_collision_region)) {
            player.start_taking_damage();
        }
    }
}

/*
void cannon_king_collision(Cannon* cannon_ptr, King* king)
{
    auto& player = *king;
    auto& cannon = *cannon_ptr;

    if (
        player.is_attacking &&
        !player.is_taking_damage
    ) {
        auto const& cannon_collision_region = cannon.get_collision_region_information().collision_region;
        auto player_attack_region = player.attack_region();
        
        if (check_aabb_collision(player_attack_region, cannon_collision_region)) {
            cannon.trigger_attack();
        }
    }
}

void cannonball_king_collision(CannonBall* cannon_ptr, King* king)
{
    auto& player = *king;
    auto& cannonball = *cannon_ptr;
    if (
        !player.is_taking_damage &&
        !player.after_taking_damage &&
        !player.is_dying &&
        !player.is_dead
    ) {
        auto cannonball_collision_region_info = cannonball.get_collision_region_information();
        auto cannonball_collision_region = cannonball_collision_region_info.collision_region;

        auto player_collision_region_info = player.get_collision_region_information();
        auto player_collision_region = player_collision_region_info.collision_region;

        if (check_aabb_collision(cannonball_collision_region, player_collision_region)) {
            player.start_taking_damage();
        }
    }
}

void cannonball_pig_collision(CannonBall* cannon_ptr, Pig* pig_ptr)
{
    auto& pig = *pig_ptr;
    auto& cannonball = *cannon_ptr;

    if (
        !pig.is_taking_damage &&
        !pig.is_dying &&
        !pig.is_dead
    ) {
        auto const& cannonball_collision_region = cannonball.get_collision_region_information().collision_region;
        auto const& pig_collision_region = pig.get_collision_region_information().collision_region;

        if (check_aabb_collision(cannonball_collision_region, pig_collision_region)) {
            pig.start_taking_damage();
        }
    }
}
*/

void compute_characters_collisions(std::vector<IGameCharacter*>& game_characters)
{
    for (int i = 0; i < game_characters.size(); ++i) {
        for (int j = i + 1; j < game_characters.size(); ++j) {
            if (false) {}

            else if (dynamic_cast<Pig*>(game_characters[i]) && dynamic_cast<King*>(game_characters[j])) { pig_king_collision(dynamic_cast<Pig*>(game_characters[i]), dynamic_cast<King*>(game_characters[j])); }
            else if (dynamic_cast<Pig*>(game_characters[j]) && dynamic_cast<King*>(game_characters[i])) { pig_king_collision(dynamic_cast<Pig*>(game_characters[j]), dynamic_cast<King*>(game_characters[i])); }

            // else if (dynamic_cast<Cannon*>(game_characters[i]) && dynamic_cast<King*>(game_characters[j])) { cannon_king_collision(dynamic_cast<Cannon*>(game_characters[i]), dynamic_cast<King*>(game_characters[j])); }
            // else if (dynamic_cast<Cannon*>(game_characters[j]) && dynamic_cast<King*>(game_characters[i])) { cannon_king_collision(dynamic_cast<Cannon*>(game_characters[j]), dynamic_cast<King*>(game_characters[i])); }

            // else if (dynamic_cast<CannonBall*>(game_characters[i]) && dynamic_cast<King*>(game_characters[j])) { cannonball_king_collision(dynamic_cast<CannonBall*>(game_characters[i]), dynamic_cast<King*>(game_characters[j])); }
            // else if (dynamic_cast<CannonBall*>(game_characters[j]) && dynamic_cast<King*>(game_characters[i])) { cannonball_king_collision(dynamic_cast<CannonBall*>(game_characters[j]), dynamic_cast<King*>(game_characters[i])); }

            // else if (dynamic_cast<CannonBall*>(game_characters[i]) && dynamic_cast<Pig*>(game_characters[j])) { cannonball_pig_collision(dynamic_cast<CannonBall*>(game_characters[i]), dynamic_cast<Pig*>(game_characters[j])); }
            // else if (dynamic_cast<CannonBall*>(game_characters[j]) && dynamic_cast<Pig*>(game_characters[i])) { cannonball_pig_collision(dynamic_cast<CannonBall*>(game_characters[j]), dynamic_cast<Pig*>(game_characters[i])); }
        }
    }
}

int main(int argc, char* args[])
{
    SDL_Window* window = nullptr;
    bool quit = false;
    SDL_Event e;
    
    auto tilemap = std::array<std::array<int, WIDTH>, HEIGHT>{{
        {13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13},
        {13, 19, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 18, 25, 25, 25, 25, 25,  5, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 15, 13, 13, 13, 13, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 15, 13, 13, 13, 13, 13, 12, 13},
        {13, 14, 39, 39, 39, 61, 61, 61, 61, 61, 39, 39, 39, 39, 39, 39, 39, 15, 39, 39, 39, 13, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 15, 13, 13, 13, 13, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 27, 13, 13, 13, 39, 39, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 39, 61, 61, 61, 61, 61, 61, 61, 13, 13, 13, 13, 13, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 61, 39, 61, 61, 61, 61, 61, 61, 13, 13, 13, 13, 13, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 61, 61, 39, 61, 61, 61, 61, 61,  3, 39, 39, 39, 13, 39, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 39, 61, 61, 61, 61, 15, 13, 13, 13, 13, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 39, 15, 13, 13, 13, 13, 13, 12, 13},
        {13, 19, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 61, 61, 61, 39, 15, 13, 13, 13, 39, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 39, 39, 15, 13, 13, 13, 13, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 39, 39, 39, 15, 13, 39, 13, 13, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 39, 39, 39, 39, 15, 13, 13, 13, 13, 13, 12, 13},
        {13, 16,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  7,  1,  1,  1,  1,  1, 17, 13},
        {13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13},
    }};
    auto foreground = std::array<std::array<int, WIDTH>, HEIGHT>{{
        { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0, 16, 17, 18, 19,  0, 16, 17, 18, 19,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0, 23, 24, 25, 26,  0, 23, 24, 25, 26,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    }};

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL could not initialize!");
    }
    if (TTF_Init() != 0) {
        throw std::runtime_error("FAILED to initialize TTF library");
    }
    auto* default_font = TTF_OpenFont("./FreeMono.ttf", 12);
    if (default_font == nullptr) {
        throw std::runtime_error("Unable to create default TTF font.");
    }

    window = SDL_CreateWindow(
        "Pigs game",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (window == NULL) {
        throw std::runtime_error("SDL Error: Window could not be created");
    }

    auto* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    bool show_debug = false;
    
    auto load_spritesheet = [&renderer](std::string const& filename) {
        return load_media("assets/sprites/" + filename, renderer);
    };
    auto tileset = load_spritesheet("tileset.png");
    auto foreground_set = load_spritesheet("foreground_set.png");
    auto lifebar = load_spritesheet("lifebar.png");
    auto lifebar_heart = load_spritesheet("small_heart18x14.png");
    auto door = load_spritesheet("door46x56.png");

    auto window_is_shaking = false;
    auto window_shaker = StateTimeout(300., [&window_is_shaking](){ window_is_shaking = false; });

    auto game_characters = std::vector<IGameCharacter*>();
    auto player = King(renderer, 100.0, 100.0);
    game_characters.push_back(&player);
    int n_pigs = 10;
    for (int i = 0; i < n_pigs; ++i) {
        auto pos_x = 300 + 5 * i;
        auto pos_y = 422;
        auto *pig = new Pig(renderer, pos_x, pos_y);
        game_characters.push_back(pig);

        // TODO: Move this to somewhere else
        pig->on_start_taking_damage = [&window_is_shaking, &window_shaker]() {
            window_is_shaking = true;
            window_shaker.restart();
        };
    }

    auto cannon = Cannon(renderer, 80.0, 64.0, -1);
    game_characters.push_back(&cannon);
    cannon.set_on_before_fire([&game_characters, &renderer, &cannon]() {
        auto cannon_position = cannon.get_position();
        auto ball = new CannonBall(renderer, cannon_position.x + CannonBall::ball_exit_offset_x, cannon_position.y + CannonBall::ball_exit_offset_y);
        ball->set_velocity(+0.4, 0.0);
        game_characters.push_back(ball);
    });
//     auto pig_with_match = PigWithMatches(renderer, 56., 64., +1, cannon);
//     game_characters.push_back(&pig_with_match);

    auto last = (unsigned long long)(0);
    auto current = SDL_GetPerformanceCounter();
    auto fps_countdown = 1000.;
    auto fps_counter = 0;
    auto fps = 0;
    
    auto transition_animation = TransitionAnimation();
    player.register_on_dead_callback([&transition_animation]() {
        transition_animation.reset();
    });
    player.on_start_taking_damage = [&window_is_shaking, &window_shaker]() {
        window_is_shaking = true;
        window_shaker.restart();
    };
    
    transition_animation.register_transition_callback([&player]() {
        player.set_position(100.0, 100.0);
        player.is_dead = false;
        player.life = 2;
    });
    
    while (!quit) {
        last = current;
        current = SDL_GetPerformanceCounter();
        double elapsedTime = (double)((current - last)*1000.0 / (double)SDL_GetPerformanceFrequency() );
        fps_countdown -= elapsedTime;
        fps_counter += 1;
        if (fps_countdown < 0.) {
            fps = fps_counter;
            fps_counter = 0;
            fps_countdown = 1000.;
        }

        debug_messages.clear();
        debug_messages.push_back("FPS: " + std::to_string(fps));

        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT: {
                quit = true;
                break;
                }
                case SDL_KEYDOWN: {
                    if (e.key.keysym.sym == SDLK_TAB && !e.key.repeat) {
                        show_debug = !show_debug;
                        break;
                    }
                }
            }
        }
                        
        auto keystates = SDL_GetKeyboardState(NULL);
        player.handle_controller(keystates);
        for (auto* c : game_characters) {
            c->update(elapsedTime);
        }

        for (auto* c : game_characters) {
            compute_tilemap_collisions(tilemap, c);
        }
        compute_characters_collisions(game_characters);

        game_characters.erase(
            std::remove_if(game_characters.begin(), game_characters.end(), [](IGameCharacter* c) {
                auto* pig = dynamic_cast<Pig*>(c);
                if (pig != nullptr) {
                return pig->is_dead;
                }
                return false;
            }),
            game_characters.end()
        );

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw background
        int shake_x = window_is_shaking ? random_int(-3, 3) : 0;
        int shake_y = window_is_shaking ? random_int(-1, 1) : 0;
        window_shaker.update(elapsedTime);
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                // Background
                {
                    auto tile_id = tilemap[i][j];
                    auto offset = Vector2D<int>{TILE_SIZE * (tile_id % 12), TILE_SIZE * int(floor(tile_id / 12))};
                    auto world_position = Vector2D<int>{TILE_SIZE * j + shake_x, TILE_SIZE * (HEIGHT - i - 1) + shake_y};
                    auto size = Vector2D<int>{TILE_SIZE, TILE_SIZE};
                    draw_sprite(renderer, tileset, offset, world_position, size, camera_offset);

                    if (show_debug) {
                        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 40);
                        for (auto&& collision_tile_id : collision_tiles) {
                            if (tile_id == collision_tile_id) {
                                auto camera_position = to_camera_position(world_position, size, camera_offset);
                                auto dstrect = SDL_Rect{camera_position.x, camera_position.y, SCALE_SIZE * size.x, SCALE_SIZE * size.y};
                                SDL_RenderFillRect(renderer, &dstrect);
                            }
                        }
                    }
                }

                // Foreground
                {
                    auto tile_id = foreground[i][j];
                    auto offset = Vector2D<int>{TILE_SIZE * (tile_id % 7), TILE_SIZE * int(floor(tile_id / 7))};
                    auto world_position = Vector2D<int>{TILE_SIZE * j + shake_x, TILE_SIZE * (HEIGHT - i - 1) + shake_y};
                    auto size = Vector2D<int>{TILE_SIZE, TILE_SIZE};
                    draw_sprite(renderer, foreground_set, offset, world_position, size, camera_offset);
                }

                // Interactibles
                {
                    auto offset = Vector2D<int>{0, 0};
                    auto world_position = Vector2D<int>{128 + shake_x, 192 + shake_y};
                    auto size = Vector2D<int>{46, 56};
                    draw_sprite(renderer, door, offset, world_position, size, camera_offset);
                }
                
                // HUD
                {
                    {
                        auto offset = Vector2D<int>{0, 0};
                        auto size = Vector2D<int>{66, 34};
                        auto static_camera_position = Vector2D<int>{10, SCREEN_HEIGHT / SCALE_SIZE - size.y - 10};
                        draw_static_sprite(renderer, lifebar, offset, static_camera_position, size);
                    }

                    auto offset = Vector2D<int>{0, 0};
                    auto size = Vector2D<int>{18, 14};
                    for (int i = 0; i < player.life; ++i) {
                        auto camera_position = Vector2D<int>{21 + 11 * i, SCREEN_HEIGHT / SCALE_SIZE - size.y - 20};
                        draw_static_sprite(renderer, lifebar_heart, offset, camera_position, size);
                    }
                }
            }
        }

        for (auto& game_character : game_characters) {
            game_character->run_animation(elapsedTime);
        }
        transition_animation.run(renderer, elapsedTime);
        
        int mousex = 0;
        int mousey = 0;
        SDL_GetMouseState(&mousex, &mousey);
        debug_messages.push_back("Mouse (Camera ): " + std::to_string(mousex) + ", " + std::to_string(mousey));
        Vector2D world_mouse = to_world_position(Vector2D<int>{mousex, mousey}, Vector2D<int>{0, 0}, camera_offset);
        debug_messages.push_back("Mouse (World): " + std::to_string(int(world_mouse.x)) + ", " + std::to_string(int(world_mouse.y)));
        if (show_debug) {
            auto r = (Uint8)(0);
            auto g = (Uint8)(0);
            auto b = (Uint8)(0);
            auto a = (Uint8)(0);
            SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 90);

            for (auto& game_character : game_characters) {
                auto const& collision_region = game_character->get_collision_region_information().collision_region;
                auto camera_position = to_camera_position(
                    Vector2D<int>{int(collision_region.x), int(collision_region.y)},
                    Vector2D<int>{int(collision_region.w), int(collision_region.h)},
                    camera_offset
                );
                auto collision_rect = to_sdl_rect(Region2D<int>{
                    camera_position.x,
                    camera_position.y,
                    int(SCALE_SIZE * collision_region.w),
                    int(SCALE_SIZE * collision_region.h)
                });
                SDL_RenderFillRect(renderer, &collision_rect);
            }

//             for (auto* c : game_characters) {
//                 auto const& collision_region = c->get_collision_region_information().collision_region;
//                 auto const& collision_size = c->get_collision_region_information().collision_region_offset;
//                 auto camera_position = to_camera_position(
//                     Vector2D<int>{int(collision_region.x), int(collision_region.y)},
//                     Vector2D<int>{0, 0}
//                 );
//                 auto collision_rect = to_sdl_rect(Region2D<int>{camera_position.x, camera_position.y, collision_region.w, collision_region.h});
//                 SDL_RenderFillRect(renderer, &collision_rect);
//             }
//             auto player_attack_region = player.attack_region();
//             auto player_attack_position = to_camera_position(
//                 Vector2D<int>{int(player_attack_region.x), int(player_attack_region.y)},
//                 Vector2D<int>{0, 0}
//             );
//             auto player_attack_rect = to_sdl_rect(Region2D<int>{player_attack_position.x, player_attack_position.y, player_attack_region.w, player_attack_region.h});
//             SDL_RenderFillRect(renderer, &player_attack_rect);
// 
            debug_text(debug_messages, renderer, default_font, 10, 10);
            SDL_SetRenderDrawColor(renderer, r, g, b, a);
        }

        {
            if (keystates[SDL_SCANCODE_A]) {
                camera_offset.x -= 1;
            } else if (keystates[SDL_SCANCODE_D]) {
                camera_offset.x += 1;
            } else if (keystates[SDL_SCANCODE_W]) {
                camera_offset.y += 1;
            } else if (keystates[SDL_SCANCODE_S]) {
                camera_offset.y -= 1;
            }
        }

        // Update camera
        auto position = player.get_position().as_int();
        auto camera_min_x = 0;
        auto camera_max_x = 200;
        auto camera_min_y = 0;
        auto camera_max_y = 220;
        camera_offset.x = std::max(camera_min_x, std::min(position.x - SCREEN_WIDTH / (2 * SCALE_SIZE), camera_max_x));
        camera_offset.y = std::max(camera_min_y, std::min(position.y - SCREEN_HEIGHT / (2 * SCALE_SIZE), camera_max_y));

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

