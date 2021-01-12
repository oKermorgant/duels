#ifndef GA_H
#define GA_H

#include <vector>
#include <thread>
#include <functional>
#include <chrono>
#include <algorithm>
#include <iostream>

#include <duels/utils/rand_utils.h>

namespace duels
{

template<class T>
bool operator<(const T &i1, const T& i2)
{
  return i1.cost() < i2.cost();
}

std::pair<uint, uint> different_randoms(int max)
{
  const int n1 = fastrand(max);
  int n2 = fastrand(max);
  while(n1 == n2)
    n2 = fastrand(max);
  return {n1,n2};
}

template <class T>
bool updateBest(T& old_best, const T&new_best)
{
  if(new_best < old_best)
  {
    old_best = new_best;
    return true;
  }
  return false;
}

// perform a single run with a random population
template<class T> class GA
{
private:
  uint full_population, keep_best, half_population;
  float max_iter;
  std::vector<T> population, selected;
public:
  GA(uint full_pop, float _max_iter):
    full_population(full_pop),
    keep_best(full_pop/10),
    half_population(full_pop/2),
    max_iter(_max_iter),
    population(std::vector<T>(full_pop)),
    selected(std::vector<T>(half_population-keep_best))
  {  }

  void solve(T &best)
  {
    for(auto &indiv: population)
      indiv.randomize();

    std::nth_element(population.begin(), population.begin()+keep_best,
                     population.end());
    best = *std::min_element(population.begin(),
                             population.begin()+keep_best);

    // loop until exit conditions
    uint iter=0;

    while(iter++ < max_iter)   // max iteration and max iteration where the best is always the same
    {
      // selection, 1 vs 1 tournament to fill half of the population
      for(auto & indiv: selected)
      {
        const auto idx = different_randoms(full_population);
        indiv = std::min(population[idx.first], population[idx.second]);
      }

      // put new elements at front of new population
      std::copy(selected.begin(), selected.end(),
                population.begin()+keep_best);

      // crossing and mutation to fill other half of the new pop
      for(uint i=half_population;i<full_population;++i)
      {
        const auto idx = different_randoms(half_population);
        // cross between parents + compute cost
        population[i].crossAndMutate(population[idx.first],population[idx.second]);
      }

      // re-sort from new costs
      std::nth_element(population.begin(), population.begin()+keep_best,
                       population.end());

      best = *std::min_element(population.begin(),
                               population.begin()+keep_best);
    }
  }
};
}
#endif // GA_H
