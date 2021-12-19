#ifndef PIGSGAME_GAMETIMEHANDLER_HPP
#define PIGSGAME_GAMETIMEHANDLER_HPP

class GameTimeHandler
{
public:
    GameTimeHandler();

    void update();

    inline unsigned long long get_fps() const
    {
        return this->fps;
    }

    inline double get_elapsed_time() const
    {
        return this->elapsed_time;
    }

private:
    unsigned long long last;
    unsigned long long current;
    unsigned long long fps_counter;
    unsigned long long fps;
    double fps_countdown;
    double elapsed_time;
};

#endif //PIGSGAME_GAMETIMEHANDLER_HPP
