#include <AssetsRegistry.hpp>
#include <SoundHandler.hpp>
#include <characters/Pig.hpp>

Pig::Pig(SDL_Renderer* renderer, double pos_x, double pos_y)
    : running_side(0)
    , position { pos_x, pos_y }
    , old_position { pos_x, pos_y }
    , velocity { 0.0, 0.0 }
    , renderer(renderer)
    , spritesheet(load_media("assets/sprites/pig80x80.png", renderer))
    , think_timeout(1000.)
    , is_taking_damage(false)
    , life(2)
    , is_dying(false)
    , is_dead(false)
    , is_talking(false)
    , is_angry(false)
    , is_fear(false)
    , talking_message("")
    , talk_color { 0, 0, 0 }
{
    auto register_animation = [&](int id, std::vector<std::tuple<int, int>> const& frames, double time) {
        this->animations.insert(std::make_pair(id, Animation(this->spritesheet, frames, 80, 80, time)));
    };

    register_animation(Pig::IDLE_ANIMATION,
        {
            { 0, 1 },
            { 1, 1 },
            { 2, 1 },
            { 3, 1 },
            { 4, 1 },
        },
        100.);
    register_animation(Pig::RUNNING_ANIMATION,
        {
            { 5, 1 },
            { 0, 2 },
            { 1, 2 },
            { 2, 2 },
            { 3, 2 },
            { 4, 2 },
        },
        100.);
    register_animation(Pig::TAKING_DAMAGE_ANIMATION,
        {
            { 1, 4 },
            { 2, 4 },
            { 1, 4 },
            { 2, 4 },
        },
        100.);
    register_animation(Pig::DYING_ANIMATION,
        {
            { 3, 4 },
            { 4, 4 },
            { 5, 4 },
            { 0, 5 },
        },
        100.);
    register_animation(Pig::TALKING_ANIMATION,
        {
            { 1, 5 },
            { 2, 5 },
            { 3, 5 },
            { 4, 5 },
            { 5, 5 },
            { 0, 6 },
            { 1, 6 },
        },
        100.);
    register_animation(Pig::ANGRY_ANIMATION,
        {
            { 5, 6 },
            { 0, 7 },
        },
        100.);
    register_animation(Pig::ANGRY_TALKING_ANIMATION,
        {
            { 1, 7 },
            { 2, 7 },
            { 3, 7 },
            { 4, 7 },
            { 5, 7 },
        },
        100.);
    register_animation(Pig::FEAR_ANIMATION,
        {
            { 0, 8 },
            { 1, 8 },
        },
        100.);

    this->connect_callbacks();
}

Pig::Pig(Pig const& other)
{
    this->running_side = other.running_side;

    // Force callbacks re-connection, so that the lambda functions are up-to-date
    // in relation to the lambda captures
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
    this->is_talking = other.is_talking;
    this->is_angry = other.is_angry;
    this->is_fear = other.is_fear;
}

Pig::~Pig()
{
}

void Pig::set_position(double x, double y)
{
    this->position.x = x;
    this->position.y = y;
}

