#include <GameController.hpp>

GameController::GameController()
{
    this->keyconfig = {
        { ControllerAction::StartKey, SDL_SCANCODE_RETURN },
        { ControllerAction::DebugKey, SDL_SCANCODE_TAB },
        { ControllerAction::AttackKey, SDL_SCANCODE_LCTRL },
        { ControllerAction::DashKey, SDL_SCANCODE_LSHIFT },
        { ControllerAction::JumpKey, SDL_SCANCODE_SPACE },
        { ControllerAction::UpKey, SDL_SCANCODE_UP },
        { ControllerAction::DownKey, SDL_SCANCODE_DOWN },
        { ControllerAction::LeftKey, SDL_SCANCODE_LEFT },
        { ControllerAction::RightKey, SDL_SCANCODE_RIGHT }
    };

    for (auto const& [k, _] : this->keyconfig) {
        this->keystate[k] = ControllerState::NotPressed;
    }
}

GameController::~GameController()
{
}

void GameController::update()
{
    auto keystate = SDL_GetKeyboardState(nullptr);
    for (auto const& [k, _] : this->keyconfig) {
        auto const& sdl_key = this->keyconfig[k];
        if (keystate[sdl_key]) {
            if (this->keystate[k] == ControllerState::NotPressed) {
                this->keystate[k] = ControllerState::JustPressed;
            } else {
                this->keystate[k] = ControllerState::Pressed;
            }
        } else {
            this->keystate[k] = ControllerState::NotPressed;
        }
    }
}

ControllerState GameController::get_state(ControllerAction const& action) const
{
    return this->keystate.at(action);
}

bool GameController::just_pressed(ControllerAction const& action) const
{
    return this->keystate.at(action) == ControllerState::JustPressed;
}

bool GameController::is_pressed(ControllerAction const& action) const
{
    return this->keystate.at(action) == ControllerState::Pressed;
}

GameController game_controller;
