#ifndef DUELS_GRID_H
#define DUELS_GRID_H

#include <duels/utils/vector2d.h>

// A simple 2D grid containing some integers
namespace duels
{
class Grid
{
public:
  Grid(int _rows, int _cols) : rows(_rows), cols(_cols)
  {
    grid = new int[rows*cols];
    // init to 0
    for(auto elem = grid; elem != grid+rows*cols; elem++)
      *elem = 0;
  }
  ~Grid()
  {
    delete[] grid;
  }

  int & cell(int x, int y)
  {
    return grid[x+cols*y];
  }

  int cell(int x, int y) const
  {
    return grid[x+cols*y];
  }

  int & cell(const Vector2D<int> &v)
  {
    return grid[v.x+cols*v.y];
  }

  int cell(const Vector2D<int> &v) const
  {
    return grid[v.x+cols*v.y];
  }

  bool inBounds(int x, int y) const
  {
    return x >= 0 && y >= 0 && x < cols && y < rows;
  }
  bool inBounds(const Vector2D<int> &v) const
  {
    return v.x >= 0 && v.y >= 0 && v.x < cols && v.y < rows;
  }

  bool isFree(int x, int y) const
  {
    return inBounds(x, y) && cell(x,y) == 0;
  }

  int height() {return rows;}
  int width() {return cols;}

protected:
  const int rows;
  const int cols;
  int * grid;
};

}
#endif

