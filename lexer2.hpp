#pragma once
#include <absl/container/flat_hash_map.h>
#include <absl/status/status.h>
#include <algorithm>
#include <array>
#include <boost/contract.hpp>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <memory>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace net::ancillarycat::waver {
enum class signal_type {
  kUnknown = 0,
  kWire = 1,
  kRegister = 2,
};

struct identifier {
  char name;
};

struct signal {
  signal() = default;
  signal(const signal_type type, const size_t width,
         const identifier &identifier, std::string name) noexcept
      : type(type), width(width), identifier(identifier),
        name(std::move(name)) {}

  signal_type type = signal_type::kUnknown;
  size_t width{};
  identifier identifier{};
  std::string name;
};
struct scope {
	std::string name;
	std::vector<scope> subscopes;
};

struct module: public scope {
  std::vector<signal> signals;
};
struct value_change {
  std::string identifier;
  std::string value;
};
using ssize_t = long long;
struct time_stamp {
  ssize_t time;
  std::vector<value_change> changes;
};

class lexer {

public:
  void lex() {
    std::ifstream file(filepath);
    if (not file)
      return (void)std::println(stderr,
                                "Unable to open file: ", filepath.string());
    std::stringstream ss;
    ss << file.rdbuf();
    contents = ss.str();

    // std::erase_if(contents,
    //               [](auto &&c) { return (c == '\r'); });
    std::ranges::replace_if(
        contents, [](auto &&c) { return (c == '\r') || (c == '\n'); }, ' ');
    // clang-format off
    token_views = contents
    	| std::ranges::views::split(' ')
		| std::ranges::views::filter([](auto&& token_view) { return not token_view.empty(); })
		| std::ranges::views::transform([](auto&& token_view) { return std::string_view(token_view.begin(), token_view.end()); })
    	| std::ranges::to<std::vector<std::string_view>>()
    	;
    // clang-format on
  }
  std::string_view next_token() {
    if (cursor >= token_views.size())
      return {};
    return token_views[cursor++];
  }
  std::string_view peek() {
    if (cursor >= token_views.size())
      return {};
    return token_views[cursor + 1];
  }
  // debug
  void print_tokens() {
    for (auto &&token : token_views)
      std::println("token: {}",
                   token); // cannot use `.data` since it's not null-terminated
  }

  std::string_view begin() const noexcept { return token_views.front(); }

	std::string_view current() const noexcept {
		if (cursor >= token_views.size())
			return {};
		return token_views[cursor];
	}

	std::string_view end() const noexcept { return token_views.back(); }

private:
  std::filesystem::path filepath;
  std::string contents;
  size_t cursor = 0;
  std::vector<std::string_view> token_views;

public:
  inline constexpr explicit lexer(
      const std::filesystem::path &filepath) noexcept
      : filepath(filepath) {}
  inline constexpr explicit lexer(std::filesystem::path &&filepath) noexcept
      : filepath(std::move(filepath)) {}
  inline
      // constexpr
      explicit lexer(const std::string &filepath) noexcept
      : filepath(filepath) {}
  inline
      // constexpr
      explicit lexer(std::string &&filepath) noexcept
      : filepath(filepath) {}
  inline
      // constexpr
      explicit lexer(const char *const filepath) noexcept
      : filepath(filepath) {}
  inline constexpr explicit lexer() noexcept = default;
  inline constexpr ~lexer() noexcept = default;
};

using namespace std::string_view_literals;
constexpr inline auto empty_sv = std::string_view{};

