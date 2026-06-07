#pragma once

#include <vector>
#include <string>
#include <fstream>

inline void write_str(std::ofstream& f, const std::string& s) {
    int len = static_cast<int>(s.size());
    f.write(reinterpret_cast<const char*>(&len), sizeof(len));
    f.write(s.data(), len);
}

inline std::string read_str(std::ifstream& f) {
    int len = 0;
    f.read(reinterpret_cast<char*>(&len), sizeof(len));
    std::string s(len, '\0');
    f.read(s.data(), len);
    return s;
}

bool auto_load();
bool load();

bool try_lock_master();

void auto_save();
void check_reload();
