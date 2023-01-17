#include <Animation.hpp>
#include <utility>

Animation::Animation(
    SDL_Texture* spritesheet,
    std::vector<std::tuple<int, int>> frames,
    Vector2D<int> const& sprite_offset,
    int framesize_x,
    int framesize_y,
    double animation_time
)
    : spritesheet(spritesheet)
    , frames(std::move(frames))
    , sprite_offset(sprite_offset)
    , framesize_x(framesize_x)
    , framesize_y(framesize_y)
    , animation_time(animation_time)
    , state(0)
    , counter(0)
{
}

void Animation::set_on_finish_animation_callback(std::function<void()> const& f)
{
    this->on_finish_animation = f;
}

void Animation::run(
    SDL_Renderer* renderer,
    double elapsedTime,
    int face,
    Vector2D<int> const& world_position,
    Vector2D<int> const& camera_offset)
{
    this->counter += elapsedTime;
    if (this->counter >= this->animation_time) {
        this->state += 1;
        if ((this->state % this->frames.size()) == 0) {
            this->state = 0;
            if (this->on_finish_animation) {
                (*this->on_finish_animation)();
                return;
            }
        }
        this->counter = 0.0;
    }
    auto frame_x = int(0);
    auto frame_y = int(0);
    std::tie(frame_x, frame_y) = frames[state];

    auto offset = Vector2D<int> { frame_x * this->framesize_x, frame_y * this->framesize_y };
    auto size = Vector2D<int> { this->framesize_x, this->framesize_y };
    auto flip = (face == +1) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    auto draw_position = world_position - this->sprite_offset;
    draw_sprite(renderer, this->spritesheet, offset, draw_position, size, camera_offset, flip);
}
