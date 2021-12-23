#ifndef __GAME_HANDLER_HPP
#define __GAME_HANDLER_HPP

#include <StateTimeout.hpp>
#include <GameTimeHandler.hpp>
#include <Vector2D.hpp>
#include <characters/King.hpp>
#include <levels/IGameLevel.hpp>
#include <memory>
#include <random.hpp>
#include <sdl_wrappers.hpp>
#include <screens/TitleScreen.hpp>
#include <TransitionAnimation.hpp>
#include <WindowShaker.hpp>

class SDL_Window;

class GameHandler {
public:
    explicit GameHandler(SDL_Window* window);

    void process_inputs();
    void update();
    void render();

    inline bool is_game_finished() const
    {
        return this->game_finished;
    }

    inline SDL_Renderer* get_renderer()
    {
        return this->renderer;
    }

    // TODO: const correctness
    inline TransitionAnimation& get_transition_animation()
    {
        return this->transition_animation;
    }

    // TODO: const correctness
    inline WindowShaker& get_window_shaker()
    {
        return this->window_shaker;
    }

    // TODO: Review this, check if really necessary workflow
    inline IScreen* get_active_screen()
    {
        return this->screen.get();
    }

    inline GameTimeHandler const& get_time_handler() const
    {
        return this->time_handler;
    }

private:
    SDL_Renderer* renderer;
    GameTimeHandler time_handler;
    bool game_finished;
    std::unique_ptr<IScreen> screen;
    WindowShaker window_shaker;
    TransitionAnimation transition_animation;
};

#endif
