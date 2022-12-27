#include <items/Key.hpp>

extern Vector2D<int> camera_offset;

Key::Key(SDL_Renderer* renderer, double pos_x, double pos_y)
        : position { pos_x, pos_y }
        , renderer(renderer)
        , spritesheet(load_media("assets/sprites/key.png", renderer))
        , is_collected(false)
{
    auto register_animation = [&](int id, std::vector<std::tuple<int, int>> const& frames, double time) {
        this->animations.insert(std::make_pair(id, Animation(this->spritesheet, frames, SPRITESHEET_OFFSET, 16, 16, time)));
    };

    register_animation(Key::IDLE_ANIMATION,
                       {
                               { 0, 0 },
                       },
                       100.);
}

void Key::update(double elapsedTime)
{
    // Does nothing
}

void Key::run_animation(double elapsed_time)
{
    if (this->is_collected) {
        return;
    }

    auto current_animation = ([this]() {
        return IDLE_ANIMATION;
    })();

    this->animations.at(current_animation)
            .run(this->renderer, elapsed_time, 1, Vector2D<int> { int(this->position.x), int(this->position.y) }, camera_offset);
}

void Key::set_position(double x, double y)
{
    this->position.x = x;
    this->position.y = y;
}

Vector2D<double> Key::get_position() const
{
    return this->position;
}

Vector2D<double> Key::get_velocity() const
{
    return {0, 0};
}

void Key::set_velocity(double x, double y)
{
}

void Key::handle_collision(CollisionType const& type, CollisionSide const& side)
{
}

CollisionRegionInformation Key::get_collision_region_information() const
{
    return CollisionRegionInformation(this->position, this->position, this->collision_size);
}

void Key::on_after_collision()
{
}

void Key::collect()
{
    this->is_collected = true;
}
