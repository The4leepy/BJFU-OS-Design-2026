#pragma once

#include <vector>
#include <string>
#include <atomic>

inline std::atomic<bool> exit_requested{false};

void dispatch(const std::vector<std::string>&);
void dispatch_direct(const std::vector<std::string>&);