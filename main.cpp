#define WAVER_DEBUG_ENABLED 1

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <print>
#include "include/net/ancillarycat/waver/waver.hpp"


int main(const int argc, const char *const *const argv) {
  std::filesystem::path source_file;
  std::filesystem::path output_file;
  if (argc == 1) {
    source_file = R"(Z:\Cpp-Playground\waver\test\ALU4.vcd)";
    output_file = R"(Z:\Cpp-Playground\waver\test\ALU4.json)";
  }
  if (argc == 2) {
    source_file = argv[1];
    output_file = source_file;
    output_file.replace_extension(".json");
  }
  if (argc == 3) {
    source_file = argv[1];
    output_file = argv[2];
  }
  auto res = net::ancillarycat::waver::value_change_dump::parse(source_file)
               .value_or(net::ancillarycat::waver::value_change_dump{});

  auto j = res.as_json();

  std::ofstream output(output_file);
  output << j.dump(4);
  return 0;
}
