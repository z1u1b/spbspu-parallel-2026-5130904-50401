#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <future>
#include <random>
#include <functional>
namespace zubarev
{
  bool isInside(std::tuple< double, double, double > circle, double x, double y);
  std::pair< double, double > calc(const std::vector< std::tuple< double, double, double > >& circles,
      const size_t& tests, const size_t& seed, const double& min_x, const double& min_y, const double& max_x,
      const double& max_y);

  std::pair< double, double > area(std::vector< std::tuple< double, double, double > > circles, const size_t& tests,
      const size_t& seed, const double& min_x, const double& min_y, const double& max_x, const double& max_y,
      size_t threads);

}

int main(int argc, char** argv)
{
  if (argc != 3 && argc != 4)
  {
    std::cerr << "Invalid count of arguments" << '\n';
    return 1;
  }

  long long threads_arg = 0;
  long long tests_arg = 0;
  long long seed_arg = 0;

  try
  {
    size_t pos = 0;

    threads_arg = std::stoll(argv[1], &pos);
    if (argv[1][pos] != '\0')
    {
      throw;
    }

    tests_arg = std::stoll(argv[2], &pos);
    if (argv[2][pos] != '\0')
    {
      throw;
    }

    if (argc == 4)
    {
      seed_arg = std::stoll(argv[3], &pos);
      if (argv[3][pos] != '\0')
      {
        throw;
      }
    }
  }
  catch (...)
  {
    std::cerr << "Invalid type of arguments" << '\n';
    return 1;
  }

  if (threads_arg < 0)
  {
    std::cerr << "Invalid number of threads" << '\n';
    return 1;
  }
  if (threads_arg == 0)
  {
    threads_arg = 1;
  }
  if (tests_arg <= 0)
  {
    std::cerr << "Invalid number of tries" << '\n';
    return 1;
  }
  if (seed_arg < 0)
  {
    std::cerr << "Invalid seed" << '\n';
    return 1;
  }

  const size_t threads = static_cast< size_t >(threads_arg);
  const size_t tests = static_cast< size_t >(tests_arg);
  const size_t seed = static_cast< size_t >(seed_arg);

  double min_x = std::numeric_limits< double >::max();
  double min_y = std::numeric_limits< double >::max();
  double max_x = std::numeric_limits< double >::min();
  double max_y = std::numeric_limits< double >::min();

  std::vector< std::tuple< double, double, double > > circles;

  std::vector< double > record;
  double value = 0.0;
  while (std::cin >> value)
  {
    record.push_back(value);
    if (record.size() == 4)
    {
      const double r = record[0];
      const double x = record[2];
      const double y = record[3];

      circles.push_back(std::make_tuple(x, y, r));

      min_x = std::min(min_x, x - r);
      min_y = std::min(min_y, y - r);
      max_x = std::max(max_x, x + r);
      max_y = std::max(max_y, y + r);

      record.clear();
    }
  }

  if (!std::cin.eof() || !record.empty())
  {
    std::cerr << "Invalid data" << '\n';
    return 1;
  }

  if (circles.empty())
  {
    std::cout << 0 << " " << 0 << '\n';
    return 0;
  }

  const std::pair< double, double > result = zubarev::area(circles, tests, seed, min_x, min_y, max_x, max_y, threads);

  std::cout << std::setprecision(std::numeric_limits< double >::max_digits10) << result.first << " " << result.second
            << '\n';

  return 0;
}

bool zubarev::isInside(std::tuple< double, double, double > circle, double x, double y)
{
  const double circle_x = std::get< 0 >(circle);
  const double circle_y = std::get< 1 >(circle);
  const double circle_r = std::get< 2 >(circle);

  return (x - circle_x) * (x - circle_x) + (y - circle_y) * (y - circle_y) <= circle_r * circle_r;
}

std::pair< double, double > zubarev::calc(const std::vector< std::tuple< double, double, double > >& circles,
    const size_t& tests, const size_t& seed, const double& min_x, const double& min_y, const double& max_x,
    const double& max_y)
{
  std::default_random_engine gen(seed);
  std::uniform_real_distribution< double > dist_x(min_x, max_x);
  std::uniform_real_distribution< double > dist_y(min_y, max_y);

  size_t hits_one = 0;
  size_t hits_all = 0;

  for (size_t j = 0; j < tests; ++j)
  {
    bool is_hits_one = false;
    bool is_hits_all = true;

    const double x = dist_x(gen);
    const double y = dist_y(gen);
    for (size_t i = 0; i < circles.size(); ++i)
    {
      if (isInside(circles[i], x, y))
      {
        is_hits_one = true;
      }
      else
      {
        is_hits_all = false;
      }
    }
    if (is_hits_one)
    {
      hits_one++;
    }
    if (is_hits_all)
    {
      hits_all++;
    }
  }
  return std::make_pair(hits_one, hits_all);
}

std::pair< double, double > zubarev::area(std::vector< std::tuple< double, double, double > > circles,
    const size_t& tests, const size_t& seed, const double& min_x, const double& min_y, const double& max_x,
    const double& max_y, size_t threads)
{
  std::vector< std::future< std::pair< double, double > > > tasks(threads);

  for (size_t i = 0; i < threads; ++i)
  {
    size_t tests_per_thread = tests / threads;

    if (i < tests % threads)
    {
      tests_per_thread += 1;
    }

    tasks[i] = std::async(
        std::launch::async, calc, std::cref(circles), tests_per_thread, seed + i, min_x, min_y, max_x, max_y);
  }

  size_t count_in = 0;
  size_t count_in_all = 0;

  for (size_t i = 0; i < threads; ++i)
  {
    const std::pair< size_t, size_t > result_i = tasks[i].get();
    count_in += result_i.first;
    count_in_all += result_i.second;
  }

  return std::make_pair((max_x - min_x) * (max_y - min_y) * (static_cast< double >(count_in) / tests),
      (max_x - min_x) * (max_y - min_y) * (static_cast< double >(count_in_all) / tests));
}
