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

string readFromFile(const string& path){
  string res;
  std::ifstream stream(path);
  if (stream.is_open()) {
    std::string line;
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> res;
  }
  return res;
}

string readFromFile(const string& path, const string& keyword){
  string res;
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
  std::ifstream stream(path);
  if (stream.is_open()) {
    std::string line;
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      std::string input;
      linestream >> input;
      //TODO: change logic to extract the keywords
      if (std::find(keywords.begin(), keywords.end(), input) != keywords.end()) {
        // Element in vector.
        long val;
        linestream >> val;
        res[input] = val;
      }
    }
  }
  if(res.size() == argc){
    return res;
  }
  return {};
}

// DONE: An example of how to read data from the filesystem
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

// DONE: An example of how to read data from the filesystem
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

// BONUS: Update this to use std::filesystem
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

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  vector<string> keywords{"MemTotal", "MemFree", "MemAvailable", "Buffers"};
  std::unordered_map<string, long> readings = readFromFile(kProcDirectory + kMeminfoFilename, keywords, 4);
  for(const auto& el: readings){
    
  }
 }

// TODO: Read and return the system uptime
long LinuxParser::UpTime() { 
  string val = readFromFile(string(kProcDirectory + kUptimeFilename));
  return stod(val);
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return 0; }


// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return 0; }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return 0; }

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { return {}; }

int LinuxParser::TotalProcesses() { 
  string val = readFromFile(kProcDirectory + kStatFilename);
  return std::stol(val);
 }

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  string val = readFromFile(kProcDirectory + kStatFilename);
  return std::stol(val);
 }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid[[maybe_unused]]) { return string(); }

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
