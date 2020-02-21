#ifndef RANDUTILS_H
#define RANDUTILS_H

#include <random>

namespace duels
{
int fastrand_seed = 0;

inline int fastrand()
{
  fastrand_seed = (214013*fastrand_seed+2531011);
  return (fastrand_seed>>16)&0x7FFF;
}

inline int fastrand(int max)
{
  constexpr static float rand_denum(1./RAND_MAX);
  return ((fastrand_seed>>16)&0x7FFF)*rand_denum * max;
}
}

#endif // RANDUTILS_H
