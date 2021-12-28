#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <sys/time.h>
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
  for(const auto& file :std::filesystem::directory_iterator(kProcDirectory.c_str())) {
    auto filename = file.path().filename().generic_string();
    if (std::filesystem::is_directory(file.status())) {
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = std::stoi(filename);
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
  return CurrentCpuUtilization()["jiffies"];
 }

float LinuxParser::CpuUtilization(int pid){
  return PidUtilization(pid)["utilization"];
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
      
      long activeJiffies = (utime + stime + cutime + cstime) / sysconf(_SC_CLK_TCK);
      res["activeJiffies"] = activeJiffies;

      long jiffies = LinuxParser::ActiveJiffies();
      res["utilization"] = activeJiffies*1.0 / jiffies;
  }
  return res;
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

int LinuxParser::TotalProcesses() { 
  return readFromFile(kProcDirectory + kStatFilename, "processes");
 }

int LinuxParser::RunningProcesses() { 
  return readFromFile(kProcDirectory + kStatFilename, "procs_running");
 }

string LinuxParser::Command(int pid) { 
  std::ifstream stream(kProcDirectory + "/" + to_string(pid) + "/" + kCmdlineFilename);
  std::string line ;
  if (stream.is_open()) {
    std::getline(stream, line);
  }
  return line;
 }

string LinuxParser::Ram(int pid) { 
  std::ifstream stream(kProcDirectory + "/" + to_string(pid) + "/" + kStatusFilename);
  std::string line, key, units;
  long val,mem;
  if (stream.is_open()) {
    std::getline(stream, line);
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      
      linestream >> key >> val >> units;
      if(key == "VmRSS:"){
        mem = val;
        return to_string(mem/1024);
      }
    }
  }
  return "";
 }

string LinuxParser::Uid(int pid) { 
  std::ifstream stream(kProcDirectory + "/" + to_string(pid) + "/" + kStatusFilename);
  std::string line, key, units;
  long val,mem;
  if (stream.is_open()) {
    std::getline(stream, line);
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      
      linestream >> key >> val;
      if(key == "Uid:"){
        return val;
      }
    }
  }
  return "";
 }

string LinuxParser::User(int pid) { 
  std::ifstream stream(kPasswordPath);
  std::string line, user, pwd, uid, curruid;
  uid = Uid(pid);
  if (stream.is_open()) {
    std::getline(stream, line);
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      
      linestream >> user >> pwd >> curruid;
      if(curruid == uid){
        return user;
      }
    }
  }
  return "";
 }

long LinuxParser::UpTime(int pid) { 
  std::ifstream stream(kProcDirectory + "/" + to_string(pid) + "/" + kStatFilename);
  long starttime = 0;
  if (stream.is_open()) {
      std::string line;
      std::getline(stream, line);
      std::istringstream linestream(line);
      std::string ignore;
      for(int i = 0; i < 21; i++) linestream >> ignore;
      linestream >> starttime;
      return starttime*1.0 / sysconf(_SC_CLK_TCK);
  }
  return starttime; 
 }

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