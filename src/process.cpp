#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid):pid_(pid){}

int Process::Pid() { return pid_; }

float Process::CpuUtilization() {
    long totalTime = LinuxParser::ActiveJiffies(pid_);
  	long seconds = Process::UpTime();
    return (float)totalTime / seconds;
}

string Process::Command() { return LinuxParser::Command(pid_); }

string Process::Ram() { return LinuxParser::Ram(pid_); }

string Process::User() { return LinuxParser::User(pid_); }

long int Process::UpTime() { return LinuxParser::UpTime() - LinuxParser::UpTime(pid_); }

bool Process::operator<(Process const& a) const { return a.cpuUtilization_ < LinuxParser::PidUtilization(pid_)["utilization"]*1.0; }