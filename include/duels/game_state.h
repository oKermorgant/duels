#ifndef DUELS_GAMESTATE_H
#define DUELS_GAMESTATE_H

namespace duels
{
enum class Result:short {NONE=0, P1_WINS=1, P2_WINS=2, DRAW=3};
enum class Bond:short {OK=0, TIMEOUT=1, DISCONNECT=2};

struct State
{
    Result result=Result::NONE;
    Bond bond=Bond::OK;
    inline void set(Result result) {this->result = result;}
    inline void set(Bond bond) {this->bond = bond;}
    inline bool is(Result other) const {return result == other;}
    inline bool is(Bond other) const {return bond == other;}
    inline bool stillInGame() const
    {
        return is(Result::NONE) && is(Bond::OK);
    }
};

}

#endif // DUELS_GAMESTATE_H
