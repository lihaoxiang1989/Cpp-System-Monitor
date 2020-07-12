#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// Read os-release data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return string();
}

// Read kernel version data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  string key;
  int value, memTotal, memFree;
  string line;
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") {
          memTotal = value;
        } else if (key == "MemFree:") {
          memFree = value;
        }
      }
    }
  }
  return 1.0 * (memTotal - memFree) / memTotal;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  double suspend;
  string line;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> suspend;
    return (long int)(suspend);
  }
  return 0;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    string line, cpu;
    long int user, nice, system, idle, iowait, irq, softirq, steal, guest,
        guest_nice;
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> cpu;
    linestream >> user >> nice >> system >> idle >> iowait;
    linestream >> irq >> softirq >> steal >> guest >> guest_nice;
    idle += iowait;
    long int noIdle = user + nice + system + irq + softirq + steal;
    return idle + noIdle;
  }
  return 0;
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    long int utime, stime, cutime, cstime;
    string stats[52]{};
    string line;
    std::getline(filestream, line);
    std::istringstream linestream(line);
    for (int i = 0; i < 52; i++) {
      linestream >> stats[i];
    }
    utime = stol(stats[13]);
    stime = stol(stats[14]);
    cutime = stol(stats[15]);
    cstime = stol(stats[16]);
    return utime + stime + cutime + cstime;
  }
  return 0;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    string line, cpu;
    long int user, nice, system, idle, iowait, irq, softirq, steal, guest,
        guest_nice;
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> cpu;
    linestream >> user >> nice >> system >> idle >> iowait;
    linestream >> irq >> softirq >> steal >> guest >> guest_nice;
    long int noIdle = user + nice + system + irq + softirq + steal;
    return noIdle;
  }
  return 0;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    string line, cpu;
    long int user, nice, system, idle, iowait;
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> cpu;
    linestream >> user >> nice >> system >> idle >> iowait;
    idle += iowait;
    return idle;
  }
  return 0;
}

// Read CPU utilization
vector<string> LinuxParser::CpuUtilization() { return {}; }

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  std::ifstream filestream(kProcDirectory + kStatFilename);
  string line;
  string key;
  int value;
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          return value;
        }
      }
    }
  }
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  std::ifstream filestream(kProcDirectory + kStatFilename);
  string line;
  string key;
  int value;
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::stringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          return value;
        }
      }
    }
  }
  return 0;
}

// Read the command associated with a process from  /proc/[PID]/stat
string LinuxParser::Command(int pid_) {
  string comm{};
  std::ifstream filestream(kProcDirectory + to_string(pid_) + kStatFilename);
  if (filestream.is_open()) {
    string line;
    std::getline(filestream, line);
    int left = line.find('(');
    int right = line.find(')');
    comm = line.substr(left + 1, right - left - 1);
  }
  return comm;
}

// Read the memory used by a process from /proc/[PID]/status
string LinuxParser::Ram(int pid) {
  string ram;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    string line, key;
    long int value;
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "VmSize:") {
        ram = to_string(value / 1024);
      }
    }
  }
  return ram;
}

// Return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    string line, key, value;
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:") {
          return value;
        }
      }
    }
  }
  return string();
}

// Return the user associated with a process
string LinuxParser::User(int pid) {
  string procUid = Uid(pid);
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    string line, loginName, passwd, uid, gid, username;
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::replace(line.begin(), line.end(), ',', ' ');
      std::istringstream linestream(line);
      while (linestream >> loginName >> passwd >> uid >> gid >> username) {
        if (uid == procUid) {
          return username;
        }
      }
    }
  }
  return string();
}

// Read the uptime of a process from /proc/[PID]/stat
long LinuxParser::UpTime(int pid_) {
  long int uptime;
  std::ifstream filestream(kProcDirectory + to_string(pid_) + kStatFilename);
  if (filestream.is_open()) {
    vector<string> stats;
    string line, temp;
    bool reading{false};
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (linestream >> temp) {
      if (temp[temp.length() - 1] == ')') {
        reading = true;
      } else if (reading) {
        stats.push_back(temp);
      }
    }
    uptime = std::stoi(stats[19]);
  }
  return uptime / sysconf(_SC_CLK_TCK);
}