#ifndef DUELS_Vector2D_H
#define DUELS_Vector2D_H

#include <cstdlib>
#include <math.h>
#include <ostream>
#include <yaml-cpp/yaml.h>

namespace duels
{

template <typename Numeric>
class Vector2D
{
public:

    Vector2D(Numeric _x=0, Numeric _y=0): x(_x), y(_y) {}

    Vector2D<Numeric>& operator=(const Vector2D &p)
    {
        x = p.x;
        y = p.y;
        return *this;
    }

    template <class Position>
    void writeInto(Position &p) const
    {
        p.x = x;
        p.y = y;
    }

    // 2 positions are equal if they have the same x and y
    inline bool operator==(const Vector2D &other) const
    {
        return x == other.x && y == other.y;
    }
    inline bool operator!=(const Vector2D &other) const
    {
        return x != other.x || y!=other.y;
    }

    double norm(bool use_manhattan = false) const
    {
        if(use_manhattan)
            return  std::abs(x) + std::abs(y);
        return sqrt(x*x + y*y);
    }

    double distance(const Vector2D &other, bool use_manhattan = false) const
    {
        if(use_manhattan)
            return  std::abs(x-other.x) + std::abs(y-other.y);
        return sqrt((x-other.x)*(x-other.x) + (y-other.y)*(y-other.y));
    }

    Numeric sq_distance(const Vector2D &other) const
    {
        return (x-other.x)*(x-other.x) + (y-other.y)*(y-other.y);
    }

    // math operations
    Vector2D operator+(const Vector2D &other) const
    {
      return {x+other.x, y+other.y};
    }
    Vector2D operator+=(const Vector2D &other)
    {
      x += other.x;
      y += other.y;
      return *this;
    }
    Vector2D operator-(const Vector2D &other) const
    {
      return {x-other.x, y-other.y};
    }
    Vector2D operator-=(const Vector2D &other)
    {
      x -= other.x;
      y -= other.y;
      return *this;
    }

    Vector2D operator*(float v) const
    {
      return {x*v, y*v};
    }
    Vector2D operator*=(float v)
    {
      x *= v;
      y *= v;
      return *this;
    }
    Vector2D operator/(float v) const
    {
      return {x/v, y/v};
    }
    Vector2D operator/=(float v)
    {
      x /= v;
      y /= v;
      return *this;
    }

    inline friend std::ostream& operator<<(std::ostream& ss, const duels::Vector2D<Numeric> &v)
    {
      ss << "{";
      ss << "x: " << v.x << ",";
      ss << "y: " << v.y << "}";
      return ss;
    }

    Numeric x{}, y{};
};

using Position2D = Vector2D<int>;
}

namespace YAML
{
template<typename Numeric>
struct convert<duels::Vector2D<Numeric>>
{
  static bool decode(Node const& node, duels::Vector2D<Numeric> & rhs)
  {
    rhs.x = node["x"].as<int>();
    rhs.y = node["y"].as<int>();
    return true;
  }
};
}


#endif // DUELS_Vector2D_H
