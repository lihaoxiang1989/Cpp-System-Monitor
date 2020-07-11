#include <sstream>
#include <string>

#include "format.h"

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  int hour, min, sec;
  std::stringstream output;
  hour = seconds / 3600;
  min = (seconds % 3600) / 60;
  sec = seconds % 60;
  output << hour << ':' << min << ':' << sec;
  return output.str();
}