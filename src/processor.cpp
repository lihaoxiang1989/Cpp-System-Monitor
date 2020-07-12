#include "processor.h"
#include "linux_parser.h"

// Return the aggregate CPU utilization
float Processor::Utilization() {
  return 1.0 * LinuxParser::ActiveJiffies() / LinuxParser::Jiffies();
}