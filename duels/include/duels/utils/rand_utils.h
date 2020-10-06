#ifndef RANDUTILS_H
#define RANDUTILS_H

#include <ctime>

namespace duels
{
static int fastrand_seed = 0;

namespace{
constexpr static float rand_denum(1.f/32767);
}

inline void randseed()
{
  fastrand_seed = static_cast<int>(time(nullptr));
}

inline int fastrand()
{
  fastrand_seed = (214013*fastrand_seed+2531011);
  return (fastrand_seed>>16)&0x7FFF;
}

inline int fastrand(int max)
{
  return fastrand()*rand_denum * max;
}

inline int fastrand(int min, int max)
{
  if(min == max)
    return min;
  return min + fastrand(max-min);
}

inline float fastrandf(float min=0, float max=1)
{
  if(min == max)
    return min;
  return min + fastrand()*rand_denum*(max-min);
}

}

#endif // RANDUTILS_H
