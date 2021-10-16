#ifndef __ANIMATION_HPP
#define __ANIMATION_HPP

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <Vector2D.hpp>
#include <drawing.hpp>
#include <functional>
#include <optional>
#include <vector>

class Animation {
public:
    Animation(SDL_Texture* spritesheet, std::vector<std::tuple<int, int>> frames, int framesize_x, int framesize_y,
        double animation_time);

    void set_on_finish_animation_callback(std::function<void()> const& f)
    {
        this->on_finish_animation = f;
    }

    void run(SDL_Renderer* renderer, double elapsedTime, int face, Vector2D<int> const& world_position,
        Vector2D<int> const& sprite_offset, Vector2D<int> const& camera_offset)
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

        auto offset = Vector2D<int> { frame_x * this->framesize_x, frame_y * this->framesize_y };
        auto size = Vector2D<int> { this->framesize_x, this->framesize_y };
        auto flip = (face == +1) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
        auto draw_position = world_position - sprite_offset;
        draw_sprite(renderer, this->spritesheet, offset, draw_position, size, camera_offset, flip);
    }

public:
    SDL_Texture* spritesheet;
    std::vector<std::tuple<int, int>> frames;
    double counter;
    int state;
    std::optional<std::function<void()>> on_finish_animation;
    int framesize_x;
    int framesize_y;
    double animation_time;
};

#endif
