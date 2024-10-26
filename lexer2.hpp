#pragma once
#include <absl/status/status.h>
#include <algorithm>
#include <boost/contract.hpp>
#include <boost/contract/check.hpp>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <ostream>
#include <print>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace net::ancillarycat::waver {
using absl::AlreadyExistsError;
using absl::InvalidArgumentError;
using absl::NotFoundError;
using absl::OkStatus;
using absl::Status;
using absl::StatusCode;
using namespace std::string_view_literals;

static constexpr inline auto empty_sv = ""sv;
class file_reader {
public:
  inline explicit constexpr file_reader() = default;
  inline explicit constexpr file_reader(
      const std::filesystem::path &filepath) noexcept
      : filepath(filepath) {}
  inline constexpr ~file_reader() noexcept = default;

public:
  std::optional<std::string> get_contents() const {
    std::ifstream file(filepath);
    if (not file)
      return std::nullopt;
    std::stringstream ss;
    ss << file.rdbuf();
    return std::make_optional(ss.str());
  }

private:
  std::filesystem::path filepath;
};
class lexer {
public:
  inline explicit constexpr lexer() = default;
  inline constexpr ~lexer() noexcept = default;

public:
  Status load(const std::filesystem::path &filepath) {
    if (not contents.empty())
      return AlreadyExistsError("File already loaded");
    file_reader reader(filepath);
    const auto maybe_contents = reader.get_contents();
    if (not maybe_contents)
      return NotFoundError("Unable to open file: " + filepath.string());
    contents = *maybe_contents;
    return OkStatus();
  }
  Status load(const std::string &content) {
    if (not contents.empty())
      return AlreadyExistsError("Content already loaded");
    contents = content;
    return OkStatus();
  }
  Status lex() {
    if (contents.empty())
      return NotFoundError("No content to lex");
    std::ranges::replace_if(
        contents, [](auto &&c) { return (c == '\r') || (c == '\n'); }, ' ');
    // clang-format off
		token_views = contents
			| std::ranges::views::split(' ')
			| std::ranges::views::filter([](auto &&token_view) { return not token_view.empty(); })
			| std::ranges::views::transform([](auto &&token_view) { return std::string_view(token_view.begin(), token_view.end()); })
			| std::ranges::to<std::vector<std::string_view>>();
    // clang-format on
    return OkStatus();
  }

  std::string_view front() const noexcept {

    boost::contract::check c = boost::contract::function().precondition(
        [&] { return not token_views.empty(); });

    return token_views.front();
  }

  std::string_view current() const noexcept {

    boost::contract::check c = boost::contract::function().precondition(
        [&] { return cursor < token_views.size(); });

    return token_views[cursor];
  }

  std::string_view back() const noexcept {

    boost::contract::check c = boost::contract::function().precondition(
        [&] { return not token_views.empty(); });

    return token_views.back();
  }

  /// @note get the next token and ADVANCE the cursor
  std::string_view consume() {
    if (cursor >= token_views.size())
      return empty_sv;
    return token_views[cursor++];
  }

  lexer &print_tokens() {
    for (auto &&token : token_views)
      std::println("token: {}",
                   token); // cannot use `token.data()` since std::string_view
                           // is not null-terminated
    return *this;
  }
  bool is_empty() const noexcept { return token_views.empty(); }

private:
  std::string contents;
  size_t cursor = 0;
  /// @note non-owning views
  std::vector<std::string_view> token_views;
};
namespace keywords {
static constinit inline const auto $version = "$version"sv;
static constinit inline const auto $date = "$date"sv;
static constinit inline const auto $timescale = "$timescale"sv;
static constinit inline const auto $scope = "$scope"sv;
static constinit inline const auto $upscope = "$upscope"sv;
static constinit inline const auto $var = "$var"sv;
static constinit inline const auto $enddefinitions = "$enddefinitions"sv;
static constinit inline const auto $comment = "$comment"sv;
static constinit inline const auto $dumpvars = "$dumpvars"sv;
static constinit inline const auto $dumpall = "$dumpall"sv;
static constinit inline const auto $dumpon = "$dumpon"sv;
static constinit inline const auto $dumpoff = "$dumpoff"sv;
static constinit inline const auto $end = "$end"sv;

static constinit inline const auto module = "module"sv;
static constinit inline const auto wire = "wire"sv;
static constinit inline const auto reg = "reg"sv;

} // namespace keywords
using identifier_t = std::string;
using value_t = std::string;
struct port {
  enum type {
    kUnknown = 0,
    kInput = 1,
    kOutput = 2,
    kInout = kInput | kOutput, // 3
    kWire = 4,
    kRegistor = 8,
  };
  type type;
  size_t width;
  identifier_t identifier; // 1 or 2 characters
  std::string name;
  std::string reference; // [4:0], a[0], a[4:0]
};
class scope {
public:
  std::vector<std::shared_ptr<scope>> subscopes;
  inline constexpr std::string_view get_name() const noexcept {
    return get_name_impl();
  }

public:
  inline constexpr scope() = default;
  inline virtual constexpr ~scope() = default;

private:
  [[clang::always_inline]] constexpr virtual std::string_view
  get_name_impl() const noexcept = 0;
  // static constinit inline const auto name = "<unnamed scope>"sv;
};
class module : public scope {
public:
  module &set_name(const std::string_view name) noexcept {
    this->name = name;
    return *this;
  }
  std::vector<port> ports;

public:
  inline constexpr module() = default;
  inline explicit constexpr module(const std::string_view name) noexcept
      : name(name) {}
  inline virtual constexpr ~module() = default;

private:
  virtual std::string_view get_name_impl() const noexcept override {
    return name;
  }
  std::string
      name; // dont use string_view here! name will be `\0ame`, dunno why
};
class task : public scope {};

