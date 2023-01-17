#include <TransitionAnimation.hpp>

TransitionAnimation::TransitionAnimation()
    : animation_state(TransitionAnimationState::finished)
    , wait_timeout(0.0)
    , transition_acceleration(0.0)
    , transition_velocity(0.0)
    , transition_width(0.0)
{
}

TransitionAnimationState TransitionAnimation::current_state() const
{
    return this->animation_state;
}

void TransitionAnimation::reset()
{
    this->animation_state = TransitionAnimationState::blacking;
    this->wait_timeout = 500.0;
    this->transition_acceleration = 0.01;
    this->transition_velocity = 0.0;
    this->transition_width = 0.0;
}

void TransitionAnimation::register_transition_callback(std::function<void()> const& f)
{
    this->transition_callback = f;
}

void TransitionAnimation::run(SDL_Renderer* renderer, double elapsedTime)
{
    if (this->animation_state == TransitionAnimationState::blacking) {
        this->transition_velocity += this->transition_acceleration * elapsedTime;
        this->transition_width += this->transition_velocity * elapsedTime;

        if (this->transition_width >= SCREEN_WIDTH) {
            this->transition_width = SCREEN_WIDTH;
            this->transition_velocity = 0.0;
            this->animation_state = TransitionAnimationState::waiting;

            if (this->transition_callback) {
                (*this->transition_callback)();
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        auto rect = SDL_Rect { 0, 0, int(this->transition_width), SCREEN_HEIGHT };
        SDL_RenderFillRect(renderer, &rect);
    } else if (this->animation_state == TransitionAnimationState::waiting) {
        this->wait_timeout -= elapsedTime;
        if (this->wait_timeout <= 0.0) {
            this->animation_state = TransitionAnimationState::clearing;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        auto rect = SDL_Rect { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
        SDL_RenderFillRect(renderer, &rect);
    } else if (this->animation_state == TransitionAnimationState::clearing) {
        this->transition_velocity += this->transition_acceleration * elapsedTime;
        this->transition_width -= this->transition_velocity * elapsedTime;
        if (this->transition_width <= 0.0) {
            this->transition_width = 0.0;
            this->transition_velocity = 0.0;
            this->animation_state = TransitionAnimationState::finished;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        auto rect = SDL_Rect { SCREEN_WIDTH - int(this->transition_width), 0, int(this->transition_width), SCREEN_HEIGHT };
        SDL_RenderFillRect(renderer, &rect);
    }
}
