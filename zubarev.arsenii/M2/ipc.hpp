#ifndef IPC_HPP
#define IPC_HPP

#include <cstddef>
#include <cstdint>

namespace zubarev
{
  constexpr size_t NAME_MAX_LEN = 32;
  constexpr size_t PIPE_ENDS = 2;

  struct RequestHeader
  {
    char name[NAME_MAX_LEN];
    uint64_t threads;
    uint64_t tries;
    uint64_t count;
  };

  struct Response
  {
    char name[NAME_MAX_LEN];
    double ms;
    double area;
  };

  size_t send(int& err, int wr, const char* b, size_t k);
  size_t recv(int& err, int rd, char* b, size_t k);

  class Pipe
  {
  public:
    Pipe();
    ~Pipe();

    Pipe(const Pipe&) = delete;
    Pipe& operator=(const Pipe&) = delete;

    Pipe(Pipe&& other) noexcept;
    Pipe& operator=(Pipe&& other) noexcept;

    int readFd() const noexcept;
    int writeFd() const noexcept;
    int getReadFd() const noexcept;
    int gerReadFd() const noexcept;

    int releaseRead() noexcept;
    int releaseWrite() noexcept;

    void closeRead() noexcept;
    void closeWrite() noexcept;
    void closeAll() noexcept;

  private:
    int fds_[PIPE_ENDS];
  };
}

#endif
