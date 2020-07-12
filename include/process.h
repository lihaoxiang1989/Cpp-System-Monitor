#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(int pid);
  int Pid();
  std::string User();
  std::string Command();
  float CpuUtilization();
  void CpuUtilization(long int activej, long int totalj);
  std::string Ram();                       // TODO: See src/process.cpp
  long int UpTime();                       // TODO: See src/process.cpp
  bool operator<(Process const& a) const;

 private:
  int pid_;
  float cpu_usage_;

};

#endif