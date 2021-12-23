#ifndef __GAME_HANDLER_HPP
#define __GAME_HANDLER_HPP

#include <StateTimeout.hpp>
#include <TransitionAnimation.hpp>
#include <GameTimeHandler.hpp>
#include <Vector2D.hpp>
#include <characters/King.hpp>
#include <levels/IGameLevel.hpp>
#include <memory>
#include <random.hpp>
#include <sdl_wrappers.hpp>
#include <screens/TitleScreen.hpp>

class SDL_Window;

class WindowShaker {
public:
    WindowShaker()
        : state()
        , is_shaking(false)
    {
        this->state = StateTimeout(300., [&]() { this->is_shaking = false; });
    }

    Vector2D<int> get_shake()
    {
        if (!this->is_shaking) {
            return { 0, 0 };
        }
        return { random_int(-1, 1), random_int(-1, 1) };
    }

    void start_shake() {
        this->is_shaking = true;
        this->state.restart();
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
    explicit GameHandler(SDL_Window* window);

    void set_active_level(std::unique_ptr<IGameLevel>&& lvl);

    void process_inputs();
    void update();
    void render();

    King* player();

    SDL_Renderer* get_renderer();
    TransitionAnimation& get_transition_animation();

    inline bool is_game_finished() const
    {
        return this->game_finished;
    }

private:
    void update_characters(double elapsed_time);
    void compute_collisions();
    void render_lvl();

private:
    std::unique_ptr<IGameLevel> active_lvl;
    SDL_Renderer* renderer;

    WindowShaker window_shaker;
    TransitionAnimation transition_animation;

    bool enable_debug;
    std::vector<std::string> debug_messages;
    GameTimeHandler time_handler;

    bool game_finished;

    // TODO: Make screen dynamic
    TitleScreen screen;
};

#endif
