#ifndef SHAPES_HPP
#define SHAPES_HPP

#include <vector>

namespace zubarev
{
  struct Circle
  {
    double x = 0.0;
    double y = 0.0;
    double r = 0.0;

    Circle() = default;
    Circle(double x, double y, double r);
  };

  struct Frame
  {
    double min_x;
    double min_y;
    double max_x;
    double max_y;
  };

  Frame frame(const Circle& c);
  Frame frame(const std::vector< Circle >& circles);

  inline bool isInside(const Circle& c, double x, double y)
  {
    return (x - c.x) * (x - c.x) + (y - c.y) * (y - c.y) <= c.r * c.r;
  }
}

#endif
