#ifndef WORKER_HPP
#define WORKER_HPP

#include <cstddef>

namespace zubarev
{
  void runWorker(int req_fd, int resp_fd, size_t seed);
}

#endif
