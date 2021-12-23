#include <screens/TitleScreen.hpp>
#include <AssetsRegistry.hpp>
#include <Vector2D.hpp>
#include <constants.hpp>
#include <drawing.hpp>

#include <iostream>

TitleScreen::TitleScreen(
    std::function<void()> const& on_start_game_pressed,
    std::function<void()> const& on_exit_game_pressed
)
    : logo_timeout_done(false)
    , state(TitleScreen::State::SHOWING_TITLE)
    , selected_menu(TitleScreen::SelectedMenu::START_GAME)
    , on_start_game_pressed(on_start_game_pressed)
    , on_exit_game_pressed(on_exit_game_pressed)
{
    this->main_screen_timeout = StateTimeout(1000., [this](){
        this->logo_timeout_done = true;
    });
    this->main_screen_timeout.restart();
}

void TitleScreen::handle_controller(GameController const& controller)
{
    if (this->state == TitleScreen::State::SHOWING_TITLE) {
        return;
    }

    auto direction = 0u;
    if (controller.just_pressed(ControllerAction::UpKey)) {
        direction = +1;
    } else if (controller.just_pressed(ControllerAction::DownKey)) {
        direction = -1;
    }
    this->selected_menu = static_cast<TitleScreen::SelectedMenu>(
        ((int)(this->selected_menu) + direction) % (int)TitleScreen::SelectedMenu::SIZE
    );

    if (controller.just_pressed(ControllerAction::ActionKey)) {
        if (this->selected_menu == TitleScreen::SelectedMenu::START_GAME) {
             this->on_start_game_pressed();
        } else if (this->selected_menu == TitleScreen::SelectedMenu::EXIT_GAME) {
            this->on_exit_game_pressed();
        }
    }
}

void TitleScreen::update(double elapsed_time)
{
    main_screen_timeout.update(elapsed_time);
    if (this->logo_timeout_done) {
        this->state = TitleScreen::State::SHOWING_MAIN_MENU;
        return;
    }
}

void TitleScreen::render(SDL_Renderer* renderer) const
{
    if (this->state == TitleScreen::State::SHOWING_TITLE) {
        auto text_position = Vector2D<int> { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
        gout(renderer, assets_registry.monogram, text_position, "Pigs Game", RGBColor { 255, 255, 255 });
    } else if (this->state == TitleScreen::State::SHOWING_MAIN_MENU) {
        // TODO: Make actual buttons
        {
            auto text_color = (this->selected_menu == TitleScreen::SelectedMenu::START_GAME) ? RGBColor { 0, 200, 0 } : RGBColor { 255, 255, 255 };
            auto text_position = Vector2D<int> { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            gout(renderer, assets_registry.monogram, text_position, "Start new game", text_color);
        }
        {
            auto text_color = (this->selected_menu == TitleScreen::SelectedMenu::EXIT_GAME) ? RGBColor { 0, 200, 0 } : RGBColor { 255, 255, 255 };
            auto text_position = Vector2D<int> { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40};
            gout(renderer, assets_registry.monogram, text_position, "Exit", text_color);
        }
    }
}
