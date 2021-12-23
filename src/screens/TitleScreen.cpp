#include <screens/TitleScreen.hpp>
#include <AssetsRegistry.hpp>
#include <Vector2D.hpp>
#include <constants.hpp>
#include <drawing.hpp>

TitleScreen::TitleScreen()
    : logo_timeout_done(false)
    , state(TitleScreen::State::SHOWING_TITLE)
{
    this->main_screen_timeout = StateTimeout(1000., [this](){
        this->logo_timeout_done = true;
    });
    this->main_screen_timeout.restart();
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
    }
}
