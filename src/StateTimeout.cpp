#include <StateTimeout.hpp>
#include <utility>

StateTimeout::StateTimeout()
    : timeout(0.0)
    , function()
    , current_time(0.0)
    , started(false)
    {}

StateTimeout::StateTimeout(double timeout, std::function<void()>  f)
    : timeout(timeout)
    , function(std::move(f))
    , current_time(0.0)
    , started(false)
    {}

void StateTimeout::restart()
{
    this->current_time = timeout;
    this->started = true;
}

void StateTimeout::update(double elapsedTime)
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

