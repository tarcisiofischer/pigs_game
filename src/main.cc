#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <iostream>
#include <functional>
#include <optional>
#include <vector>
#include <tuple>
#include <memory>
#include <random>
#include <map>
#include <variant>

auto constexpr WIDTH = 25;
auto constexpr HEIGHT = 18;
auto constexpr tile_size = 32;
auto constexpr SCREEN_WIDTH = tile_size * WIDTH;
auto constexpr SCREEN_HEIGHT = tile_size * HEIGHT;

auto constexpr gravity = 0.001;

auto constexpr collision_tiles = std::array<int, 10>{1, 3, 12, 14, 15, 25, 27, 37, 38, 39};

std::vector<std::string> debug_messages;

using Tilemap = std::array<std::array<int, WIDTH>, HEIGHT>;

enum class CollisionType {
    TILEMAP_COLLISION = 0
};

enum class CollisionSide {
    LEFT_COLLISION = 0,
    RIGHT_COLLISION = 1,
    TOP_COLLISION = 2,
    BOTTOM_COLLISION = 3
};

struct Vector2D {
    double x;
    double y;
};

struct Region2D {
    double x;
    double y;
    double w;
    double h;
};

struct CollisionRegionInformation
{
    Region2D collision_region;
    Region2D old_collision_region;
    Vector2D collision_region_offset;
};

class IGameCharacter
{
public:
    virtual void update(double elapsedTime) = 0;
    virtual void run_animation(double elapsedTime) = 0;
    virtual void set_position(double x, double y) = 0;
    virtual Vector2D get_position() const = 0;
    virtual Vector2D get_velocity() const = 0;
    virtual void set_velocity(double x, double y) = 0;
    virtual void handle_collision(CollisionType const& type, CollisionSide const& side) = 0;
    virtual CollisionRegionInformation get_collision_region_information() const = 0;
    virtual void on_after_collision() = 0;
};

SDL_Rect to_sdl_rect(Region2D const& region)
{
    return SDL_Rect{
        int(region.x), int(region.y),
        int(region.w), int(region.h)};
}

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

