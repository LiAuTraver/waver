#define WAVER_DEBUG_ENABLED 1

#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <net/ancillarycat/waver/waver.hpp>
#include <nlohmann/json.hpp>
#include <fmt/core.h>
#include <string>
#include <string_view>



int main(const int argc, const char *const *const argv) {
  std::filesystem::path source_file;
  std::filesystem::path output_file;
  if (argc == 1) {
    // source_file = R"(Z:\Cpp-Playground\waver\test\ALU4.vcd)";
    // output_file = R"(Z:\Cpp-Playground\waver\test\ALU4.json)";
    fmt::println("Waver: unknown command line arguments");
    fmt::println("Usage: waver <source_file> <output_file>");
    fmt::println("Usage: waver <source_file>");
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
  const auto res = net::ancillarycat::waver::value_change_dump::parse(source_file);
  if (not res.ok()) {
    fmt::println("Failed to parse the VCD file: {}", res.status().message().data());
    return EXIT_FAILURE;
  }
  const auto    j = res->as_json();
  std::ofstream output(output_file);
  output << j.dump(4);
  fmt::println("Successfully wrote to {}", output_file.string());
  return 0;
}
