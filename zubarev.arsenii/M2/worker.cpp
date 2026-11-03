#include "worker.hpp"
#include <chrono>
#include <cstring>
#include <vector>
#include "ipc.hpp"
#include "monte_carlo.hpp"
#include "shapes.hpp"

void zubarev::runWorker(int req_fd, int resp_fd, size_t seed)
{
  while (true)
  {
    RequestHeader req{};
    int err = 0;
    const size_t rec = recv(err, req_fd, reinterpret_cast< char* >(&req), sizeof(req));

    if (rec == 0 && err == 0)
    {
      break;
    }
    if (rec < sizeof(req) || err < 0)
    {
      break;
    }

    std::vector< Circle > circles(req.count);
    if (req.count > 0)
    {
      const size_t circles_bytes = req.count * sizeof(Circle);
      const size_t c_rec = recv(err, req_fd, reinterpret_cast< char* >(circles.data()), circles_bytes);
      if (c_rec < circles_bytes || err < 0)
      {
        break;
      }
    }

    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    const double calculated_area = area(circles, req.threads, req.tries, seed);
    const std::chrono::steady_clock::time_point finish = std::chrono::steady_clock::now();

    const std::chrono::duration< double, std::milli > elapsed = finish - start;

    Response resp{};
    std::strncpy(resp.name, req.name, sizeof(resp.name) - 1);
    resp.name[sizeof(resp.name) - 1] = '\0';
    resp.ms = elapsed.count();
    resp.area = calculated_area;

    const size_t sent = send(err, resp_fd, reinterpret_cast< const char* >(&resp), sizeof(resp));
    if (sent < sizeof(resp) || err < 0)
    {
      break;
    }
  }
}
