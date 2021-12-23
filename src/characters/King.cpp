#include <characters/King.hpp>

King::King(SDL_Renderer* renderer, double pos_x, double pos_y)
    : running_side(0)
    , animations()
    , after_taking_damage_timeout()
    , face(+1)
    , life(2)
    , old_position { pos_x, pos_y }
    , position { pos_x, pos_y }
    , velocity { 0.0, 0.0 }
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
    register_animation(King::IDLE_ANIMATION,
        {
            { 1, 3 },
            { 2, 3 },
            { 3, 3 },
            { 4, 3 },
            { 5, 3 },
            { 6, 3 },
        },
        100.);
    register_animation(King::RUNNING_ANIMATION,
        {
            { 4, 4 },
            { 5, 4 },
            { 6, 4 },
            { 0, 5 },
            { 1, 5 },
            { 2, 5 },
            { 3, 5 },
        },
        100.);
    register_animation(King::JUMPING_ANIMATION,
        {
            { 0, 4 },
        },
        100.);
    register_animation(King::FALLING_ANIMATION,
        {
            { 1, 4 },
        },
        100.);
    register_animation(King::ATTACKING_ANIMATION,
        {
            { 3, 5 },
            { 4, 5 },
            { 5, 5 },
        },
        100.);
    register_animation(King::JUST_TOUCHED_GROUND_ANIMATION,
        {
            { 2, 4 },
        },
        150.);
    register_animation(King::TAKING_DAMAGE_ANIMATION,
        {
            { 1, 0 },
            { 2, 0 },
            { 4, 1 },
            { 2, 0 },
            { 4, 1 },
        },
        100.);
    register_animation(King::DYING_ANIMATION,
        {
            { 1, 0 },
            { 2, 0 },
            { 3, 0 },
            { 4, 0 },
            { 5, 0 },
        },
        100.);
    register_animation(King::DEAD_ANIMATION,
        {
            { 5, 0 },
        },
        100.);

    this->animations.at(ATTACKING_ANIMATION).set_on_finish_animation_callback([this]() { this->is_attacking = false; });
    this->animations.at(JUST_TOUCHED_GROUND_ANIMATION).set_on_finish_animation_callback([this]() {
        this->just_touched_ground = false;
    });
    this->animations.at(TAKING_DAMAGE_ANIMATION).set_on_finish_animation_callback([this]() {
        this->is_taking_damage = false;
        if (this->life > 0) {
            this->after_taking_damage = true;
            this->after_taking_damage_timeout.restart();
        } else {
            this->is_dying = true;
        }
    });
    this->after_taking_damage_timeout = StateTimeout(500., [this]() { this->after_taking_damage = false; });
    this->animations.at(DYING_ANIMATION).set_on_finish_animation_callback([this]() {
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

void King::set_position(double x, double y)
{
    this->position.x = x;
    this->position.y = y;
}

Vector2D<double> King::get_position() const
{
    return this->position;
}

Vector2D<double> King::get_velocity() const
{
    return this->velocity;
}

void King::set_velocity(double x, double y)
{
    this->velocity.x = x;
    this->velocity.y = y;
}

CollisionRegionInformation King::get_collision_region_information() const
{
    return CollisionRegionInformation(this->position, this->old_position, this->collision_size);
}

void King::handle_collision(const CollisionType& type, const CollisionSide& side)
{
    if (type == CollisionType::TILEMAP_COLLISION && side == CollisionSide::TOP_COLLISION) {
        this->set_velocity(0.0, +0.01); // Force response
    }

    if ((type == CollisionType::TILEMAP_COLLISION || type == CollisionType::FOREGROUND_COLLISION) && side == CollisionSide::BOTTOM_COLLISION) {
        this->is_grounded = true;
    }
}

void King::on_after_collision()
{
    this->is_falling = (!this->is_grounded && this->velocity.y < 0.0);
    this->is_jumping = (!this->is_grounded && this->velocity.y > 0.0);
    if (this->is_grounded && (this->position.y + 0.1) < this->old_position.y) {
        this->just_touched_ground = true;
    }
}

void King::handle_controller(GameController const& controller)
{
    if (this->is_taking_damage || this->is_dying || this->is_dead) {
        return;
    }

    if (controller.is_pressed(ControllerAction::LeftKey)) {
        this->running_side = -1;
        this->face = -1;
    } else if (controller.is_pressed(ControllerAction::RightKey)) {
        this->running_side = +1;
        this->face = +1;
    } else {
        this->running_side = 0;
    }

    if (controller.is_pressed(ControllerAction::UpKey)) {
        if (!this->is_jumping && !this->is_falling) {
            this->start_jumping = true;
        }
    }

    if (controller.is_pressed(ControllerAction::ActionKey)) {
        if (!this->is_attacking) {
            this->is_attacking = true;
        }
    }
}

void King::register_on_dead_callback(const std::function<void()>& f)
{
    this->on_dead_callback = f;
}

void King::update(double elapsedTime)
{
    // velocity x setup
    if (!this->is_taking_damage && !this->is_dying && !this->is_dead) {
        if (this->running_side == +1)
            this->velocity.x = +0.2;
        else if (this->running_side == -1)
            this->velocity.x = -0.2;
        else
            this->velocity.x = 0.0;

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

void King::start_taking_damage()
{
    this->velocity.x = 0.05;
    this->velocity.y = 0.1;
    this->is_taking_damage = true;
    this->life -= 1;
    if (this->on_start_taking_damage) {
        (*this->on_start_taking_damage)();
    }
}

void King::run_animation(double elapsedTime)
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
    this->animations.at(current_animation)
        .run(this->renderer, elapsedTime, this->face, this->position.as_int(), this->spritesheet_offset, camera_offset);
    for (auto& on_after_run_animation : this->on_after_run_animation_callbacks) {
        on_after_run_animation(this->renderer, this, elapsedTime);
    }
}

Region2D<double> King::attack_region() const
{
    auto const& collision_region = this->get_collision_region_information().collision_region;
    return { collision_region.x + this->face * attack_region_offset_x, collision_region.y + attack_region_offset_y,
        collision_region.w + attack_region_w, collision_region.h + attack_region_h };
}
