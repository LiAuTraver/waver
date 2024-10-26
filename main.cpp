#include "include/net/ancillarycat/waver/waver.hpp"
#include <filesystem>
#include <fstream>
// #include <nlohmann/json.hpp>
#include <print>

int main(int argc, char *argv[]) {
  net::ancillarycat::waver::parser parser;
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
  auto _ = parser.load(source_file);
  _ = parser.parse();
}