void Pig::set_script(SceneScript&& s)
{
    this->script = std::move(s);
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

void Pig::handle_collision(CollisionType const& type, CollisionSide const& side)
{
}

void Pig::on_after_collision()
{
}

void Pig::update(double elapsedTime)
{
    // velocity x setup
    if (!this->is_taking_damage && !this->is_dead && !this->is_dying) {
        this->think(elapsedTime);
    } else {
        this->running_side = 0;
    }

    if (this->running_side == +1)
        this->velocity.x = +0.05;
    else if (this->running_side == -1)
        this->velocity.x = -0.05;
    else
        this->velocity.x = 0.0;
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
    sound_handler.play("hit");
}

void Pig::run_animation(double elapsed_time)
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

        if (this->is_fear) {
            return FEAR_ANIMATION;
        }

        if (this->is_angry && this->is_talking) {
            return ANGRY_TALKING_ANIMATION;
        }
        if (this->is_angry) {
            return ANGRY_ANIMATION;
        }

        if (this->is_talking) {
            return TALKING_ANIMATION;
        }
        return IDLE_ANIMATION;
    })();
    this->animations.at(current_animation)
        .run(this->renderer, elapsed_time, -this->face, Vector2D<int> { int(this->position.x), int(this->position.y) },
            this->spritesheet_offset, camera_offset);
    if (this->is_talking) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        auto player_world_position = this->get_position().as_int();
        auto player_camera_position = to_camera_position(player_world_position + Vector2D<int> { 10, 40 }, { 0, 0 }, camera_offset);
        // Talking area
        auto rect = to_sdl_rect(Region2D<int> {
            player_camera_position.x - 5 * SCALE_SIZE,
            player_camera_position.y - 5 * SCALE_SIZE,
            (5 + int(this->talking_message.size()) * 6 + 5) * SCALE_SIZE,
            (5 + 6 * 1 + 5) * SCALE_SIZE,
        });
        SDL_RenderFillRect(renderer, &rect);

        {
            auto srcrect = SDL_Rect { 0, 0, 5, 4 };
            auto dstrect = SDL_Rect { rect.x - 5 * SCALE_SIZE, rect.y - 4 * SCALE_SIZE, 5 * SCALE_SIZE, 4 * SCALE_SIZE };
            SDL_RenderCopy(renderer, assets_registry.talk_baloon, &srcrect, &dstrect);
        }
        {
            auto srcrect = SDL_Rect { 20, 4, 1, 4 };
            auto dstrect = SDL_Rect { rect.x, rect.y - 4 * SCALE_SIZE, rect.w, 5 * SCALE_SIZE };
            SDL_RenderCopy(renderer, assets_registry.talk_baloon, &srcrect, &dstrect);
        }
        {
            auto srcrect = SDL_Rect { 10, 0, 5, 4 };
            auto dstrect = SDL_Rect { rect.x + rect.w, rect.y - 4 * SCALE_SIZE, 5 * SCALE_SIZE, 4 * SCALE_SIZE };
            SDL_RenderCopy(renderer, assets_registry.talk_baloon, &srcrect, &dstrect);
        }
        {
            auto srcrect = SDL_Rect { 25, 0, 5, 1 };
            auto dstrect = SDL_Rect { rect.x + rect.w, rect.y, 5 * SCALE_SIZE, rect.h };
            SDL_RenderCopy(renderer, assets_registry.talk_baloon, &srcrect, &dstrect);
        }
        {
            auto srcrect = SDL_Rect { 15, 0, 5, 4 };
            auto dstrect = SDL_Rect { rect.x + rect.w, rect.y + rect.h, 5 * SCALE_SIZE, 4 * SCALE_SIZE };
            SDL_RenderCopy(renderer, assets_registry.talk_baloon, &srcrect, &dstrect);
        }
        {
            auto srcrect = SDL_Rect { 20, 0, 1, 4 };
            auto dstrect = SDL_Rect { rect.x, rect.y + rect.h, rect.w, 4 * SCALE_SIZE };
            SDL_RenderCopy(renderer, assets_registry.talk_baloon, &srcrect, &dstrect);
        }
        {
            auto srcrect = SDL_Rect { 5, 0, 5, 4 };
            auto dstrect = SDL_Rect { rect.x - 5 * SCALE_SIZE, rect.y + rect.h, 5 * SCALE_SIZE, 4 * SCALE_SIZE };
            SDL_RenderCopy(renderer, assets_registry.talk_baloon, &srcrect, &dstrect);
        }
        {
            auto srcrect = SDL_Rect { 25, 4, 5, 1 };
            auto dstrect = SDL_Rect { rect.x - 5 * SCALE_SIZE, rect.y, 5 * SCALE_SIZE, rect.h };
            SDL_RenderCopy(renderer, assets_registry.talk_baloon, &srcrect, &dstrect);
        }
        {
            auto srcrect = SDL_Rect { 0, 4, 5, 4 };
            auto dstrect = SDL_Rect { rect.x + 15 * SCALE_SIZE, rect.y + rect.h + 3 * SCALE_SIZE, 5 * SCALE_SIZE, 4 * SCALE_SIZE };
            SDL_RenderCopy(renderer, assets_registry.talk_baloon, &srcrect, &dstrect);
        }

        gout(this->renderer, assets_registry.monogram, player_camera_position, this->talking_message, this->talk_color);
    }
}

void Pig::run_left()
{
    this->running_side = -1;
    this->face = -1;
}

void Pig::run_right()
{
    this->running_side = +1;
    this->face = +1;
}

void Pig::stop()
{
    this->running_side = 0;
}

void Pig::turn_to(int face)
{
    this->face = face;
}

void Pig::talk(std::string const& message, RGBColor const& talk_color)
{
    this->stop();
    this->is_talking = true;
    this->talking_message = message;
    this->talk_color = talk_color;
}

void Pig::set_angry(bool angry)
{
    this->is_angry = angry;
}

void Pig::set_fear(bool fear)
{
    this->is_fear = fear;
}

void Pig::think(double elapsed_time)
{
    if (this->script) {
        (*this->script).run(this, elapsed_time);
    } else {
        this->think_timeout -= elapsed_time;
        if (this->think_timeout <= 0.) {
            switch (random_int(0, 2)) {
            case 0: {
                this->run_left();
                break;
            };
            case 1: {
                this->stop();
                break;
            };
            case 2: {
                this->run_right();
                break;
            };
            }
            this->think_timeout = 1000.;
        }
    }
}

int Pig::get_dynamic_property(int property_id) const
{
    if (property_id == SceneScriptLinePropertyId) {
        if (this->script) {
            return this->script->get_active_script_line();
        }
    }
    throw std::runtime_error("Unknown property");
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
    this->animations.at(DYING_ANIMATION).set_on_finish_animation_callback([this]() { this->is_dead = true; });
}

Pig& Pig::operator=(Pig const& other)
{
    this->running_side = other.running_side;

    // Force callbacks re-connection, so that the lambda functions are up-to-date
    // in relation to the lambda captures
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
