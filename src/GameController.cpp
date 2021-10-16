#include <GameController.hpp>

GameController::GameController()
{
    this->keyconfig = { { ControllerAction::ActionKey, SDL_SCANCODE_LCTRL } };
    this->keystate = { { ControllerAction::ActionKey, ControllerState::NotPressed } };
}

GameController::~GameController()
{
}

void GameController::update()
{
    auto keystate = SDL_GetKeyboardState(nullptr);

    auto const& sdl_key = this->keyconfig[ControllerAction::ActionKey];
    if (keystate[sdl_key]) {
        if (this->keystate[ControllerAction::ActionKey] == ControllerState::NotPressed) {
            this->keystate[ControllerAction::ActionKey] = ControllerState::JustPressed;
        } else {
            this->keystate[ControllerAction::ActionKey] = ControllerState::Pressed;
        }
    } else {
        this->keystate[ControllerAction::ActionKey] = ControllerState::NotPressed;
    }
}

ControllerState GameController::get_state(ControllerAction const& action) const
{
    return this->keystate.at(action);
}

GameController game_controller;
