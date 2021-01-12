#ifndef DUELS_GAMESTATE_H
#define DUELS_GAMESTATE_H

#include <string>

namespace duels
{
    
enum State
{
    ONGOING=0,
    WIN_FAIR=1, WIN_TIMEOUT=2, WIN_DISCONNECT=3,
    LOSE_FAIR=4, LOSE_TIMEOUT=5,OTHER=6
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