inline bool check_aabb_collision(Region2D const& a, Region2D const& b)
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
                    auto const& old_collision_region = collision_region_info.old_collision_region;
                    auto const& collision_region_offset = collision_region_info.collision_region_offset;
                    auto current_position = c->get_position();
                    auto current_velocity = c->get_velocity();

                    auto tile_region = Region2D{double(tile_size * j), double(tile_size * i), double(tile_size), double(tile_size)};

                    if (check_aabb_collision(collision_region, tile_region)) {
                        if (
                            collision_region.x + collision_region.w > tile_region.x &&
                            old_collision_region.x + old_collision_region.w <= tile_region.x
                        ) {
                            c->set_position(tile_region.x - collision_region_offset.x - collision_region.w - 0.1, current_position.y);
                            c->set_velocity(0.0, current_velocity.y);

                            c->handle_collision(CollisionType::TILEMAP_COLLISION, CollisionSide::RIGHT_COLLISION);
                        } else if (
                            collision_region.x < tile_region.x + tile_region.w &&
                            old_collision_region.x >= tile_region.x + tile_region.w
                        ) {
                            c->set_position(tile_region.x + tile_region.w - collision_region_offset.x + 0.1, current_position.y);
                            c->set_velocity(0.0, current_velocity.y);

                            c->handle_collision(CollisionType::TILEMAP_COLLISION, CollisionSide::LEFT_COLLISION);
                        } else if (
                            collision_region.y < tile_region.y + tile_region.h &&
                            old_collision_region.y >= tile_region.y + tile_region.h
                        ) {
                            c->set_position(current_position.x, tile_region.y + tile_region.h - collision_region_offset.y + 0.1);
                            c->set_velocity(current_velocity.x, 0.0);

                            c->handle_collision(CollisionType::TILEMAP_COLLISION, CollisionSide::TOP_COLLISION);
                        } else if (
                            collision_region.y + collision_region.h > tile_region.y &&
                            old_collision_region.y + old_collision_region.h <= tile_region.y
                        ) {
                            c->set_position(current_position.x, tile_region.y - collision_region.h - collision_region_offset.y - 0.1);
                            c->set_velocity(current_velocity.x, 0.0);
                            
                            c->handle_collision(CollisionType::TILEMAP_COLLISION, CollisionSide::BOTTOM_COLLISION);
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

class Animation
{
public:
    Animation(
        SDL_Texture* spritesheet,
        std::vector<std::tuple<int, int>> const& frames,
        int framesize_x,
        int framesize_y,
        double animation_time
    )
        : spritesheet(spritesheet)
        , frames(frames)
        , state(0)
        , framesize_x(framesize_x)
        , framesize_y(framesize_y)
        , animation_time(animation_time)
    {
    }
    
    void set_on_finish_animation_callback(std::function<void()> const& f)
    {
        this->on_finish_animation = f;
    }

    void run(SDL_Renderer* renderer, double elapsedTime, int face, int pos_x, int pos_y)
    {
        this->counter += elapsedTime;
        if (this->counter >= this->animation_time) {
            this->state += 1;
            if ((this->state % this->frames.size()) == 0) {
                this->state = 0;
                if (this->on_finish_animation) {
                    (*this->on_finish_animation)();
                }
            }
            this->counter = 0.0;
        }
        auto frame_x = int(0);
        auto frame_y = int(0);
        std::tie(frame_x, frame_y) = frames[state];

        auto sprite_rect = SDL_Rect{frame_x * this->framesize_x, frame_y * this->framesize_y, this->framesize_x, this->framesize_y};
        auto world_rect = SDL_Rect{pos_x, pos_y, this->framesize_x, this->framesize_y};

        auto flip = (face == +1) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
        SDL_RenderCopyEx(renderer, this->spritesheet, &sprite_rect, &world_rect, 0.0, nullptr, flip);
    }
    
private:
    SDL_Texture* spritesheet;
    std::vector<std::tuple<int, int>> frames;
    double counter;
    int state;
    std::optional<std::function<void()>> on_finish_animation;
    int framesize_x;
    int framesize_y;
    double animation_time;
};

class Pig : public IGameCharacter {
public:
    static auto constexpr IDLE_ANIMATION = 0;
    static auto constexpr RUNNING_ANIMATION = 1;
    static auto constexpr TAKING_DAMAGE_ANIMATION = 2;
    static auto constexpr DYING_ANIMATION = 3;

    static auto constexpr collision_offset_x = 30.0;
    static auto constexpr collision_offset_y = 30.0;
    static auto constexpr collision_size_x = 18.0;
    static auto constexpr collision_size_y = 18.0;

    Pig(Pig const& other)
    {
        this->running_side = other.running_side;

        // Force callbacks re-connection, so that the lambda functions are
        // up-to-date in relation to the lambda captures
        this->animations = other.animations;
        this->connect_callbacks();

        this->face = other.face;
        this->pos_x = other.pos_x;
        this->pos_y = other.pos_y;
        this->velocity_x = other.velocity_x;
        this->velocity_y = other.velocity_y;
        this->renderer = other.renderer;
        this->spritesheet = other.spritesheet;
        this->think_timeout = other.think_timeout;
        this->old_pos_x = other.old_pos_x;
        this->old_pos_y = other.old_pos_y;
        this->is_taking_damage = other.is_taking_damage;
        this->life = other.life;
        this->is_dying = other.is_dying;
        this->is_dead = other.is_dead;
    }
    
    virtual ~Pig() {} 

    Pig& operator=(Pig const& other)
    {
        this->running_side = other.running_side;

        // Force callbacks re-connection, so that the lambda functions are
        // up-to-date in relation to the lambda captures
        this->animations = other.animations;
        this->connect_callbacks();

        this->face = other.face;
        this->pos_x = other.pos_x;
        this->pos_y = other.pos_y;
        this->velocity_x = other.velocity_x;
        this->velocity_y = other.velocity_y;
        this->renderer = other.renderer;
        this->spritesheet = other.spritesheet;
        this->think_timeout = other.think_timeout;
        this->old_pos_x = other.old_pos_x;
        this->old_pos_y = other.old_pos_y;
        this->is_taking_damage = other.is_taking_damage;
        this->life = other.life;
        this->is_dying = other.is_dying;
        this->is_dead = other.is_dead;

        return *this;
    }

    Pig(SDL_Renderer* renderer, double pos_x, double pos_y)
        : running_side(0)
        , pos_x(pos_x)
        , pos_y(pos_y)
        , velocity_x(0.0)
        , velocity_y(0.0)
        , renderer(renderer)
        , spritesheet(load_media("assets/sprites/pig80x80.png", renderer))
        , think_timeout(1000.)
        , old_pos_x(pos_x)
        , old_pos_y(pos_y)
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
        this->pos_x = x;
        this->pos_y = y;
    }

    Vector2D get_position() const override
    {
        return Vector2D{this->pos_x, this->pos_y};
    }
    
    Vector2D get_velocity() const override
    {
        return Vector2D{this->velocity_x, this->velocity_y};
    }
    
    void set_velocity(double x, double y) override
    {
        this->velocity_x = x;
        this->velocity_y = y;
    }

    CollisionRegionInformation get_collision_region_information() const override
    {
        return CollisionRegionInformation{
            Region2D{
                this->pos_x + Pig::collision_offset_x,
                this->pos_y + Pig::collision_offset_y,
                Pig::collision_size_x,
                Pig::collision_size_y
            },
            Region2D{
                this->old_pos_x + Pig::collision_offset_x,
                this->old_pos_y + Pig::collision_offset_y,
                Pig::collision_size_x,
                Pig::collision_size_y
            },
            Vector2D{Pig::collision_offset_x, Pig::collision_offset_y}
        };
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

        if (this->running_side == +1) this->velocity_x = +0.05;
        else if (this->running_side == -1) this->velocity_x = -0.05;
        else this->velocity_x = 0.0;
        this->velocity_y = velocity_y + gravity * elapsedTime;

        // Position setup
        this->old_pos_x = this->pos_x;
        this->old_pos_y = this->pos_y;
        this->pos_x += this->velocity_x * elapsedTime;
        this->pos_y += this->velocity_y * elapsedTime;        
    }
    
    void start_taking_damage()
    {
        this->velocity_x = 0.05;
        this->velocity_y = -0.1;
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
        this->animations.at(current_animation).run(this->renderer, elapsedTime, -this->face, this->pos_x, this->pos_y);
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
    double pos_x;
    double pos_y;
    double velocity_x;
    double velocity_y;
    SDL_Renderer* renderer;
    SDL_Texture* spritesheet;
    double think_timeout;
    double old_pos_x;
    double old_pos_y;
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
    static auto constexpr collision_size_x = 24.;
    static auto constexpr collision_size_y = 21.;
    
    Cannon(SDL_Renderer* renderer, double pos_x, double pos_y, int face)
        : face(face)
        , pos_x(pos_x)
        , pos_y(pos_y)
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

    Vector2D get_position() const override
    {
        return Vector2D{this->pos_x, this->pos_y};
    }

    Vector2D get_velocity() const override
    {
        return Vector2D{0.0, 0.0};
    }

    void set_velocity(double x, double y) override {}

    CollisionRegionInformation get_collision_region_information() const override
    {
        return CollisionRegionInformation{
            Region2D{
                this->pos_x + Cannon::collision_offset_x,
                this->pos_y + Cannon::collision_offset_y,
                Cannon::collision_size_x,
                Cannon::collision_size_y
            },
            // Cannons are not movable
            Region2D{
                this->pos_x + Cannon::collision_offset_x,
                this->pos_y + Cannon::collision_offset_y,
                Cannon::collision_size_x,
                Cannon::collision_size_y
            },
            Vector2D{Cannon::collision_offset_x, Cannon::collision_offset_y}
        };
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
        this->animations.at(current_animation).run(this->renderer, elapsedTime, this->face, this->pos_x, this->pos_y);
    }

public:
    std::map<int, Animation> animations;
    int face;
    double pos_x;
    double pos_y;
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
    static auto constexpr exploding_collision_size_x = 40.;
    static auto constexpr exploding_collision_size_y = 40.;
    
    static auto constexpr ball_exit_offset_x = 23.0;
    static auto constexpr ball_exit_offset_y = 33.0;    

    CannonBall(SDL_Renderer* renderer, double pos_x, double pos_y)
        : animations()
        , boom_animation(nullptr, {}, 0, 0, 100.)
        , old_pos_x(pos_x)
        , old_pos_y(pos_y)
        , pos_x(pos_x)
        , pos_y(pos_y)
        , velocity_x(0.0)
        , velocity_y(0.0)
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
        this->old_pos_x = this->pos_x;
        this->old_pos_y = this->pos_y;
        this->pos_x = this->pos_x + this->velocity_x * elapsedTime;
        this->pos_y = this->pos_y + this->velocity_y * elapsedTime;        
    }
    
    void run_animation(double elapsedTime) override {
        if (this->state == CannonBallState::active) {
            this->animations.at(IDLE_ANIMATION).run(this->renderer, elapsedTime, +1, int(this->pos_x), int(this->pos_y));
        } else if (this->state == CannonBallState::exploding) {
            this->boom_animation.run(this->renderer, elapsedTime, +1, int(this->pos_x - 8.0), int(this->pos_y - 28.0));
        }
    }

    void set_position(double x, double y) override {}

    Vector2D get_position() const override {
        return Vector2D{this->pos_x, this->pos_y};
    }

    Vector2D get_velocity() const override {
        return Vector2D{this->velocity_x, this->velocity_y};
    }

    void set_velocity(double x, double y) override {
        this->velocity_x = x;
        this->velocity_y = y;
    }
    
    void handle_collision(CollisionType const& type, CollisionSide const& side) override {
        if (type == CollisionType::TILEMAP_COLLISION) {
            this->state = CannonBallState::exploding;
        }
    }

    CollisionRegionInformation get_collision_region_information() const override {
        if (this->state == CannonBallState::active) {
            return CollisionRegionInformation{
                Region2D{
                    this->pos_x + CannonBall::collision_offset_x,
                    this->pos_y + CannonBall::collision_offset_y,
                    CannonBall::collision_size_x,
                    CannonBall::collision_size_y
                },
                Region2D{
                    this->old_pos_x + CannonBall::collision_offset_x,
                    this->old_pos_y + CannonBall::collision_offset_y,
                    CannonBall::collision_size_x,
                    CannonBall::collision_size_y
                },
                Vector2D{CannonBall::collision_offset_x, CannonBall::collision_offset_y}
            };
        } else if (this->state == CannonBallState::exploding) {
            return CollisionRegionInformation{
                Region2D{
                    this->pos_x + CannonBall::exploding_collision_offset_x,
                    this->pos_y + CannonBall::exploding_collision_offset_y,
                    CannonBall::exploding_collision_size_x,
                    CannonBall::exploding_collision_size_y
                },
                Region2D{
                    this->old_pos_x + CannonBall::exploding_collision_offset_x,
                    this->old_pos_y + CannonBall::exploding_collision_offset_y,
                    CannonBall::exploding_collision_size_x,
                    CannonBall::exploding_collision_size_y
                },
                Vector2D{CannonBall::exploding_collision_offset_x, CannonBall::exploding_collision_offset_y}
            };
        } else {
            return CollisionRegionInformation{
                Region2D{
                    0.0,
                    0.0,
                    0.0,
                    0.0,
                },
                Region2D{
                    0.0,
                    0.0,
                    0.0,
                    0.0,
                },
                Vector2D{0.0, 0.0}
            };
        }
    }

    void on_after_collision() override {}

public:
    std::map<int, Animation> animations;
    Animation boom_animation;
    double old_pos_x;
    double old_pos_y;
    double pos_x;
    double pos_y;
    double velocity_x;
    double velocity_y;
    CannonBallState state;
    SDL_Renderer* renderer;
    SDL_Texture* spritesheet;
    SDL_Texture* boom_spritesheet;
};

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

    static auto constexpr collision_offset_x = 38.;
    static auto constexpr collision_offset_y = 36.;
    static auto constexpr collision_size_x = 20.;
    static auto constexpr collision_size_y = 27.;

    static auto constexpr attack_region_offset_x = 30.;
    static auto constexpr attack_region_offset_y = -10.;
    static auto constexpr attack_region_w = 10.;
    static auto constexpr attack_region_h = 20.;

public:
    King(SDL_Renderer* renderer, double pos_x, double pos_y)
        : running_side(0)
        , animations()
        , after_taking_damage_timeout()
        , face(+1)
        , life(2)
        , old_pos_x(pos_x)
        , old_pos_y(pos_y)
        , pos_x(pos_x)
        , pos_y(pos_y)
        , velocity_x(0.0)
        , velocity_y(0.0)
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
        this->pos_x = x;
        this->pos_y = y;
    }

    Vector2D get_position() const override
    {
        return Vector2D{this->pos_x, this->pos_y};
    }

    Vector2D get_velocity() const override
    {
        return Vector2D{this->velocity_x, this->velocity_y};
    }

    void set_velocity(double x, double y) override
    {
        this->velocity_x = x;
        this->velocity_y = y;
    }

    CollisionRegionInformation get_collision_region_information() const override
    {
        return CollisionRegionInformation{
            Region2D{
                this->pos_x + King::collision_offset_x,
                this->pos_y + King::collision_offset_y,
                King::collision_size_x,
                King::collision_size_y
            },
            Region2D{
                this->old_pos_x + King::collision_offset_x,
                this->old_pos_y + King::collision_offset_y,
                King::collision_size_x,
                King::collision_size_y
            },
            Vector2D{King::collision_offset_x, King::collision_offset_y}
        };
    }

    void handle_collision(CollisionType const& type, CollisionSide const& side) override {
        if (type == CollisionType::TILEMAP_COLLISION) {
            if (side == CollisionSide::TOP_COLLISION) {
                this->set_velocity(0.0, -0.01); // Force response
            } else if (side == CollisionSide::BOTTOM_COLLISION) {
                this->is_grounded = true;
            }
        }
    }
    
    void on_after_collision() override
    {
        this->is_falling = (!this->is_grounded && this->velocity_y > 0.0);
        this->is_jumping = (!this->is_grounded && this->velocity_y < 0.0);
        if (this->is_grounded && abs(this->pos_y - this->old_pos_y) > 0.1) {
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
            if (this->running_side == +1) this->velocity_x = +0.2;
            else if (this->running_side == -1) this->velocity_x = -0.2;
            else this->velocity_x = 0.0;
            
            // velocity y setup
            if (this->start_jumping) {
                this->start_jumping = false;
                this->is_grounded = false;
                this->velocity_y -= 0.5;
            }
        }
        if (this->is_dead) {
            this->velocity_x = 0.0;
        }
        this->velocity_y += gravity * elapsedTime;

        // Position setup
        this->old_pos_x = this->pos_x;
        this->old_pos_y = this->pos_y;
        this->pos_x = this->pos_x + this->velocity_x * elapsedTime;
        this->pos_y = this->pos_y + this->velocity_y * elapsedTime;

        this->after_taking_damage_timeout.update(elapsedTime);
    }

    void start_taking_damage()
    {
        this->velocity_x = 0.05;
        this->velocity_y = -0.1;
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
        this->animations.at(current_animation).run(this->renderer, elapsedTime, this->face, int(this->pos_x), int(this->pos_y));
    }

    Region2D attack_region() const {
        auto collision_region_info = this->get_collision_region_information();
        auto const& collision_region = collision_region_info.collision_region;

        return Region2D{
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
    double old_pos_x;
    double old_pos_y;
    double pos_x;
    double pos_y;
    double velocity_x;
    double velocity_y;
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
    
    auto pig_collision_region_info = pig.get_collision_region_information();
    auto pig_collision_region = pig_collision_region_info.collision_region;

    auto player_collision_region_info = player.get_collision_region_information();
    auto player_collision_region = player_collision_region_info.collision_region;

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

void cannon_king_collision(Cannon* cannon_ptr, King* king)
{
    auto& player = *king;
    auto& cannon = *cannon_ptr;

    if (
        player.is_attacking &&
        !player.is_taking_damage
    ) {
        auto cannon_collision_region_info = cannon.get_collision_region_information();
        auto cannon_collision_region = cannon_collision_region_info.collision_region;
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

void compute_characters_collisions(std::vector<IGameCharacter*>& game_characters)
{
    for (int i = 0; i < game_characters.size(); ++i) {
        for (int j = i + 1; j < game_characters.size(); ++j) {
            if (false) {}

            else if (dynamic_cast<Pig*>(game_characters[i]) && dynamic_cast<King*>(game_characters[j])) { pig_king_collision(dynamic_cast<Pig*>(game_characters[i]), dynamic_cast<King*>(game_characters[j])); }
            else if (dynamic_cast<Pig*>(game_characters[j]) && dynamic_cast<King*>(game_characters[i])) { pig_king_collision(dynamic_cast<Pig*>(game_characters[j]), dynamic_cast<King*>(game_characters[i])); }

            else if (dynamic_cast<Cannon*>(game_characters[i]) && dynamic_cast<King*>(game_characters[j])) { cannon_king_collision(dynamic_cast<Cannon*>(game_characters[i]), dynamic_cast<King*>(game_characters[j])); }
            else if (dynamic_cast<Cannon*>(game_characters[j]) && dynamic_cast<King*>(game_characters[i])) { cannon_king_collision(dynamic_cast<Cannon*>(game_characters[j]), dynamic_cast<King*>(game_characters[i])); }

            else if (dynamic_cast<CannonBall*>(game_characters[i]) && dynamic_cast<King*>(game_characters[j])) { cannonball_king_collision(dynamic_cast<CannonBall*>(game_characters[i]), dynamic_cast<King*>(game_characters[j])); }
            else if (dynamic_cast<CannonBall*>(game_characters[j]) && dynamic_cast<King*>(game_characters[i])) { cannonball_king_collision(dynamic_cast<CannonBall*>(game_characters[j]), dynamic_cast<King*>(game_characters[i])); }

            else if (dynamic_cast<CannonBall*>(game_characters[i]) && dynamic_cast<Pig*>(game_characters[j])) { cannonball_pig_collision(dynamic_cast<CannonBall*>(game_characters[i]), dynamic_cast<Pig*>(game_characters[j])); }
            else if (dynamic_cast<CannonBall*>(game_characters[j]) && dynamic_cast<Pig*>(game_characters[i])) { cannonball_pig_collision(dynamic_cast<CannonBall*>(game_characters[j]), dynamic_cast<Pig*>(game_characters[i])); }
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
        {13, 14, 61, 39, 39, 39, 39, 61, 61, 39, 39, 39, 39, 39, 39, 39, 39, 15, 39, 39, 39, 13, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 39, 61, 61, 61, 61, 61, 61, 61, 61, 61, 15, 13, 13, 13, 13, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 39, 61, 61, 61, 61, 61, 61, 61, 61, 27, 13, 13, 13, 39, 39, 12, 13},
        {13, 14, 61, 39, 61, 61, 61, 61, 61, 39, 61, 61, 61, 61, 61, 61, 61, 13, 13, 13, 13, 13, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 61, 39, 61, 61, 61, 61, 61, 61, 13, 13, 13, 13, 13, 13, 12, 13},
        {13, 14, 39, 61, 61, 61, 61, 61, 61, 61, 61, 39, 61, 61, 61, 61, 61,  3, 39, 39, 39, 13, 39, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 15, 13, 13, 13, 13, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 15, 13, 13, 13, 13, 13, 12, 13},
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
    auto player = King(renderer, 120.0, 320.0);
    game_characters.push_back(&player);
    int n_pigs = 10;
    for (int i = 0; i < n_pigs; ++i) {
        auto pos_x = 300 + 5 * i;
        auto pos_y = 422;
        auto *pig = new Pig(renderer, pos_x, pos_y);
        game_characters.push_back(pig);

        pig->on_start_taking_damage = [&window_is_shaking, &window_shaker]() {
            window_is_shaking = true;
            window_shaker.restart();
        };
    }
    for (int i = 0; i < 2 * n_pigs; ++i) {
        auto pos_x = 300 + 5 * i;
        auto pos_y = 72;
        auto *pig = new Pig(renderer, pos_x, pos_y);
        game_characters.push_back(pig);

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
        player.pos_x = 120.0;
        player.pos_y = 320.0;
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
                    auto offset_x = 32 * (tile_id % 12);
                    auto offset_y = 32 * int(floor(tile_id / 12));
                    auto size = SDL_Rect{offset_x, offset_y, 32, 32};
                    auto position = SDL_Rect{32 * j + shake_x, 32 * i + shake_y, 32, 32};
                    SDL_RenderCopyEx(renderer, tileset, &size, &position, 0.0, nullptr, SDL_FLIP_NONE);

                    if (show_debug) {
                        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 40);
                        for (auto&& collision_tile_id : collision_tiles) {
                            if (tile_id == collision_tile_id) {
                                SDL_RenderFillRect(renderer, &position);
                            }
                        }
                    }                            
                }

                // Foreground
                {
                    auto tile_id = foreground[i][j];
                    auto offset_x = 32 * (tile_id % 7);
                    auto offset_y = 32 * int(floor(tile_id / 7));
                    auto size = SDL_Rect{offset_x, offset_y, 32, 32};
                    auto position = SDL_Rect{32 * j + shake_x, 32 * i + shake_y, 32, 32};
                    SDL_RenderCopyEx(renderer, foreground_set, &size, &position, 0.0, nullptr, SDL_FLIP_NONE);
                }

                // Interactibles
                {
                    auto size = SDL_Rect{0, 0, 46, 56};
                    auto position = SDL_Rect{128 + shake_x, 329 + shake_y, 46, 56};
                    SDL_RenderCopyEx(renderer, door, &size, &position, 0.0, nullptr, SDL_FLIP_NONE);                    
                }
                
                // HUD
                {
                    {
                        auto size = SDL_Rect{0, 0, 66, 34};
                        auto position = SDL_Rect{10, 10, 66, 34};
                        SDL_RenderCopyEx(renderer, lifebar, &size, &position, 0.0, nullptr, SDL_FLIP_NONE);
                    }

                    for (int i = 0; i < player.life; ++i) {
                        auto size = SDL_Rect{0, 0, 18, 14};
                        auto position = SDL_Rect{20 + 12 * i, 20, 18, 14};
                        SDL_RenderCopyEx(renderer, lifebar_heart, &size, &position, 0.0, nullptr, SDL_FLIP_NONE);
                    }
                }
            }
        }

        for (auto& game_character : game_characters) {
            game_character->run_animation(elapsedTime);
        }
        transition_animation.run(renderer, elapsedTime);

        debug_messages.push_back("Position: " + std::to_string(player.pos_x) + ", " + std::to_string(player.pos_y) + ")");
        if (show_debug) {
            auto r = (Uint8)(0);
            auto g = (Uint8)(0);
            auto b = (Uint8)(0);
            auto a = (Uint8)(0);
            SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 90);

            for (auto* c : game_characters) {
                auto collision_region_info = c->get_collision_region_information();
                auto collision_rect = to_sdl_rect(collision_region_info.collision_region);
                SDL_RenderFillRect(renderer, &collision_rect);
            }
            auto player_attack_region = player.attack_region();
            auto player_attack_rect = to_sdl_rect(player_attack_region);
            SDL_RenderFillRect(renderer, &player_attack_rect);

            debug_text(debug_messages, renderer, default_font, 10, 10);
            SDL_SetRenderDrawColor(renderer, r, g, b, a);
        }        
        
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

