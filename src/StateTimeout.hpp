#ifndef __STATETIMEOUT_HPP
#define __STATETIMEOUT_HPP

#include <functional>

class StateTimeout {
public:
    StateTimeout();    
    StateTimeout(double timeout, std::function<void()> const& f);

    void restart();
    void update(double elapsedTime);

private:
    double timeout;
    std::function<void()> function;
    double current_time;
    bool started;
};

#endif
