#ifndef __STATETIMEOUT_HPP
#define __STATETIMEOUT_HPP

#include <functional>

class StateTimeout {
public:
    StateTimeout()
        : timeout(0.0)
        , function()
        , current_time(0.0)
        , started(false)
        {}
    
    StateTimeout(double timeout, std::function<void()> const& f)
        : timeout(timeout)
        , function(f)
        , current_time(0.0)
        , started(false)
        {}


    void restart()
    {
        this->current_time = timeout;
        this->started = true;
    }
    
    void update(double elapsedTime)
    {
        if (!this->started) {
            return;
        }

        this->current_time -= elapsedTime;
        if (this->current_time <= 0.0) {
            this->function();
            this->started = false;
        }
    }

private:
    double timeout;
    std::function<void()> function;
    double current_time;
    bool started;
};

#endif