struct timestamp {
  size_t time;
  std::vector<std::pair<identifier_t, value_t>> changes;
};

struct version {};
struct date {};
struct timescale {};

/// @brief Represents the header part of a VCD file, which contains the module
/// definitions, timescale, and date
struct header {
  std::vector<std::shared_ptr<scope>> scopes;
  version version;
  date date;
  timescale timescale;
};
/// @brief Represents the value change part of a VCD file
struct value_changes {
  std::vector<timestamp> timestamps;
};

/// @brief Represents a Value Change Dump (VCD) file
struct value_change_dump {
  header header;
  value_changes value_changes;
};

class parser {
public:
  inline constexpr explicit parser() noexcept = default;
  inline constexpr ~parser() noexcept = default;

public:
  enum [[nodiscard]] parse_error : std::uint8_t {
    // success
    kSuccess = 0,
    // unrecoverable errors
    kUnexpectedEndOfFile = 1,
    kInvalidScope = 2,
    kInvalidSignalWidth = 3,
    kUnknownKeyword = 4,
    // recoverable errors
    kInvalidSignalType,
    // unknown error
    kUnknown = std::numeric_limits<std::uint8_t>::max(),
  };
  using value_t = value_change_dump;
  using lexer_t = lexer;
  using parse_error_t = parse_error;
  using reference = value_t &;
  using const_reference = const value_t &;
  using pointer = value_t *;
  Status load(const std::filesystem::path &filepath) {
    return lexer.load(filepath);
  }
  Status parse() {
    if (const auto res = lexer.lex(); res != OkStatus())
      return res;
    if (lexer.is_empty())
      return NotFoundError("no tokens to parse");
    // we have a bunch of tokens, now we can parse them
    if (const auto res = parse_header(); res != parse_error_t::kSuccess)
      return InvalidArgumentError("Failed to parse header");
    else // debug breakpoint
      (void)res;
    return OkStatus();
  }

private:
  parse_error_t parse_header() {
    // clang-format off
    boost::contract::check c = boost::contract::function()
            .precondition([&] { return lexer.current() == lexer.front(); })
            .postcondition([&] { return lexer.current() == keywords::$enddefinitions; });
    // clang-format on

    for (token = lexer.front(); token != keywords::$enddefinitions;
         token = lexer.current()) {
      if (token == keywords::$version) {
        if (auto res = parse_version(); res != parse_error_t::kSuccess)
          return res;
        else
          continue;
      }
      if (token == keywords::$comment) {
        if (auto res = parse_comments(); res != parse_error_t::kSuccess)
          return res;
        else
          continue;
      }
      if (token == keywords::$timescale) {
        if (auto res = parse_timescale(); res != parse_error_t::kSuccess)
          return res;
        else
          continue;
      }
      if (token == keywords::$scope) {
        if (auto res = parse_scope_fwd(nullptr); res != parse_error_t::kSuccess)
          return res;
        else
          continue;
      }
      std::println("token: {}", token);
    }
    return parse_error_t::kSuccess;
  }
  parse_error_t parse_version() {
    boost::contract::check c = boost::contract::function().precondition(
        [&] { return lexer.current() == keywords::$version; });

    lexer.consume(); // consume $version

    // currently do nothing but consume the token
    token = lexer.consume();
    while (token != lexer.back())
      if (token == keywords::$end)
        return parse_error_t::kSuccess; // cursor has passed the `$end`, i.e.,
                                        // now the cursor is the one after the
                                        // `$end`
      else
        token = lexer.consume();
    return parse_error_t::kUnexpectedEndOfFile;
  }
  parse_error_t parse_comments() {
    boost::contract::check c = boost::contract::function().precondition(
        [&] { return lexer.current() == keywords::$comment; });

    lexer.consume(); // consume $comment

    // currently do nothing but consume the token
    token = lexer.consume();
    while (token != lexer.back())
      if (token == keywords::$end)
        return parse_error_t::kSuccess; // cursor has passed the `$end`, i.e.,
                                        // now the cursor is the one after the
                                        // `$end`
      else
        token = lexer.consume();
    return parse_error_t::kUnexpectedEndOfFile;
  }

