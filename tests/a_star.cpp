#include <duels/algo/a_star.h>
#include <duels/utils/grid_point.h>
#include <duels/utils/rand_utils.h>
#include <map>

int main()
{
  const int rows(20);
  const int cols(20);

  duels::Grid grid(rows, cols);

  for(int i = 0; i < rows*cols/10; ++i)
  {
    auto row(duels::fastrand(rows));
    auto col(duels::fastrand(cols));
    grid.cell(row, col) = 1;
  }

  // get initial and final points
  duels::GridPoint start(0,0), goal(rows-1, cols-1);
  grid.cell(start) = 0;
  grid.cell(goal) = 0;
  duels::GridPoint::setMap(grid, true);

  auto path = duels::Astar(start, goal, true);

  for(auto p: path)
    grid.cell(p) = 2;

  grid.cell(start) = 3;
  grid.cell(goal) = 4;

  std::map<int, char> display;
  display[0] = '.';
  display[1] = '#';
  display[2] = 'x';
  display[3] = 'S';
  display[4] = 'G';

  for(int row = 0; row < rows; ++row)
  {
    for(int col = 0; col < cols; ++col)
    {
      std::cout << display[grid.cell(row,col)] << " ";
    }
    std::cout << std::endl;
  }

}
