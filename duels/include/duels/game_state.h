#ifndef DUELS_GAMESTATE_H
#define DUELS_GAMESTATE_H

#include <string>

namespace duels
{
enum class State
{
    ONGOING,
    WIN_FAIR, WIN_TIMEOUT, WIN_DISCONNECT,
    LOSE_FAIR, LOSE_TIMEOUT
};

inline std::string winMsg(State state)
{
    switch(state)
    {
    case State::WIN_FAIR:
        return "fair";
        break;
    case State::WIN_TIMEOUT:
        return "timeout";
        break;
    case State::WIN_DISCONNECT:
        return "disconnect";
        break;
    default:
        return "";
    }
}

}

#endif // DUELS_GAMESTATE_H
