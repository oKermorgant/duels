#ifndef DUELS_A_STAR_H
#define DUELS_A_STAR_H

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <queue>
#include <chrono>
#include <memory>

namespace duels
{

// a custom map for unique ptr's
template <class T>
class PtrMap : std::vector<std::pair<std::unique_ptr<T>, T*>>
{
public:
  void write(T* key, T* val)
  {
    for(auto &v: *this)
    {
      if(v.first.get() == key)
      {
        v.second = val;
        return;
      }
    }
  }

  T* get(T* key)
  {
    for(auto &v: *this)
    {
      if(v.first.get() == key)
      {
        return v.second;
      }
    }
    return 0;
  }

  // insertion
  void add(std::unique_ptr<T> &key, T* val)
  {
    this->push_back({std::move(key), val});
  }
};


// reconstruct path from last best element
template<class T>
void reconstructPath(PtrMap<T> &come_from, T* best, int dist)
{
  std::vector<T*> summary = {best};
  // build list from end to start
  while(come_from.get(best))
  {
    best = come_from.get(best);
    summary.push_back(best);
  }
  // list from start to end
  std::reverse(summary.begin(),summary.end());
}

// templated version of A* algorithm
template<class T>
void Astar(T start, T goal, bool shuffle = false)
{
  auto t0 = std::chrono::system_clock::now();

  typedef std::unique_ptr<T> Tptr;

  struct NodeWithCost
  {
    T* elem;
    int f;
    int g;
    NodeWithCost(T* _elem, int _h, int _g)
      : elem(_elem), f(_h+_g), g(_g) {}
    bool operator<(const NodeWithCost & other) const
    {
      return f > other.f;
    }
  };

  class priority_access :
      public std::priority_queue<NodeWithCost, std::vector<NodeWithCost>>
  {
  public:
    NodeWithCost* find( const T& node)
    {
      for(auto &it : this->c)
      {
        if(it.elem->is(node))
          return &it;
      }
      return 0;
    }
  };

  std::vector<T*> closedSet;
  priority_access queue;
  queue.push({&start, start.h(goal, use_manhattan), 0});

  // keep track of who comes from who
  PtrMap<T> come_from;

  while(!queue.empty())
  {
    auto best = queue.top();

    if(best.elem->is(goal))
    {
      reconstructPath(come_from, best.elem, best.g);
      return;
    }

    closedSet.push_back(best.elem);
    queue.pop();

    auto children = best.elem->children();

    // to avoid equal costs leading to favorite directions
    if(shuffle)
      std::random_shuffle(children.begin(), children.end());
    
    for(auto &child: children)
    {
      auto child_ptr = child.get();
      // ensure we have not been here
      if(std::find_if(closedSet.rbegin(), closedSet.rend(),
                      [&child_ptr](T* elem){return elem->is(*child_ptr);}) == closedSet.rend())
      {
        auto twin = queue.find(*child_ptr);
        const int child_g = best.g + child_ptr->distToParent();
        if(!twin)
        {
          queue.push({child_ptr,
                      child_ptr->h(goal),
                      child_g});
          come_from.add(child, best.elem);
        }
        else if(twin->g > child_g)
        {
          come_from.write(twin->elem, best.elem);
          queue.push({twin->elem, twin->f - twin->g, child_g});
        }
      }
    }
  }
}

}

#endif // A_STAR_H
