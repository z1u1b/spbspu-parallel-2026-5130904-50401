#include "calc_manager.hpp"
#include <cerrno>
#include <csignal>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <poll.h>
#include <stdexcept>
#include <system_error>
#include <sys/wait.h>
#include <unistd.h>
#include "ipc.hpp"
#include "worker.hpp"

namespace
{
  bool isValidIdentifier(const std::string& name)
  {
    if (name.empty())
    {
      return false;
    }
    const unsigned char first = static_cast< unsigned char >(name[0]);
    if (!((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z')))
    {
      return false;
    }
    for (const char ch : name)
    {
      if (std::isspace(static_cast< unsigned char >(ch)))
      {
        return false;
      }
    }
    return true;
  }

  void updateResponses(zubarev::CalcTable& table, int timeout_ms)
  {
    std::vector< struct pollfd > pfds;

    for (std::map< std::string, zubarev::ProcessInfo >::const_iterator it = table.processes.begin();
        it != table.processes.end(); ++it)
    {
      if (it->second.resp_rd != -1)
      {
        struct pollfd pfd{};
        pfd.fd = it->second.resp_rd;
        pfd.events = POLLIN;
        pfds.push_back(pfd);
      }
    }

    if (pfds.empty())
    {
      return;
    }

    const int ret = ::poll(pfds.data(), pfds.size(), timeout_ms);
    if (ret <= 0)
    {
      return;
    }

    for (size_t i = 0; i < pfds.size(); ++i)
    {
      if (pfds[i].revents & POLLIN)
      {
        zubarev::Response resp{};
        int err = 0;
        const size_t rec = zubarev::recv(err, pfds[i].fd, reinterpret_cast< char* >(&resp), sizeof(resp));
        if (rec == sizeof(resp) && err >= 0)
        {
          resp.name[sizeof(resp.name) - 1] = '\0';
          const std::string name(resp.name);
          const std::map< std::string, zubarev::CalcInfo >::iterator it = table.calcs.find(name);
          if (it != table.calcs.end())
          {
            it->second.status = zubarev::CalcStatus::COMPLETED;
            it->second.ms = resp.ms;
            it->second.area = resp.area;
          }
        }
        else if (rec == 0)
        {
          for (std::map< std::string, zubarev::ProcessInfo >::iterator proc = table.processes.begin();
              proc != table.processes.end(); ++proc)
          {
            if (proc->second.resp_rd == pfds[i].fd)
            {
              ::close(proc->second.resp_rd);
              proc->second.resp_rd = -1;
              for (std::map< std::string, zubarev::CalcInfo >::iterator c = table.calcs.begin(); c != table.calcs.end();
                  ++c)
              {
                if (c->second.process_name == proc->first && c->second.status == zubarev::CalcStatus::IN_PROGRESS)
                {
                  c->second.status = zubarev::CalcStatus::CANCELED;
                }
              }
              break;
            }
          }
        }
      }
      else if (pfds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
      {
        for (std::map< std::string, zubarev::ProcessInfo >::iterator proc = table.processes.begin();
            proc != table.processes.end(); ++proc)
        {
          if (proc->second.resp_rd == pfds[i].fd)
          {
            ::close(proc->second.resp_rd);
            proc->second.resp_rd = -1;
            for (std::map< std::string, zubarev::CalcInfo >::iterator c = table.calcs.begin(); c != table.calcs.end();
                ++c)
            {
              if (c->second.process_name == proc->first && c->second.status == zubarev::CalcStatus::IN_PROGRESS)
              {
                c->second.status = zubarev::CalcStatus::CANCELED;
              }
            }
            break;
          }
        }
      }
    }
  }
}

zubarev::CalcTable::~CalcTable()
{
  cleanup(*this);
}

void zubarev::setupSignals()
{
  struct sigaction sa{};
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (::sigaction(SIGPIPE, &sa, nullptr) == -1)
  {
    throw std::system_error(errno, std::generic_category(), "sigaction SIGPIPE failed");
  }
}

void zubarev::cleanup(CalcTable& table)
{
  for (std::map< std::string, ProcessInfo >::iterator it = table.processes.begin(); it != table.processes.end(); ++it)
  {
    if (it->second.req_wr != -1)
    {
      ::close(it->second.req_wr);
      it->second.req_wr = -1;
    }
  }

  for (std::map< std::string, ProcessInfo >::iterator it = table.processes.begin(); it != table.processes.end(); ++it)
  {
    if (it->second.pid > 0)
    {
      int status = 0;
      ::waitpid(it->second.pid, &status, 0);
      it->second.pid = -1;
    }
    if (it->second.resp_rd != -1)
    {
      ::close(it->second.resp_rd);
      it->second.resp_rd = -1;
    }
  }
}

void zubarev::spawn(CalcTable& table, const std::string& process_name, size_t seed)
{
  if (!isValidIdentifier(process_name))
  {
    throw std::logic_error("invalid process name");
  }
  if (table.used_processes.find(process_name) != table.used_processes.end())
  {
    throw std::logic_error("process name already used");
  }

  Pipe req_pipe;
  Pipe resp_pipe;

  const pid_t pid = ::fork();
  if (pid < 0)
  {
    throw std::system_error(errno, std::generic_category(), "fork failed");
  }

  if (pid == 0)
  {
    const int req_fd = req_pipe.readFd();
    const int resp_fd = resp_pipe.writeFd();

    req_pipe.closeWrite();
    resp_pipe.closeRead();

    for (std::map< std::string, ProcessInfo >::const_iterator it = table.processes.begin(); it != table.processes.end();
        ++it)
    {
      if (it->second.req_wr != -1)
      {
        ::close(it->second.req_wr);
      }
      if (it->second.resp_rd != -1)
      {
        ::close(it->second.resp_rd);
      }
    }

    runWorker(req_fd, resp_fd, seed);
    ::_exit(0);
  }

  const int req_wr = req_pipe.releaseWrite();
  const int resp_rd = resp_pipe.releaseRead();

  req_pipe.closeRead();
  resp_pipe.closeWrite();

  ProcessInfo proc;
  proc.pid = pid;
  proc.req_wr = req_wr;
  proc.resp_rd = resp_rd;
  proc.seed = seed;
  proc.name = process_name;

  table.processes[process_name] = proc;
  table.used_processes.insert(process_name);
}

bool zubarev::hasProcess(const CalcTable& table, const std::string& process_name)
{
  return table.processes.find(process_name) != table.processes.end();
}

void zubarev::areaOn(CalcTable& table, const std::string& process_name, const std::string& calc_name,
    const std::vector< Circle >& circles, uint64_t threads, uint64_t tries)
{
  if (!isValidIdentifier(calc_name) || hasCalc(table, calc_name))
  {
    throw std::logic_error("invalid or duplicate calc name");
  }
  const std::map< std::string, ProcessInfo >::iterator proc_it = table.processes.find(process_name);
  if (proc_it == table.processes.end())
  {
    throw std::logic_error("process not found");
  }
  if (tries == 0)
  {
    throw std::logic_error("tries must be positive");
  }

  RequestHeader req{};
  std::strncpy(req.name, calc_name.c_str(), sizeof(req.name) - 1);
  req.name[sizeof(req.name) - 1] = '\0';
  req.threads = threads;
  req.tries = tries;
  req.count = circles.size();

  int err = 0;
  const size_t sent = send(err, proc_it->second.req_wr, reinterpret_cast< const char* >(&req), sizeof(req));
  if (sent < sizeof(req) || err < 0)
  {
    throw std::runtime_error("failed to send request header");
  }

  if (req.count > 0)
  {
    const size_t circles_bytes = req.count * sizeof(Circle);
    const size_t sent_c
        = send(err, proc_it->second.req_wr, reinterpret_cast< const char* >(circles.data()), circles_bytes);
    if (sent_c < circles_bytes || err < 0)
    {
      throw std::runtime_error("failed to send circles data");
    }
  }

  CalcInfo info;
  info.calc_name = calc_name;
  info.process_name = process_name;
  info.status = CalcStatus::IN_PROGRESS;
  info.ms = 0.0;
  info.area = 0.0;

  table.calcs[calc_name] = info;
}

bool zubarev::hasCalc(const CalcTable& table, const std::string& calc_name)
{
  return table.calcs.find(calc_name) != table.calcs.end();
}

void zubarev::status(CalcTable& table, const std::string& calc_name, std::ostream& out)
{
  if (!hasCalc(table, calc_name))
  {
    throw std::logic_error("calc not found");
  }

  updateResponses(table, 0);

  const CalcInfo& c = table.calcs[calc_name];
  if (c.status == CalcStatus::COMPLETED)
  {
    out << c.process_name << " " << std::fixed << std::setprecision(3) << c.ms << " " << c.area << "\n";
  }
  else if (c.status == CalcStatus::CANCELED)
  {
    out << c.process_name << " <CANCELED>\n";
  }
  else
  {
    out << c.process_name << " <IN PROGRESS>\n";
  }
  out << std::flush;
}

void zubarev::wait(CalcTable& table, const std::string& calc_name, std::ostream& out)
{
  if (!hasCalc(table, calc_name))
  {
    throw std::logic_error("calc not found");
  }

  while (table.calcs[calc_name].status == CalcStatus::IN_PROGRESS)
  {
    updateResponses(table, -1);
  }

  status(table, calc_name, out);
}
