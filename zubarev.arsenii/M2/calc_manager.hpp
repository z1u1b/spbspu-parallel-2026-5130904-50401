#ifndef CALC_MANAGER_HPP
#define CALC_MANAGER_HPP

#include <cstdint>
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <sys/types.h>
#include "shapes.hpp"

namespace zubarev
{
  enum class CalcStatus
  {
    IN_PROGRESS,
    COMPLETED,
    CANCELED
  };

  struct ProcessInfo
  {
    pid_t pid = -1;
    int req_wr = -1;
    int resp_rd = -1;
    size_t seed = 0;
    std::string name;
  };

  struct CalcInfo
  {
    std::string calc_name;
    std::string process_name;
    CalcStatus status = CalcStatus::IN_PROGRESS;
    double ms = 0.0;
    double area = 0.0;
  };

  struct CalcTable
  {
    std::map< std::string, ProcessInfo > processes;
    std::set< std::string > used_processes;
    std::map< std::string, CalcInfo > calcs;

    ~CalcTable();
  };

  void setupSignals();

  void spawn(CalcTable& table, const std::string& process_name, size_t seed);
  bool hasProcess(const CalcTable& table, const std::string& process_name);

  void areaOn(CalcTable& table, const std::string& process_name, const std::string& calc_name,
      const std::vector< Circle >& circles, uint64_t threads, uint64_t tries);

  bool hasCalc(const CalcTable& table, const std::string& calc_name);

  void status(CalcTable& table, const std::string& calc_name, std::ostream& out);
  void wait(CalcTable& table, const std::string& calc_name, std::ostream& out);

  void cleanup(CalcTable& table);
}

#endif
