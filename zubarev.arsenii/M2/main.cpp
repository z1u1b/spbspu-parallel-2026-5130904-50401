#include <iostream>
#include <limits>
#include <map>
#include <string>
#include "calc_manager.hpp"
#include "commands.hpp"

int main()
{
  namespace zub = zubarev;

  zub::setupSignals();

  using cmd_t = void (*)(std::istream &, std::ostream &, zub::Context &);
  std::map< std::string, cmd_t > cmds;

  cmds["circle"] = zub::cmdCircle;
  cmds["set"] = zub::cmdSet;
  cmds["show"] = zub::cmdShow;
  cmds["showset"] = zub::cmdShowset;
  cmds["frame"] = zub::cmdFrame;
  cmds["frameset"] = zub::cmdFrameset;
  cmds["spawn"] = zub::cmdSpawn;
  cmds["area-on"] = zub::cmdAreaOn;
  cmds["status"] = zub::cmdStatus;
  cmds["wait"] = zub::cmdWait;

  zub::Context ctx;

  std::string cmd;
  while (std::cin >> cmd)
  {
    try
    {
      const std::map< std::string, cmd_t >::const_iterator it = cmds.find(cmd);
      if (it == cmds.end())
      {
        throw std::out_of_range("unknown command");
      }
      it->second(std::cin, std::cout, ctx);
    }
    catch (const std::out_of_range &)
    {
      std::cout << "<UNKNOWN COMMAND>\n";
      const std::streamsize toignore = std::numeric_limits< std::streamsize >::max();
      std::cin.ignore(toignore, '\n');
    }
    catch (const std::logic_error &)
    {
      std::cout << "<INVALID COMMAND>\n";
    }
  }

  zub::cleanup(ctx.calcs);
  return 0;
}
