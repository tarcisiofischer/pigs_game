#include <characters/Pig.hpp>

Pig::Pig(Pig const& other)
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

Pig::~Pig() {}

void Pig::set_position(double x, double y)
{
    this->position.x = x;
    this->position.y = y;
}

Vector2D<double> Pig::get_position() const
{
    return this->position;
}

Vector2D<double> Pig::get_velocity() const
{
    return this->velocity;
}

void Pig::set_velocity(double x, double y)
{
    this->velocity.y = x;
    this->velocity.y = y;
}

CollisionRegionInformation Pig::get_collision_region_information() const
{
    return CollisionRegionInformation(this->position, this->old_position, this->collision_size);
}

void Pig::handle_collision(CollisionType const& type, CollisionSide const& side) {}

void Pig::on_after_collision() {}

void Pig::update(double elapsedTime)
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

void Pig::start_taking_damage()
{
    this->velocity.x = 0.05;
    this->velocity.y = -0.1;
    this->is_taking_damage = true;
    if (this->on_start_taking_damage) {
        (*this->on_start_taking_damage)();
    }
}

void Pig::run_animation(double elapsedTime)
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

void Pig::think(double elapsedTime)
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

void Pig::connect_callbacks()
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

Pig& Pig::operator=(Pig const& other)
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

Pig::Pig(SDL_Renderer* renderer, double pos_x, double pos_y)
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
