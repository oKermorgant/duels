#ifndef DUELS_NODE_POINT_H
#define DUELS_NODE_POINT_H

#include <duels/utils/point.h>
#include <duels/utils/grid.h>

// A 2D point on a grid that is able to generate its neighboors for A*

namespace duels
{

class NodePoint : public Point<int>
{
public:

  NodePoint(int _x=0, int _y=0) : Point(_x, _y) {}

  double h(const NodePoint &other) const
  {
    // use 2D Manhattan
    return static_cast<double>(distance(other, true));
  }

protected:
  static Grid* grid;
};

}

#endif // DUELS_POINT_H
