#ifndef PIGSGAME_WINDOWSHAKER_HPP
#define PIGSGAME_WINDOWSHAKER_HPP

#include <StateTimeout.hpp>
#include <Vector2D.hpp>

class WindowShaker {
public:
    WindowShaker();
    [[nodiscard]] Vector2D<int> get_shake() const;
    void start_shake();
    void update(double elapsed_time);

private:
    StateTimeout state;
    bool is_shaking;
};

#endif //PIGSGAME_WINDOWSHAKER_HPP
