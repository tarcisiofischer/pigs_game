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
    Animation(
        SDL_Texture* spritesheet,
        std::vector<std::tuple<int, int>> frames,
        Vector2D<int> const& sprite_offset,
        int framesize_x,
        int framesize_y,
        double animation_time
    );

    void set_on_finish_animation_callback(std::function<void()> const& f);
    void run(
        SDL_Renderer* renderer,
        double elapsedTime,
        int face,
        Vector2D<int> const& world_position,
        Vector2D<int> const& camera_offset
    );

public:
    SDL_Texture* spritesheet;
    std::vector<std::tuple<int, int>> frames;
    Vector2D<int> sprite_offset;
    int framesize_x;
    int framesize_y;
    double animation_time;
    int state;
    double counter;
    std::optional<std::function<void()>> on_finish_animation;
};

#endif
