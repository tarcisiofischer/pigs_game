#ifndef __TRANSITION_ANIMATION_HPP
#define __TRANSITION_ANIMATION_HPP

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <functional>
#include <optional>

#include <constants.hpp>

class TransitionAnimation
{
public:
    static auto constexpr BLACKING = 0;
    static auto constexpr WAITING = 1;
    static auto constexpr CLEARING = 2;
    static auto constexpr FINISHED = 3;

    TransitionAnimation();
    void reset();
    void register_transition_callback(std::function<void()> const& f);
    void run(SDL_Renderer* renderer, double elapsedTime);

private:
    int animation_state;
    double wait_timeout;
    double transition_acceleration;
    double transition_velocity;
    double transition_width;
    std::optional<std::function<void()>> transition_callback;
};

#endif
