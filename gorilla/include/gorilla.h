#ifndef GORILLA_H
#define GORILLA_H

// game mechanics
#include <duels/gorilla/msg.h>

using namespace duels::gorilla;

class Gorilla
{
public:
  Gorilla(InitMsg &msg, int level = 1);

  void launchBanana(const InputMsg &msg);
  void writeState(FeedbackMsg &msg);

  bool nextTurn(DisplayMsg &msg);
  bool onAir(DisplayMsg &msg);

  int winner() const;

  bool p1_turn = false;


private:
  float grav = 9.81;
  float wind = 0;
  float drag = 0.5;
};

#endif // GORILLA_H
