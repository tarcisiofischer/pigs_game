#ifndef __GAME_CONTROLLER_HPP
#define __GAME_CONTROLLER_HPP

#include <SDL.h>

#include <map>

enum class ControllerAction {
    ActionKey,
    UpKey,
    DownKey,
    LeftKey,
    RightKey,
    DebugKey
};

enum class ControllerState {
    NotPressed,
    JustPressed,
    Pressed
};

class GameController {
public:
    GameController();
    ~GameController();

    void update();
    ControllerState get_state(ControllerAction const& action) const;
    bool just_pressed(ControllerAction const& action) const;
    bool is_pressed(ControllerAction const& action) const;

private:
    std::map<ControllerAction, int> keyconfig;
    std::map<ControllerAction, ControllerState> keystate;
};

extern GameController game_controller;

#endif
