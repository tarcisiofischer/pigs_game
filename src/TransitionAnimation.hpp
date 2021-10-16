#ifndef __TRANSITION_ANIMATION_HPP
#define __TRANSITION_ANIMATION_HPP

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <constants.hpp>
#include <functional>
#include <optional>

enum class TransitionAnimationState {
    blacking = 0,
    waiting = 1,
    clearing = 2,
    finished = 3
};

class TransitionAnimation {
public:
    TransitionAnimation();
    void reset();
    void register_transition_callback(std::function<void()> const& f);
    void run(SDL_Renderer* renderer, double elapsedTime);
    TransitionAnimationState current_state() const;

private:
    TransitionAnimationState animation_state;
    double wait_timeout;
    double transition_acceleration;
    double transition_velocity;
    double transition_width;
    std::optional<std::function<void()>> transition_callback;
};

#endif
