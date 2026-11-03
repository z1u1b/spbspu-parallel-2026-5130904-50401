#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <iosfwd>
#include <map>
#include <string>
#include <vector>
#include "calc_manager.hpp"
#include "shapes.hpp"

namespace zubarev
{
  struct ShapesTable
  {
    std::map< std::string, Circle > circles;
    std::map< std::string, std::vector< std::string > > sets;
  };

  struct Context
  {
    ShapesTable shapes;
    CalcTable calcs;
  };

  std::vector< std::string > splitLine(const std::string& line);
  bool isValidName(const std::string& name);
  bool isNameOccupied(const ShapesTable& table, const std::string& name);

  void circle(std::istream& in, std::ostream& out, ShapesTable& shapes);
  void set(std::istream& in, std::ostream& out, ShapesTable& shapes);
  void show(std::istream& in, std::ostream& out, ShapesTable& shapes);
  void showset(std::istream& in, std::ostream& out, ShapesTable& shapes);
  void frame(std::istream& in, std::ostream& out, ShapesTable& shapes);
  void frameset(std::istream& in, std::ostream& out, ShapesTable& shapes);

  void cmdCircle(std::istream& in, std::ostream& out, Context& ctx);
  void cmdSet(std::istream& in, std::ostream& out, Context& ctx);
  void cmdShow(std::istream& in, std::ostream& out, Context& ctx);
  void cmdShowset(std::istream& in, std::ostream& out, Context& ctx);
  void cmdFrame(std::istream& in, std::ostream& out, Context& ctx);
  void cmdFrameset(std::istream& in, std::ostream& out, Context& ctx);
  void cmdSpawn(std::istream& in, std::ostream& out, Context& ctx);
  void cmdAreaOn(std::istream& in, std::ostream& out, Context& ctx);
  void cmdStatus(std::istream& in, std::ostream& out, Context& ctx);
  void cmdWait(std::istream& in, std::ostream& out, Context& ctx);
}

#endif
