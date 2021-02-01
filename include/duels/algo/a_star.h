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

// keep track of who comes from who
// use a sorted vector based on node pointers
template <class T>
class PtrMap : std::vector<std::pair<std::unique_ptr<T>, T*>>
{
public:
    void update(T* node, T* parent)
    {
        for(auto &[cur_node,cur_parent]: *this)
        {
            if(cur_node.get() == node)
            {
                cur_parent = parent;
                return;
            }
        }
    }

    T* getParent(T* key)
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
std::vector<T> reconstructPath(PtrMap<T> &come_from, T* best)
{
    std::vector<T> path = {*best};
    // build list from end to start
    while(come_from.getParent(best))
    {
        best = come_from.getParent(best);
        path.push_back(*best);
    }
    // list from start to end
    std::reverse(path.begin(),path.end());
    return path;
}

// templated version of A* algorithm
template<class T>
std::vector<T> Astar(T start, T goal, bool shuffle = false)
{
    struct NodeWithCost
    {
        T* node;
        float f;
        float g;
        NodeWithCost(T* _node, float _h, float _g = 0)
            : node(_node), f(_h+_g), g(_g) {}
        bool operator<(const NodeWithCost & other) const
        {
            if(f == other.f)
                return g > other.g;
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
                if(it.node->is(node))
                    return &it;
            }
            return 0;
        }
    };

    std::vector<T*> closedSet;
    priority_access queue;
    queue.push({&start, start.h(goal)});

    // keep track of who comes from who
    PtrMap<T> come_from;

    while(!queue.empty())
    {
        auto best = queue.top();

        if(best.node->is(goal))
            return reconstructPath(come_from, best.node);

        closedSet.push_back(best.node);
        queue.pop();

        auto children = best.node->children();

        // to avoid equal costs inducing favorite directions
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
                const auto child_g = best.g + child_ptr->distToParent();
                if(!twin)
                {
                    queue.push({child_ptr,
                                child_ptr->h(goal),
                                child_g});
                    come_from.add(child, best.node);
                }
                else if(twin->g > child_g)
                {
                    come_from.update(twin->node, best.node);
                    queue.push({twin->node, twin->f - twin->g, child_g});
                }
            }
        }
    }
    return {};
}
}

#endif // A_STAR_H
