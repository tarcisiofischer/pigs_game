#ifndef __GAME_HANDLER_HPP
#define __GAME_HANDLER_HPP

#include <levels/IGameLevel.hpp>
#include <StateTimeout.hpp>
#include <sdl_wrappers.hpp>
#include <Vector2D.hpp>
#include <TransitionAnimation.hpp>

#include <characters/King.hpp>

#include <random.hpp>
#include <memory>

class SDL_Window;

class WindowShaker {
public:
    WindowShaker()
        : state()
        , is_shaking(false)
    {
        this->state = StateTimeout(300., [&](){ this->is_shaking = false; });
    }

    Vector2D<int> get_shake()
    {
        if (!this->is_shaking) {
            return {0, 0};
        }
        return {random_int(-1, 1), random_int(-1, 1)};
    }

    void update(double elapsed_time)
    {
        this->state.update(elapsed_time);
    }

private:
    StateTimeout state;
    bool is_shaking;
};

class GameHandler {
public:
    GameHandler(SDL_Window* window);

    void set_active_level(std::unique_ptr<IGameLevel>&& lvl);

    bool process_inputs();
    void update(double elapsed_time);
    void render(double elapsed_time);

    King* player();

    SDL_Renderer* get_renderer();
    TransitionAnimation& get_transition_animation();

private:
    void update_characters(double elapsed_time);
    void compute_collisions();

private:
    std::unique_ptr<IGameLevel> active_lvl;
    SDL_Renderer* renderer;

    WindowShaker window_shaker;
    TransitionAnimation transition_animation;

    bool enable_debug;
    std::vector<std::string> debug_messages;
};

#endif
