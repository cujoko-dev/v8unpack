#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <system_error>

// Temporary compatibility layer while the legacy implementation is split
// into smaller modules. It maps the old Boost.Filesystem spellings to the
// C++17 standard library without retaining a Boost dependency.
namespace boost {
namespace filesystem {
using path = std::filesystem::path;
using ifstream = std::ifstream;
using ofstream = std::ofstream;
using directory_iterator = std::filesystem::directory_iterator;

using std::filesystem::absolute;
using std::filesystem::create_directories;
using std::filesystem::exists;
using std::filesystem::file_size;
using std::filesystem::is_directory;
using std::filesystem::remove;
using std::filesystem::rename;
using std::filesystem::temp_directory_path;

inline path unique_path()
{
    static std::atomic<unsigned long long> counter{0};
    const auto ticks = std::chrono::high_resolution_clock::now()
        .time_since_epoch().count();
    return "v8unpack-" + std::to_string(ticks) + "-"
        + std::to_string(counter.fetch_add(1)) + ".tmp";
}
}
}
