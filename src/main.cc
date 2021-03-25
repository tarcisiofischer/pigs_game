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

auto constexpr WIDTH = 25;
auto constexpr HEIGHT = 18;
auto constexpr tile_size = 32;
auto constexpr SCREEN_WIDTH = tile_size * WIDTH;
auto constexpr SCREEN_HEIGHT = tile_size * HEIGHT;
auto constexpr player_size_x = 96;
auto constexpr player_size_y = 96;

auto constexpr gravity = 0.001;

auto constexpr collision_tiles = std::array<int, 10>{1, 3, 12, 14, 15, 25, 27, 37, 38, 39};

struct Region2D {
    double x;
    double y;
    double w;
    double h;
};

class IGameCharacter {
public:
    virtual void update(double elapsedTime) = 0;
    virtual void run_animation(double elapsedTime) = 0;
};

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

inline bool check_aabb_collision(double a_x, double a_y, double a_w, double a_h, double b_x, double b_y, double b_w, double b_h)
{
    return (
        a_x < b_x + b_w &&
        a_x + a_w > b_x &&
        a_y < b_y + b_h &&
        a_y + a_h > b_y
    );
}

void compute_collisions(
    std::array<std::array<int, WIDTH>, HEIGHT> const& tilemap,
    double player_collision_x, double player_collision_y,
    double player_collision_w, double player_collision_h,
    double o_player_collision_x, double o_player_collision_y,
    double player_collision_offset_x, double player_collision_offset_y,
    double& pos_x, double& pos_y, double& velocity_x, double& velocity_y,
    bool& is_grounded
)
{
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            auto tile_id = tilemap[i][j];
            for (auto&& collision_tile_id : collision_tiles) {
                if (tile_id == collision_tile_id) {
                    double tile_x = 32.0 * j;
                    double tile_y = 32.0 * i;
                    double tile_w = 32.0;
                    double tile_h = 32.0;
                    
                    if (check_aabb_collision(
                        player_collision_x, player_collision_y, player_collision_w, player_collision_h,
                        tile_x, tile_y, tile_w, tile_h
                    )) {
                        if (
                            player_collision_x + player_collision_w > tile_x &&
                            o_player_collision_x + player_collision_w <= tile_x
                        ) {
                            pos_x = tile_x - player_collision_offset_x - player_collision_w - 0.1;
                            velocity_x = 0.0;
                        } else if (
                            player_collision_x < tile_x + tile_w &&
                            o_player_collision_x >= tile_x + tile_w
                        ) {
                            pos_x = tile_x + tile_w - player_collision_offset_x + 0.1;
                            velocity_x = 0.0;
                        } else if (
                            player_collision_y < tile_y + tile_h &&
                            o_player_collision_y >= tile_y + tile_h
                        ) {
                            pos_y = tile_y + tile_h - player_collision_offset_y + 0.1;
                            velocity_y = 0.0;

                            velocity_y -= 0.01; // Force response
                        } else if (
                            player_collision_y + player_collision_h > tile_y &&
                            o_player_collision_y + player_collision_h <= tile_y
                        ) {
                            pos_y = tile_y - player_collision_h - player_collision_offset_y - 0.1;
                            velocity_y = 0.0;
                            is_grounded = true;
                        }
                    }
                }
            }
        }
    }
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

    void update(double elapsedTime) override
    {
    }
    
    void trigger_attack()
    {
        if (!this->is_attacking) {
            this->is_attacking = true;
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

    Region2D collision_region() const {
        auto constexpr player_collision_offset_x = 38.;
        auto constexpr player_collision_offset_y = 36.;
 
        return Region2D{
            this->pos_x + player_collision_offset_x,
            this->pos_y + player_collision_offset_y,
            20.0,
            27.0
        };
    }
    
    Region2D attack_region() const {
        auto collision_region = this->collision_region();
        return Region2D{
            collision_region.x + this->face * 30.,
            collision_region.y - 10.,
            collision_region.w + 10.,
            collision_region.h + 20.
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
        {13, 14, 39, 39, 39, 39, 39, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 15, 39, 39, 39, 13, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 39, 61, 61, 61, 61, 61, 61, 61, 61, 61, 15, 13, 13, 13, 13, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 39, 61, 61, 61, 61, 61, 61, 61, 61, 27, 13, 13, 13, 39, 39, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 39, 61, 61, 61, 61, 61, 61, 61, 13, 13, 13, 13, 13, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 61, 39, 61, 61, 61, 61, 61, 61, 13, 13, 13, 13, 13, 13, 12, 13},
        {13, 14, 61, 61, 61, 61, 61, 61, 61, 61, 61, 39, 61, 61, 61, 61, 61,  3, 39, 39, 39, 13, 39, 12, 13},
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
    std::vector<std::string> debug_messages;
    
    auto load_spritesheet = [&renderer](std::string const& filename) {
        return load_media("assets/sprites/" + filename, renderer);
    };
    auto tileset = load_spritesheet("tileset.png");
    auto foreground_set = load_spritesheet("foreground_set.png");
    auto lifebar = load_spritesheet("lifebar.png");
    auto lifebar_heart = load_spritesheet("small_heart18x14.png");
    auto door = load_spritesheet("door46x56.png");

    auto game_characters = std::vector<IGameCharacter*>();
    auto player = King(renderer, 120.0, 320.0);
    game_characters.push_back(&player);

    int n_pigs = 10;
    for (int i = 0; i < n_pigs; ++i) {
        auto pos_x = 300 + 5 * i;
        auto pos_y = 422;
        auto *pig = new Pig(renderer, pos_x, pos_y);
        game_characters.push_back(pig);
    }
    auto cannon = Cannon(renderer, 80.0, 64.0, -1);
    game_characters.push_back(&cannon);

    auto last = (unsigned long long)(0);
    auto current = SDL_GetPerformanceCounter();
    auto fps_countdown = 1000.;
    auto fps_counter = 0;
    auto fps = 0;
    auto window_is_shaking = false;
    auto window_shaker = StateTimeout(300., [&window_is_shaking](){ window_is_shaking = false; });
    
    auto transition_animation = TransitionAnimation();
    player.register_on_dead_callback([&transition_animation]() {
        transition_animation.reset();
    });
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
        // Player handling
        if (!player.is_taking_damage && !player.is_dying && !player.is_dead) {
            if (keystates[SDL_SCANCODE_LEFT]) {
                player.running_side = -1;
                player.face = -1;
            } else if (keystates[SDL_SCANCODE_RIGHT]) {
                player.running_side = +1;
                player.face = +1;
            } else {
                player.running_side = 0;
            }
            if (keystates[SDL_SCANCODE_UP]) {
                if (!player.is_jumping && !player.is_falling) {
                    player.start_jumping = true;
                }
            }
            if (keystates[SDL_SCANCODE_LCTRL]) {
                if (!player.is_attacking) {
                    player.is_attacking = true;
                }
            }
        }

        for (auto* c : game_characters) {
            c->update(elapsedTime);
        }

        // Player collision
        double player_collision_offset_x = 38.0;
        double player_collision_offset_y = 36.0;
        double o_player_collision_x = player.old_pos_x + player_collision_offset_x;
        double o_player_collision_y = player.old_pos_y + player_collision_offset_y;
        double player_collision_x = player.pos_x + player_collision_offset_x;
        double player_collision_y = player.pos_y + player_collision_offset_y;
        double player_collision_w = 20.0;
        double player_collision_h = 27.0;

        // Pig collision
        double pig_collision_offset_x = 30.0;
        double pig_collision_offset_y = 30.0;
        double pig_collision_w = 18.0;
        double pig_collision_h = 18.0;

        compute_collisions(
            tilemap,
            player_collision_x, player_collision_y,
            player_collision_w, player_collision_h,
            o_player_collision_x, o_player_collision_y,
            player_collision_offset_x, player_collision_offset_y,
            // Out:
            player.pos_x, player.pos_y, player.velocity_x, player.velocity_y,
            player.is_grounded
        );
        for (auto* c : game_characters) {
            auto* pig_ptr = dynamic_cast<Pig*>(c);
            if (!pig_ptr) continue;
            auto& pig = *pig_ptr;

            double pig_collision_x = pig.pos_x + pig_collision_offset_x;
            double pig_collision_y = pig.pos_y + pig_collision_offset_y;
            bool _1;
            double o_pig_collision_x = pig.old_pos_x + pig_collision_offset_x;
            double o_pig_collision_y = pig.old_pos_y + pig_collision_offset_y;

            compute_collisions(
                tilemap,
                pig_collision_x, pig_collision_y,
                pig_collision_w, pig_collision_h,
                o_pig_collision_x, o_pig_collision_y,
                pig_collision_offset_x, pig_collision_offset_y,
                // Out:
                pig.pos_x, pig.pos_y, pig.velocity_x, pig.velocity_y,
                _1
            );
        }
        player_collision_x = player.pos_x + player_collision_offset_x;
        player_collision_y = player.pos_y + player_collision_offset_y;
        player.is_falling = (!player.is_grounded && player.velocity_y > 0.0);
        player.is_jumping = (!player.is_grounded && player.velocity_y < 0.0);
        if (player.is_grounded && abs(player.pos_y - player.old_pos_y) > 0.1) { player.just_touched_ground = true; }
        
        auto player_attack_region = player.attack_region();
        for (auto* c : game_characters) {
            auto* pig_ptr = dynamic_cast<Pig*>(c);
            if (!pig_ptr) continue;
            auto& pig = *pig_ptr;

            double pig_collision_x = pig.pos_x + pig_collision_offset_x;
            double pig_collision_y = pig.pos_y + pig_collision_offset_y;

            if (
                player.is_attacking &&
                !player.is_taking_damage &&
                !pig.is_taking_damage &&
                !pig.is_dying &&
                !pig.is_dead
            ) {
                if (check_aabb_collision(
                    player_attack_region.x, player_attack_region.y, player_attack_region.w, player_attack_region.h,
                    pig_collision_x, pig_collision_y, pig_collision_w, pig_collision_h)
                ) {
                    pig.start_taking_damage();
                    window_is_shaking = true;
                    window_shaker.restart();
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
                if (check_aabb_collision(
                    player_collision_x, player_collision_y, player_collision_w, player_collision_h,
                    pig_collision_x, pig_collision_y, pig_collision_w, pig_collision_h)
                ) {
                    player.start_taking_damage();
                    window_is_shaking = true;
                    window_shaker.restart();
                }
            }
        }
        if (
            player.is_attacking &&
            !player.is_taking_damage
        ) {
            auto constexpr cannon_collision_offset_x = 20.;
            auto constexpr cannon_collision_offset_y = 20.;
            double cannon_collision_x = cannon.pos_x + cannon_collision_offset_x;
            double cannon_collision_y = cannon.pos_y + cannon_collision_offset_y;

            if (check_aabb_collision(
                player_attack_region.x, player_attack_region.y, player_attack_region.w, player_attack_region.h,
                cannon_collision_x, cannon_collision_y, pig_collision_w, pig_collision_h)
            ) {
                cannon.trigger_attack();
            }
        }
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

        if (show_debug) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 90);

            auto player_collision_rect = SDL_Rect{int(player_collision_x), int(player_collision_y), int(player_collision_w), int(player_collision_h)};
            SDL_RenderFillRect(renderer, &player_collision_rect);

            for (auto* c : game_characters) {
                auto* pig_ptr = dynamic_cast<Pig*>(c);
                if (!pig_ptr) continue;
                auto& pig = *pig_ptr;

                double pig_collision_x = pig.pos_x + pig_collision_offset_x;
                double pig_collision_y = pig.pos_y + pig_collision_offset_y;
                auto pig_collision_rect = SDL_Rect{
                    int(pig_collision_x),
                    int(pig_collision_y),
                    int(pig_collision_w),
                    int(pig_collision_h),
                };
                SDL_RenderFillRect(renderer, &pig_collision_rect);
            }
            
            auto attack_region = player.attack_region();
            auto player_weapon_rect = SDL_Rect{int(attack_region.x), int(attack_region.y), int(attack_region.w), int(attack_region.h)};
            SDL_RenderFillRect(renderer, &player_weapon_rect);

            debug_text(debug_messages, renderer, default_font, 10, 10);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        }        
        
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

