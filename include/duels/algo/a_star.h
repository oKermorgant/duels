#ifndef DUELS_A_STAR_H
#define DUELS_A_STAR_H

#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>

namespace duels
{

namespace
{
// compare pointer and object
template <class Node>
inline bool areSame(const Node * l1, const Node & l2)
{
    return *l1 == l2;
}

// resulting tree + owner of objects
template <class Node>
class Tree : public std::map<Node*, Node*>
{
    std::queue<std::unique_ptr<Node>> nodes;
    std::vector<Node*> closedSet;
public:
    inline void insert(std::unique_ptr<Node> & node, Node* parent)
    {
        std::map<Node*, Node*>::emplace(node.get(), parent);
        nodes.emplace(std::move(node));
    }
    inline void close(Node * node)
    {
        closedSet.push_back(node);
    }
    inline bool isVisited(Node * node_ptr) const
    {
        const auto &node(*node_ptr);
        return std::find_if(closedSet.rbegin(), closedSet.rend(),[&node](Node* elem)
        {return areSame(elem, node);}) != closedSet.rend();
    }
    std::vector<Node> fullPathTo(Node* best) const
    {
        std::vector<Node> path;
        const auto start(closedSet[0]);
        // build list from end to start
        while(best != start)
        {
            path.push_back(*best);
            best = this->at(best);
        }
        path.push_back(*start);
        // list from start to end
        std::reverse(path.begin(),path.end());
        return path;
    }
};

// Generic node with distance to start (g) + total cost from heuristic (f = g+h)
template <class Node, typename Heuristic>
struct NodeWithCost
{
    Node* node;
    Heuristic f;
    Heuristic g;
    inline NodeWithCost(Node* _node=nullptr, Heuristic _h=0, Heuristic _g=0)
        : node(_node), f(_h+_g), g(_g) {}
    bool operator<(const NodeWithCost & other) const
    {
        // if(f == other.f)
        //   return g < other.g;
        return f >= other.f;
    }
};

// Priory queue with find
template <class Node,typename Heuristic>
class Queue :
        public std::priority_queue<NodeWithCost<Node,Heuristic>, std::vector<NodeWithCost<Node,Heuristic>>>
{
public:
    std::pair<NodeWithCost<Node,Heuristic>,bool> find(const Node& node)
    {
        for(auto &it : this->c)
        {
            if(areSame(it.node, node))
                return {it,true};
        }
        return {{},false};
    }
};
}

// templated version of A* algorithm
template<class Node,typename Heuristic=float>
std::vector<Node> Astar(Node start, Node goal, bool shuffle = false)
{
    Queue<Node,Heuristic> queue;
    queue.push({&start, start.h(goal)});

    // keep track of who comes from who
    Tree<Node> tree;

    while(!queue.empty())
    {
        auto best = queue.top();

        if(areSame(best.node, goal))
            return tree.fullPathTo(best.node);

        tree.close(best.node);
        queue.pop();

        auto children = best.node->children();

        // to avoid equal costs inducing favorite directions
        if(shuffle)
            std::random_shuffle(children.begin(), children.end());

        for(auto &child: children)
        {
            auto child_ptr = child.get();
            // ensure we have not been here
            if(!tree.isVisited(child_ptr))
            {
                const auto child_g = best.g + child_ptr->distToParent();
                if(const auto &[twin,valid] = queue.find(*child_ptr);!valid)
                {
                    queue.push({child_ptr,
                                child_ptr->h(goal),
                                child_g});
                    tree.insert(child, best.node);
                }
                else if(twin.g > child_g)
                {
                    tree[twin.node] = best.node;
                    queue.push({twin.node, twin.f - twin.g, child_g});
                }
            }
        }
    }
    return {};
}
}

#endif // A_STAR_H
