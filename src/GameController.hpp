#ifndef __GAME_CONTROLLER_HPP
#define __GAME_CONTROLLER_HPP

#include <SDL.h>

#include <map>

enum class ControllerAction {
    ActionKey = 0,
    UpKey = 1,
    DownKey = 2
};

enum class ControllerState {
    NotPressed = 0,
    JustPressed = 1,
    Pressed = 2,
};

class GameController {
public:
    GameController();
    ~GameController();

    void update();
    ControllerState get_state(ControllerAction const& action) const;
    bool just_pressed(ControllerAction const& action) const;

private:
    std::map<ControllerAction, int> keyconfig;
    std::map<ControllerAction, ControllerState> keystate;
};

extern GameController game_controller;

#endif
