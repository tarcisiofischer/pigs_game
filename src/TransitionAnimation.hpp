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

    TransitionAnimation()
        : animation_state(FINISHED)
        , wait_timeout(0.0)
        , transition_acceleration(0.0)
        , transition_velocity(0.0)
        , transition_width(0.0)
    {
    }

    void reset()
    {
        this->animation_state = BLACKING;
        this->wait_timeout = 500.0;
        this->transition_acceleration = 0.01;
        this->transition_velocity = 0.0;
        this->transition_width = 0.0;
    }
    
    void register_transition_callback(std::function<void()> const& f)
    {
        this->transition_callback = f;
    }
    
    void run(SDL_Renderer* renderer, double elapsedTime)
    {
        if (this->animation_state == BLACKING) {
            this->transition_velocity += this->transition_acceleration * elapsedTime;
            this->transition_width += this->transition_velocity * elapsedTime;

            if (this->transition_width >= SCREEN_WIDTH) {
                this->transition_width = SCREEN_WIDTH;
                this->transition_velocity = 0.0;
                this->animation_state = WAITING;
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            auto rect = SDL_Rect{0, 0, int(this->transition_width), SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &rect);
        } else if (this->animation_state == WAITING) {
            if (this->transition_callback) {
                (*this->transition_callback)();
            }

            this->wait_timeout -= elapsedTime;
            if (this->wait_timeout <= 0.0) {
                this->animation_state = CLEARING;
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            auto rect = SDL_Rect{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &rect);
        } else if (this->animation_state == CLEARING) {
            this->transition_velocity += this->transition_acceleration * elapsedTime;
            this->transition_width -= this->transition_velocity * elapsedTime;
            if (this->transition_width <= 0.0) {
                this->transition_width = 0.0;
                this->transition_velocity = 0.0;
                this->animation_state = FINISHED;
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            auto rect = SDL_Rect{SCREEN_WIDTH - int(this->transition_width), 0, int(this->transition_width), SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &rect);
        }

    }

private:
    int animation_state;
    double wait_timeout;
    double transition_acceleration;
    double transition_velocity;
    double transition_width;
    std::optional<std::function<void()>> transition_callback;
};

#endif
