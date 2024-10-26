#pragma once
#include "components.hpp"
#include "config.hpp"
#include "lexer.hpp"
#include <algorithm>
#include <boost/contract.hpp>
#include <cctype>
#include <charconv>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <ostream>
#include <print>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace net::ancillarycat::waver {
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
    kInvalidTimestamp,
    // unknown error
    kUnknown = std::numeric_limits<std::uint8_t>::max(),
  };
  using value_type = value_change_dump;
  using lexer_t = lexer;
  using parse_error_t = parse_error;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = value_type *;
  Status load(const std::filesystem::path &filepath) {
    return lexer.load(filepath);
  }
  Status parse() {
    if (const auto res = lexer.lex(); res != OkStatus())
      return res;
    if (lexer.is_empty())
      return NotFoundError("no tokens to parse");
    // we have a bunch of tokens, now we can parse them
    token = lexer.front();
    if (const auto res = parse_header(); res != parse_error_t::kSuccess)
      return InvalidArgumentError("Failed to parse header");
    else
      // token was at `$enddefinitions`, so does lexer.current(); call
      // lexer.consume() should also yield `$enddefinitions`
      lexer.consume(), lexer.consume();

    token = lexer.current();
    if (auto res = parse_value_changes(); res != parse_error_t::kSuccess)
      return InvalidArgumentError("Failed to parse value changes");
    else
      (void)res;

    return OkStatus();
  }

private:
  parse_error_t parse_value_changes() {
    for (/*token = lexer.current()*/; token != lexer.back();
         token = lexer.current()) {
      // if (token == keywords::$dumpvars) {
      // 	if (auto res = parse_dumpvars(); res != parse_error_t::kSuccess)
      // 		return res;
      // 	else
      // 		continue;
      // }
      // if (token == keywords::$dumpall) {
      // 	if (auto res = parse_dumpall(); res != parse_error_t::kSuccess)
      // 		return res;
      // 	else
      // 		continue;
      // }
      // if (token == keywords::$dumpon) {
      // 	if (auto res = parse_dumpon(); res != parse_error_t::kSuccess)
      // 		return res;
      // 	else
      // 		continue;
      // }
      // if (token == keywords::$dumpoff) {
      // 	if (auto res = parse_dumpoff(); res != parse_error_t::kSuccess)
      // 		return res;
      // 	else
      // 		continue;
      // }
      if (token.starts_with('#')) {
        timestamp timestamp;
        const auto [_, ec] = std::from_chars(
            token.data() + 1, token.data() + token.size(), timestamp.time);
        if (ec != std::errc())
          return parse_error_t::kInvalidTimestamp;
        lexer.consume();
        token = lexer.consume();
        for (/*token = lexer.current()*/;
             token != lexer.back() && not token.starts_with('#');
             token = lexer.current()) {
          if (token.size() == 2) {
            // means it's a one-bit signal
            timestamp.changes.emplace_back(timestamp::change{
                identifier_t{token.back()}, timestamp::value_t{token.front()}});
          } else {
            timestamp::value_t value = {token.begin(), token.end()};
            if (token = lexer.consume(); token == empty_sv)
              return parse_error_t::kUnexpectedEndOfFile;
            identifier_t identifier = {token.begin(), token.end()};
            timestamp.changes.emplace_back(
                timestamp::change{identifier, value});
          }
          token = lexer.consume();
        }
        value_changes.timestamps.emplace_back(timestamp);
      }
    }
    return parse_error_t::kSuccess;
  }

  parse_error_t parse_header() {
    // clang-format off
    boost::contract::check c = boost::contract::function()
            .precondition([&] { return lexer.current() == lexer.front(); })
            .postcondition([&] { return lexer.current() == keywords::$enddefinitions; });
    // clang-format on

    for (/*token = lexer.front()*/; token != keywords::$enddefinitions;
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
    if (token != keywords::$end)
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
  value_type vcd;
  lexer_t lexer;
  header &header = vcd.header;
  value_changes &value_changes = vcd.value_changes;
  std::string_view token;
};
} // namespace net::ancillarycat::waver