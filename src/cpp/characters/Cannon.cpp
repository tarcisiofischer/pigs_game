#include <characters/Cannon.hpp>

Cannon::Cannon(SDL_Renderer* renderer, double pos_x, double pos_y, int face)
    : face(face)
    , position { pos_x, pos_y }
    , is_attacking(false)
    , renderer(renderer)
    , spritesheet(load_media("assets/sprites/cannon96x96.png", renderer))
{
    auto register_animation = [&](int id, std::vector<std::tuple<int, int>> const& frames, double time) {
        this->animations.insert(std::make_pair(id, Animation(this->spritesheet, frames, SPRITESHEET_OFFSET, 96, 96, time)));
    };

    register_animation(Cannon::IDLE_ANIMATION,
        {
            { 0, 0 },
        },
        100.);
    register_animation(Cannon::ATTACKING_ANIMATION,
        {
            { 1, 0 },
            { 2, 0 },
            { 3, 0 },
            { 4, 0 },
        },
        100.);
}

void Cannon::set_on_before_fire(std::function<void()> const& f)
{
    this->on_before_fire = f;
}

void Cannon::update(double elapsedTime)
{
}

void Cannon::set_position(double x, double y)
{
}

Vector2D<double> Cannon::get_position() const
{
    return this->position;
}

Vector2D<double> Cannon::get_velocity() const
{
    return { 0.0, 0.0 };
}

void Cannon::set_velocity(double x, double y)
{
}

CollisionRegionInformation Cannon::get_collision_region_information() const
{
    return CollisionRegionInformation(this->position, this->position, this->collision_size);
}

void Cannon::handle_collision(CollisionType const& type, CollisionSide const& side)
{
}

void Cannon::on_after_collision()
{
}

void Cannon::trigger_attack()
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

void Cannon::run_animation(double elapsedTime, Vector2D<int> const& camera_offset)
{
    auto current_animation = ([this]() {
        if (this->is_attacking) {
            return ATTACKING_ANIMATION;
        }
        return IDLE_ANIMATION;
    })();

    this->animations.at(current_animation)
        .run(this->renderer, elapsedTime, this->face, Vector2D<int> { int(this->position.x), int(this->position.y) },
            camera_offset);
}
