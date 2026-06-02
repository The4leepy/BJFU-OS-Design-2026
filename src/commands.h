#pragma once

#include <vector>
#include <string>

inline bool exit_requested = false;

void dispatch(const std::vector<std::string>&);
void dispatch_direct(const std::vector<std::string>&);