#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <algorithm>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::unordered_map;

long readFromFile(const string& path){
  long res;
  std::ifstream stream(path);
  if (stream.is_open()) {
    std::string line;
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> res;
  }
  return res;
}

long readFromFile(const string& path, const string& keyword){
  long res;
  std::ifstream stream(path);
  if (stream.is_open()) {
    std::string line;
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      std::string input;
      linestream >> input;
      if(keyword != ""){
          if(input == keyword){
            linestream >> res;
            break;
          }
      }
    }
  }
  return res;
}

unordered_map<string, long> readFromFile(const string& path, const vector<string>& keywords, const size_t& argc){
  unordered_map<string, long> res;
  
  if(res.size() == argc){
    return res;
  }
  return {};
}

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
  return value;
}

string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  for(const auto& file : std::filesystem::directory_iterator(kProcDirectory.c_str())){
    if(!std::filesystem::is_directory(file)){
      string filename = std::filesystem::path(file).filename();
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  return pids;
}

float LinuxParser::MemoryUtilization() { 
  long memFree, memTotal;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  std::string key;
  long val;
  std::string units;
  if (stream.is_open()) {
    std::string line;
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      
      linestream >> key >> val >> units;
      if(key == "MemTotal:"){
        memTotal = val;
      }
      if(key == "MemFree:"){
        memFree = val;
      }
    }
  }
  return (memTotal - memFree)*1.0/memTotal*1.0;
 }

long LinuxParser::UpTime() { 
  return readFromFile(string(kProcDirectory + kUptimeFilename));
}

long LinuxParser::Jiffies() { return CurrentCpuUtilization()["jiffies"]; }


long LinuxParser::ActiveJiffies(int pid) { return PidUtilization(pid)["activeJiffies"]; }

long LinuxParser::ActiveJiffies() { 
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  long upTime = 0;
  long idleTime;
  if (filestream.is_open()) {
      std::string line;
      std::getline(filestream, line);
      std::istringstream linestream(line);
      linestream >> upTime >> idleTime;
  } 
  return 0; 
 }

long LinuxParser::IdleJiffies() { return CurrentCpuUtilization()["idleJiffies"]; }

unordered_map<string, long> LinuxParser::PidUtilization(int pid) {
  unordered_map<string, long> res;
  std::ifstream stream(kProcDirectory + '/' + to_string(pid) + '/' + kStatFilename);
  if (stream.is_open()) {
      string line, ignore;
      long utime, stime, cutime, cstime, starttime;
      std::getline(stream, line);
      std::istringstream linestream(line);
      for(int i = 0; i < 13; i++) linestream >> ignore;
      linestream >> utime >> stime >> cutime >> cstime ;
      for(int i = 0; i < 4; i++) linestream >> ignore;
      linestream >> starttime;
      
      long activeJiffies = utime + stime + cutime + cstime +starttime;
      res["activeJiffies"] = activeJiffies;
  }
}

unordered_map<string, long> LinuxParser::CurrentCpuUtilization() { 
  unordered_map<string, long> res;
  string cpu, line;
  long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
  }
  long idle_total, nonidle, total;
  idle_total = idle + iowait;
  nonidle = user + nice + system + irq + softirq + steal;
  total = idle_total + nonidle;
  long totalUserTime = user - guest;
  long totalNiceTime = nice - guest_nice;
  long totalIdleTime = idle + iowait;
  long totalSystem = system + irq + softirq;
  long totalVirtualTime = guest + guest_nice;
  long jiffies = totalUserTime + totalNiceTime + totalIdleTime + totalSystem + totalVirtualTime;
  res["idle"] = idle_total;
  res["total"] = total;
  res["jiffies"] = jiffies;
  res["idleJiffies"] = totalIdleTime;
  return res;
 }

float LinuxParser::CpuUtilizationTotal() {
  auto prev = CurrentCpuUtilization();
  sleep(1);
  auto curr = CurrentCpuUtilization();

  long idle_change = curr["idle"] - prev["idle"];
  long total_change = curr["total"] - prev["total"];

  return (total_change-idle_change)*1.0/total_change*1.0;
} 

vector<string> LinuxParser::CpuUtilization() { return {}; }

int LinuxParser::TotalProcesses() { 
  return readFromFile(kProcDirectory + kStatFilename, "processes");
 }

int LinuxParser::RunningProcesses() { 
  return readFromFile(kProcDirectory + kStatFilename, "procs_running");
 }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) { 
  std::stringstream filename;
  filename << kProcDirectory << "/" << pid << "/" << kCmdlineFilename;
  std::ifstream filestream(filename.str());
  std::string cmdLine ;
  if (filestream.is_open()) {
      std::getline(filestream, cmdLine);
  }
  return cmdLine;
 }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid[[maybe_unused]]) { return 0; }

size_t LinuxParser::Cpus() { 
  size_t cpus;
  string cpu, line;
  long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while(std::getline(stream, line)){
      std::istringstream linestream(line);
      linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
      std::cout << cpu;
      if (cpu.find("cpu") != std::string::npos) {
          cpus++;
      } else{
        break;
      }
    }
  }
  return cpus-1;
}