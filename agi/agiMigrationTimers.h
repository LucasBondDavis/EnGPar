#ifndef AGI_MIGRTIMERS
#define AGI_MIGRTIMERS

#include <string>
#include <vector>
#include <map>

namespace agi {
  class MigrationTimers {
    public:
      MigrationTimers();
      ~MigrationTimers();
      /// add a new timer - the selected name must be used in
      /// subsequent api calls exactly as entered here
      void addTimer(std::string name);
      /// add val and incriment the call count for the timer 'name'
      void update(std::string name, double val);
      /// get the maximum accumulated time across all processes: max(time)
      double processMax(std::string name);
      /// get the maximum per call time across all processes: max(time/numCalls)
      double perCallProcessMax(std::string name);
      /// get the average accumulated time across all processes: sum(time)/NP
      double processAvg(std::string name);
      /// get the average per call time across all processes: sum(time/numCalls)/NP
      double perCallProcessAvg(std::string name);

    private:
      std::vector<double> times;
      std::vector<int> counts;
      std::vector<std::string> names;
      std::map<std::string,int> nameToIdx;
      /// -1 on failure, >= 0 on success
      int getTimerIdx(std::string);
  };
}

#endif
