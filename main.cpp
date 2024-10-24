#include "lexer.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
int main() {
  net::ancillarycat::waver::Parser parser;
  parser.parse(R"(Z:\Cpp-Playground\waver\test\ALU4.vcd)");
  parser.print_parsed_data();

  nlohmann::json jsonData = parser.to_json();
  std::ofstream outFile(R"(Z:\Cpp-Playground\waver\test\ALU4.json)");
  outFile << jsonData.dump(4);
  outFile.close();

  return 0;
}