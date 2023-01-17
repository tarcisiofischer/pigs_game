#ifndef PIGSGAME_LOGGING_HPP
#define PIGSGAME_LOGGING_HPP

#include <string>
#include <iostream>

#define ENABLE_WARNINGS 1

using namespace std::string_literals;

inline void warn(std::string const& message)
{
#ifdef ENABLE_WARNINGS
    std::cout << "[WARNING]: "s + message + "\n"s;
#endif
}

inline void err(std::string const& message)
{
    throw std::runtime_error("[ERROR]: "s + message + "\n"s);
}

#endif //PIGSGAME_LOGGING_HPP