  parse_error_t parse_timescale() {
    boost::contract::check c = boost::contract::function().precondition(
        [&] { return lexer.current() == keywords::$timescale; });

    lexer.consume(); // consume $timescale

    // currently do nothing but consume the token
    token = lexer.consume();
    while (token != lexer.back())
      if (token == keywords::$end)
        return parse_error_t::kSuccess; // cursor has passed the `$end`, i.e.,
                                        // now the cursor is the one after the
                                        // `$end`
      else
        token = lexer.consume();
    return parse_error_t::kUnexpectedEndOfFile;
  }

  parse_error_t parse_scope_fwd(scope *parent) {
    boost::contract::check c = boost::contract::function().precondition(
        [&] { return lexer.current() == keywords::$scope; });

    lexer.consume(); // consume $scope

    // currently do nothing but consume the token
    token = lexer.current();
    while (token != lexer.back()) {
      if (token == keywords::$upscope) {
        if (token = lexer.consume(); token == keywords::$end)
          return parse_error_t::kSuccess;
        return parse_error_t::kUnknownKeyword;
      }
      if (token == keywords::module) {
        auto _ = parse_module(parent);
        return parse_error_t::kSuccess;
      }
    }

    return parse_error_t::kUnexpectedEndOfFile;
  }

  parse_error_t parse_module(scope *parent) {
    boost::contract::check c = boost::contract::function().precondition(
        [&] { return lexer.current() == keywords::module; });

    lexer.consume();         // consume `module` keyword
    token = lexer.consume(); // module name
    auto current_module = std::make_shared<module>();

    if (token != empty_sv)
      current_module->set_name(std::string(token));
    else
      return parse_error_t::kUnexpectedEndOfFile;

    // debug print
    std::println("module name: {}", current_module->get_name());

    std::cout << std::endl;
    token = lexer.consume();
    if (token != keywords::$end)
      return parse_error_t::kInvalidScope;

    for (token = lexer.current(); token != keywords::$upscope;
         token = lexer.current()) {
      if (token == keywords::$var) {
        if (auto res = parse_variable(current_module.get());
            res != parse_error_t::kSuccess)
          return res;
      }
      if (token == keywords::$scope) {
        if (auto res = parse_scope_fwd(current_module.get());
            res != parse_error_t::kSuccess)
          return res;
      }
    }
    // token = `$upscope`
    token = lexer.consume();
		token = lexer.consume();
    if ( token != keywords::$end)
      return parse_error_t::kInvalidScope;

    if (parent)
      parent->subscopes.emplace_back(current_module);
    else // if no parent, then it's the top module
      header.scopes.emplace_back(current_module);

    return parse_error_t::kSuccess;
  }

  parse_error_t parse_variable(module *current_module) {
    boost::contract::check c = boost::contract::function().precondition(
        [&] { return lexer.current() == keywords::$var; });

    lexer.consume(); // consume $var

    token = lexer.consume();
    auto signal_type = port::kUnknown;
    if (token == "wire"sv)
      signal_type = port::kWire;
    else if (token == "reg"sv)
      signal_type = port::kRegistor;
    else
      return parse_error_t::kInvalidSignalType;

    token = lexer.consume();
    size_t signal_width;
    const auto [_, ec] = std::from_chars(
        token.data(), token.data() + token.size(), signal_width);
    if (ec != std::errc())
      return parse_error_t::kInvalidSignalWidth;

    token = lexer.consume();
    if (token.size() != 1)
      return parse_error_t::kInvalidSignalWidth;
    identifier_t identifier = {token[0]};

    token = lexer.consume();
    std::string name = {token.begin(), token.end()};

    // bad implementation, need to be fixed
    std::string reference;
    for (; token != keywords::$end; token = lexer.consume()) {
      reference += token;
    }
    current_module->ports.emplace_back(
        port{signal_type, signal_width, identifier, name, reference});
    return parse_error_t::kSuccess; /// token should be the one after `$end`
  }

private:
  value_t vcd;
  lexer_t lexer;
  header &header = vcd.header;
  value_changes &value_changes = vcd.value_changes;
  std::string_view token;
};
} // namespace net::ancillarycat::waver