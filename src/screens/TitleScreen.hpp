#ifndef PIGSGAME_TITLESCREEN_HPP
#define PIGSGAME_TITLESCREEN_HPP

#include <StateTimeout.hpp>
#include <screens/IScreen.hpp>
#include <functional>

class TitleScreen : public IScreen
{
private:
    enum class State {
        SHOWING_TITLE,
        SHOWING_MAIN_MENU,
        DONE
    };

    enum class SelectedMenu {
        START_GAME,
        EXIT_GAME,
        SIZE
    };

public:
    TitleScreen(std::function<void()> const& on_start_game_pressed, std::function<void()> const& on_exit_game_pressed);
    TitleScreen(TitleScreen const& other) = delete;
    TitleScreen(TitleScreen&& other) = default;

    void handle_controller(GameController const& keystates) override;
    void update(double elapsed_time) override;
    void render(SDL_Renderer* renderer, double elapsed_time) override;

private:
    bool logo_timeout_done;
    StateTimeout main_screen_timeout;

    TitleScreen::State state;
    TitleScreen::SelectedMenu selected_menu;

    bool action_key_pressed;
    std::function<void()> on_start_game_pressed;
    std::function<void()> on_exit_game_pressed;
};

#endif //PIGSGAME_TITLESCREEN_HPP