class parser {
public:
  /// $version, $date, $timescale, $scope, $upscope, $var, $enddefinitions,
  /// $comment, $dumpvars, $dumpall, $dumpon, $dumpoff, $dumpall, $dumpoff,
  /// $dumpports, $end
  ///
  absl::Status parse() {
    lexer.lex();
    // we have a bunch of tokens, now we can parse them
    // we can start by parsing the module
    auto token = lexer.begin();
    while (token != "$enddefinitions"sv) {

      if (token == "$version"sv)
        if (const auto parse_version_result = parse_version();
            parse_version_result != absl::OkStatus())
          if (parse_version_result.code() != absl::StatusCode::kAlreadyExists)
            return absl::InvalidArgumentError("Failed to parse version");

      if (token == "$comment"sv)
        if (parse_comments() != absl::OkStatus())
          return absl::InvalidArgumentError("Failed to parse comments");

      if (token == "$timescale"sv)
        if (parse_timescale() != absl::OkStatus())
          return absl::InvalidArgumentError("Failed to parse timescale");

      if (token == "$scope"sv) {
        if (const auto parse_scope_result = parse_scope(top_module);
            parse_scope_result != absl::OkStatus()) {
          if (parse_scope_result.code() != absl::StatusCode::kAlreadyExists)
            return absl::InvalidArgumentError("Failed to parse scope");
          else
            return absl::AlreadyExistsError(
                "Warning: some part of the module has redefinition");
        }
      }
      token = lexer.next_token();
    }
    token = lexer.next_token();
    if (token != "$end"sv)
      return absl::InvalidArgumentError("Unexpected end of file");
    // here the definition ends. We can start parsing the value changes
    std::println("current token: {}", token);
    return absl::OkStatus();
  }

private:
  // really need to implement this
  absl::Status parse_scope(module &parent) {
    auto token = lexer.next_token();

    auto _ = boost::contract::function().precondition(
        [&] { return token == "$scope"sv; });

    token = lexer.peek();

    if (token == "module"sv) {
      if (auto parse_module_result = parse_module(parent);
          // now token should be `$end`
          parse_module_result != absl::OkStatus()) {
        if (parse_module_result.code() != absl::StatusCode::kAlreadyExists)
          return absl::InvalidArgumentError("Failed to parse module");
        return parse_module_result;
      }

      token = lexer.next_token();
    }
    return absl::InvalidArgumentError("Invalid scope");
  }
  absl::Status parse_module(module &parent) {

    module current_module;
    auto token = lexer.next_token(); // module name

    auto _ = boost::contract::function().precondition(
        [&] { return token == "module"sv; });

    token = lexer.next_token();

    // skip a verificaiton of module name...
    current_module.name = {token.begin(), token.end()};
    if (std::addressof(parent) != std::addressof(top_module))
      // if not the top module, add to the parent's submodule
      parent.subscopes.emplace_back(current_module);
    else
      // if it's the top module, assign it to the top module
      top_module = std::move(current_module);

    token = lexer.next_token();
    if (token != "$end"sv)
      return absl::InvalidArgumentError("Unexpected end of file");

    token = lexer.next_token();
    while (token != "$upscope"sv) {
      if (token == "$var"sv) {
        if (parse_variable(current_module) != absl::OkStatus())
          return absl::InvalidArgumentError("Failed to parse variable");
      }
      if (token == "$scope"sv) {
        if (parse_scope(current_module) != absl::OkStatus())
          return absl::InvalidArgumentError("Failed to parse module");
      }
      token = lexer.next_token();
    }

    // token = "$upscope"sv, don't forget the `$end` token
    token = lexer.next_token();
    if (token != "$end"sv)
      return absl::InvalidArgumentError("Unexpected end of file");
    lexer.next_token();
    // done parsing the module
    return absl::OkStatus();
  }
  absl::Status parse_variable(module &current_module) {
    // signal signal;
    // ... a bunch of parsing
    auto token = lexer.next_token();
    signal_type signal_type = signal_type::kUnknown;
    if (token == "wire"sv)
      signal_type = signal_type::kWire;
    else if (token == "reg"sv)
      signal_type = signal_type::kRegister;
    else
      return absl::InvalidArgumentError("Invalid signal type");
    token = lexer.next_token();
    size_t signal_width;
    const auto [_, ec] = std::from_chars(
        token.data(), token.data() + token.size(), signal_width);
    if (ec != std::errc())
      return absl::InvalidArgumentError("Invalid signal width");

    token = lexer.next_token();
    if (token.size() != 1)
      return absl::InvalidArgumentError("Invalid identifier");
    identifier identfier = {token[0]};

    token = lexer.next_token();
    std::string name = {token.begin(), token.end()};

    // can be 1-dimension or 2-dimension or just one bit, currently passing
    while (token != "$end"sv)
      if (token != empty_sv)
        token = lexer.next_token();
      else
        return absl::InvalidArgumentError("Unexpected end of file");

    current_module.signals.emplace_back(signal_type, signal_width, identfier,
                                        name);
    return absl::OkStatus();
  }

  // currently do nothing but consume the token
  /// @todo: implement version parsing
  absl::Status parse_version() {
    auto token = lexer.next_token();

    boost::contract::function().precondition(
        [&] { return token == "$version"sv; });

    static auto current_status = absl::StatusCode::kNotFound;

    token = lexer.next_token();

    while (token != lexer.end())
      if (token == "$end"sv)
        return current_status == absl::StatusCode::kNotFound
               ? current_status = absl::StatusCode::kOk,
                 absl::OkStatus()
               : absl::AlreadyExistsError("Version already parsed");
      else
        token = lexer.next_token();

    return absl::InvalidArgumentError("Unexpected end of file");
  }
  // currently do nothing but consume the token
  /// @todo: implement
  absl::Status parse_comments() {
    auto token = lexer.next_token();

    auto _ = boost::contract::function().precondition(
        [&] { return token == "$comment"sv; });

    token = lexer.next_token();

    while (token != lexer.end()) {
      if (token == "$end"sv)
        return absl::OkStatus();
      else
        token = lexer.next_token();
    }
    return absl::InvalidArgumentError("Unexpected end of file");
  }
  // currently do nothing but consume the token
  /// @todo: implement
  absl::Status parse_timescale() {
    auto token = lexer.next_token();

    auto _ = boost::contract::function().precondition(
        [&] { return token == "$timescale"sv; });

    token = lexer.next_token();
    while (token != lexer.end()) {
      if (token == "$end"sv)
        return absl::OkStatus();
      else
        token = lexer.next_token();
    }
    return absl::InvalidArgumentError("Unexpected end of file");
  }

private:
  lexer lexer;
  module top_module;
  std::vector<time_stamp> time_stamps;

public:
  explicit parser(const std::filesystem::path &filepath) : lexer(filepath) {}
  explicit parser(std::filesystem::path &&filepath)
      : lexer(std::move(filepath)) {}
  explicit parser(const std::string &filepath) : lexer(filepath) {}
  explicit parser(std::string &&filepath) : lexer(std::move(filepath)) {}
  explicit parser(const char *const filepath) : lexer(filepath) {}
  parser() = default;
  ~parser() = default;
};
} // namespace net::ancillarycat::waver