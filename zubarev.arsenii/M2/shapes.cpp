#include "shapes.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>

zubarev::Circle::Circle(double x, double y, double r):
  x(x),
  y(y),
  r(r)
{
  if (r <= 0.0)
  {
    throw std::invalid_argument("Radius must be positive");
  }
}

zubarev::Frame zubarev::frame(const Circle& c)
{
  return Frame{c.x - c.r, c.y - c.r, c.x + c.r, c.y + c.r};
}

zubarev::Frame zubarev::frame(const std::vector< Circle >& circles)
{
  if (circles.empty())
  {
    return Frame{0.0, 0.0, 0.0, 0.0};
  }

  double min_x = std::numeric_limits< double >::max();
  double min_y = std::numeric_limits< double >::max();
  double max_x = std::numeric_limits< double >::lowest();
  double max_y = std::numeric_limits< double >::lowest();

  for (const Circle& c : circles)
  {
    const Frame f = frame(c);
    min_x = std::min(min_x, f.min_x);
    min_y = std::min(min_y, f.min_y);
    max_x = std::max(max_x, f.max_x);
    max_y = std::max(max_y, f.max_y);
  }

  return Frame{min_x, min_y, max_x, max_y};
}
