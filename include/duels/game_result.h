#ifndef DUELS_GAMERESULT_H
#define DUELS_GAMERESULT_H

#include <iostream>
#include <array>
#include <duels/game_state.h>

namespace duels
{
namespace result
{

#ifdef DUELS_SERVER

inline Bond worstOf(Bond bond1, Bond bond2)
{
    if(bond1 == Bond::OK)
        return bond2;
    if(bond2 == Bond::OK)
        return bond1;
    return bond1 == Bond::TIMEOUT ? bond2 : bond1;
}

inline void print(const State &final, const std::string &name1, const std::string &name2)
{
    if(final.stillInGame())
    {
        std::cerr << "Sending result but no victory or draw was registered" << std::endl;
        return;
    }
    std::cout << "Winner: ";
    if(final.is(Result::DRAW))
        std::cout << "_draw / none_";
    else
        std::cout << (final.is(Result::P1_WINS) ? name1 : name2);

    switch (final.bond)
    {
    case Bond::OK:
        std::cout << " (fair)";
        break;
    case Bond::TIMEOUT:
        std::cout << " (timeout)";
        break;
    case Bond::DISCONNECT:
        std::cout << " (disconnect)";
        break;
    }
    std::cout << std::endl;
}
#else

inline bool isFinal(State state, Result you_win, bool looks_like_timeout)
{
   // std::cout << "isFinal? result = " << static_cast<short>(state.result)
     //         << " / bond = " << static_cast<short>(state.bond) << std::endl;


    if(state.stillInGame())
        return false;

    if(state.is(you_win))
    {
        std::cout << "You win! ";
        switch(state.bond)
        {
        case Bond::OK:
            std::cout << "Fair victory" << std::endl;
            break;
        case Bond::TIMEOUT:
            std::cout << "Opponent has timed out" << std::endl;
            break;
        case Bond::DISCONNECT:
            std::cout << "Opponent has crashed" << std::endl;
            break;
        }
    }
    else if(state.is(Result::DRAW))
    {
        std::cout << "It is a draw" << std::endl;
    }
    else if(state.is(Result::NONE))
    {
        std::cout << "Game has stopped - very long timeout from you or your opponent";
        if(looks_like_timeout)
            std::cout << " (it looks like it could be you)";
        std::cout << std::endl;
    }
    else
    {
        std::cout << "You lose! ";
        switch(state.bond)
        {
        case Bond::OK:
            std::cout << "Fair game" << std::endl;
            break;
        case Bond::TIMEOUT:
            std::cout << "Timed out..." << std::endl;
            break;
        case Bond::DISCONNECT:
            std::cout << "You have crashed (and should not see this message)" << std::endl;
            break;
        }
    }
    return true;
}
#endif
}
}

#endif
