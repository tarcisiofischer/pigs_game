#include <WindowShaker.hpp>
#include <random.hpp>

WindowShaker::WindowShaker()
        : state()
        , is_shaking(false)
{
    this->state = StateTimeout(300., [&]() { this->is_shaking = false; });
}

Vector2D<int> WindowShaker::get_shake() const {
    if (!this->is_shaking) {
        return { 0, 0 };
    }
    return { random_int(-1, 1), random_int(-1, 1) };
}

void WindowShaker::start_shake() {
    this->is_shaking = true;
    this->state.restart();
}

void WindowShaker::update(double elapsed_time) {
    this->state.update(elapsed_time);
}
