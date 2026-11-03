#ifndef MONTE_CARLO_HPP
#define MONTE_CARLO_HPP

#include <cstddef>
#include <vector>
#include "shapes.hpp"

namespace zubarev
{
  size_t calc(const std::vector< Circle >& circles, size_t tests, size_t seed, const Frame& f);
  double area(const std::vector< Circle >& circles, size_t threads, size_t tests, size_t seed);
}

#endif
