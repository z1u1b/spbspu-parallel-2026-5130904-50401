#include "ipc.hpp"
#include <cerrno>
#include <system_error>
#include <unistd.h>
#include <utility>

size_t zubarev::send(int& err, int wr, const char* b, size_t k)
{
  size_t r = 0;
  while (r < k)
  {
    err = write(wr, b + r, k - r);
    if (err < 0)
    {
      break;
    }
    r += err;
  }
  return r;
}

size_t zubarev::recv(int& err, int rd, char* b, size_t k)
{
  size_t r = 0;
  while (r < k)
  {
    err = read(rd, b + r, k - r);
    if (err <= 0)
    {
      break;
    }
    r += err;
  }
  return r;
}

zubarev::Pipe::Pipe():
  fds_{-1, -1}
{
  if (::pipe(fds_) == -1)
  {
    throw std::system_error(errno, std::generic_category(), "pipe failed");
  }
}

zubarev::Pipe::~Pipe()
{
  closeAll();
}

zubarev::Pipe::Pipe(Pipe&& other) noexcept:
  fds_{other.fds_[0], other.fds_[1]}
{
  other.fds_[0] = -1;
  other.fds_[1] = -1;
}

zubarev::Pipe& zubarev::Pipe::operator=(Pipe&& other) noexcept
{
  if (this != &other)
  {
    closeAll();
    fds_[0] = other.fds_[0];
    fds_[1] = other.fds_[1];
    other.fds_[0] = -1;
    other.fds_[1] = -1;
  }
  return *this;
}

int zubarev::Pipe::readFd() const noexcept
{
  return fds_[0];
}

int zubarev::Pipe::writeFd() const noexcept
{
  return fds_[1];
}

int zubarev::Pipe::getReadFd() const noexcept
{
  return fds_[0];
}

int zubarev::Pipe::gerReadFd() const noexcept
{
  return fds_[0];
}

int zubarev::Pipe::releaseRead() noexcept
{
  const int fd = fds_[0];
  fds_[0] = -1;
  return fd;
}

int zubarev::Pipe::releaseWrite() noexcept
{
  const int fd = fds_[1];
  fds_[1] = -1;
  return fd;
}

void zubarev::Pipe::closeRead() noexcept
{
  if (fds_[0] != -1)
  {
    ::close(fds_[0]);
    fds_[0] = -1;
  }
}

void zubarev::Pipe::closeWrite() noexcept
{
  if (fds_[1] != -1)
  {
    ::close(fds_[1]);
    fds_[1] = -1;
  }
}

void zubarev::Pipe::closeAll() noexcept
{
  closeRead();
  closeWrite();
}
