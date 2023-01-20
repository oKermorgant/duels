#ifndef DUELS_PLAYER_H
#define DUELS_PLAYER_H

namespace duels
{
template <class Input, class Feedback>
class Player
{
public:
    Input input;
    Feedback feedback;

    // this is where the player changes its feedback to its input
    virtual void updateInput() = 0;
};
}

#endif // DUELS_PLAYER_H