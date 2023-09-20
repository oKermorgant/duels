#ifndef DUELS_GAMESTATE_H
#define DUELS_GAMESTATE_H

#include <duels/stream_overloads.h>

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

inline std::ostream& operator<<(std::ostream& ss, const duels::State &state)
{
  ss << "{";
  ss << "result: " << state.result << ',';
  ss << "bond: " << state.bond << "}";
  return ss;
}

namespace YAML
{
template<>
struct convert<duels::State> {
  static bool decode(Node const& node, duels::State & rhs)
  {
    rhs.bond = node["bond"].as<duels::Bond>();
    rhs.result = node["result"].as<duels::Result>();
    return true;
  }
};
}

#endif // DUELS_GAMESTATE_H
