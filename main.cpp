// #include "lexer.hpp"
#include "lexer2.hpp"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <print>

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

  // net::ancillarycat::waver::parser
  // parser("Z:\\Cpp-Playground\\waver\\test\\ALU4.vcd"); auto err =
  // parser.parse(); if (err != absl::OkStatus()) 	std::println("Error: {}",
  // err.message().data());
  // // parser.print_tokens();
  // return 0;

  // net::ancillarycat::waver::lexer lexer;
  // auto _ = lexer.load(
  //     std::filesystem::path("Z:\\Cpp-Playground\\waver\\test\\ALU4.vcd"));
  // _ = lexer.lex();
	// (void)_;
  // lexer.print_tokens();
	// net::ancillarycat::waver::module top_module;
	// return 0;
	net::ancillarycat::waver::parser parser;
	parser.load(std::filesystem::path("Z:\\Cpp-Playground\\waver\\test\\ALU4.vcd"));
	parser.parse();
	// using namespace std::string_literals;
	// auto s = "hello,world!"s;
	// auto sv = std::string_view(s.begin(), s.begin() + 5);
	// std::println("{}", sv);
	// auto s2 = std::string(sv.begin(), sv.end());
	// std::println("{}", s2);
	return 0;
}