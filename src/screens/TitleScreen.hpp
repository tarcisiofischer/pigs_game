#ifndef PIGSGAME_TITLESCREEN_HPP
#define PIGSGAME_TITLESCREEN_HPP

#include <StateTimeout.hpp>
#include <sdl_wrappers.hpp>

class TitleScreen
{
private:
    enum class State {
        SHOWING_TITLE = 0,
        SHOWING_MAIN_MENU = 1
    };

public:
    TitleScreen();
    void update(double elapsed_time);
    void render(SDL_Renderer* renderer) const;

private:
    bool logo_timeout_done;
    StateTimeout main_screen_timeout;

    TitleScreen::State state;
};

#endif //PIGSGAME_TITLESCREEN_HPP
