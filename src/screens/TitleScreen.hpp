#ifndef PIGSGAME_TITLESCREEN_HPP
#define PIGSGAME_TITLESCREEN_HPP

#include <StateTimeout.hpp>
#include <sdl_wrappers.hpp>
#include <GameController.hpp>
#include <functional>

class TitleScreen
{
private:
    enum class State {
        SHOWING_TITLE = 0,
        SHOWING_MAIN_MENU = 1
    };

    enum class SelectedMenu {
        START_GAME = 0,
        EXIT_GAME = 1,
        SIZE
    };

public:
    TitleScreen(std::function<void()> const& on_start_game_pressed, std::function<void()> const& on_exit_game_pressed);
    TitleScreen(TitleScreen const& other) = delete;
    TitleScreen(TitleScreen&& other) = default;
    void handle_controller(GameController const& keystates);
    void update(double elapsed_time);
    void render(SDL_Renderer* renderer) const;

private:
    bool logo_timeout_done;
    StateTimeout main_screen_timeout;

    TitleScreen::State state;
    TitleScreen::SelectedMenu selected_menu;

    std::function<void()> on_start_game_pressed;
    std::function<void()> on_exit_game_pressed;
};

#endif //PIGSGAME_TITLESCREEN_HPP
