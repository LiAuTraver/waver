// #include "lexer.hpp"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <print>
#include "lexer2.hpp"
int main(int argc, char *argv[]) {
  // net::ancillarycat::waver::vcd_parser parser;
  // std::filesystem::path source_file;
  // std::filesystem::path output_file;
  // if (argc == 1) {
  //   source_file = R"(Z:\Cpp-Playground\waver\test\ALU4.vcd)";
  //   output_file = R"(Z:\Cpp-Playground\waver\test\ALU4.json)";
  // }
  // if (argc == 2) {
  // 	source_file = argv[1];
  // 	output_file = source_file;
  // 	output_file.replace_extension(".json");
  // }
  // if (argc == 3) {
  // 	source_file = argv[1];
  // 	output_file = argv[2];
  // }

  // parser.parse(source_file.string());
  // parser.print_parsed_data();

  // const auto jsonData = parser.to_json();
  // std::ofstream outFile(output_file);
  // outFile << jsonData.dump(2);
  // return 0;


	net::ancillarycat::waver::parser parser("Z:\\Cpp-Playground\\waver\\test\\ALU4.vcd");
	auto err = parser.parse();
	if (err != absl::OkStatus())
		std::println("Error: {}", err.message().data());
	// parser.print_tokens();
	return 0;
}