#include <string>

#include "format.h"

using std::string;

string Format::ElapsedTime(long seconds) { 
    string h = std::to_string(seconds / 3600);
    string m = std::to_string((seconds%=3600) / 60);
    string s = std::to_string((seconds%=3600)%=60);
    string hh = string(2 - h.length(), '0') + h;
    string mm = string(2 - m.length(), '0') + m;
    string ss = string(2 - s.length(), '0') + s;
    return hh+':'+mm+':'+ss;
}