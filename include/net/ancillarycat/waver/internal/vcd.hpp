#pragma once
#include <absl/strings/string_view.h>
#include <algorithm>
#ifdef WAVER_USE_BOOST_CONTRACT
#include <boost/contract.hpp>
#include <boost/contract/check.hpp>
#include <boost/contract/function.hpp>
#endif
#include <cctype>
#include <charconv>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <nlohmann/json.hpp>
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
#include "config.hpp"
#include "lexer.hpp"
#include "meta_elements.hpp"
#include "vcd_fwd.hpp"

//////////////////////////////////////////////////////////////////////////////
/// 		Declaration
//////////////////////////////////////////////////////////////////////////////
namespace net::ancillarycat::waver {
/// @brief Represents a Value Change Dump (VCD) file
/// @note the VCD file is a standard file format used to simulate digital circuits
class value_change_dump {
  /// @brief Represents the value change dump parser
  /// @note the parser is used to parse the VCD file
  class parser {
  public:
    inline constexpr explicit parser(value_change_dump &vcd) noexcept : vcd(vcd) {}

    inline constexpr  parser(const parser &)        = delete;
    inline constexpr  parser(parser &&rhs) noexcept = delete;
    inline constexpr ~parser() noexcept             = default;

    inline constexpr parser &operator=(const parser &)        = delete;
    inline constexpr parser &operator=(parser &&rhs) noexcept = delete;


  public:
    enum WAVER_NODISCARD parse_error : std::uint8_t;
    using value_type      = value_change_dump;
    using parse_error_t   = parse_error;
    using reference       = value_type &;
    using const_reference = const value_type &;
    using pointer         = value_type *;
    using const_pointer   = const value_type *;
    using string_t        = std::string;
    using string_view_t   = std::string_view;
    using size_type       = std::string::size_type;
    using lexer_t         = lexer</* default template arguments */>;
    template <typename Data>
    using optional_t = std::optional<Data>;
    template <typename Data>
    using expected_t = std::expected<Data, parse_error_t>;

  public:
    WAVER_NODISCARD inline constexpr reference       get() noexcept { return vcd; }
    WAVER_NODISCARD inline constexpr const_reference get() const noexcept { return vcd; }
    WAVER_NODISCARD inline constexpr pointer         data() const noexcept { return &vcd; }
    WAVER_NODISCARD inline constexpr const_pointer   data() noexcept { return &vcd; }
    inline Status load(const std::filesystem::path &filepath) noexcept { return lexer.load(filepath); }
    inline Status load(string_t &&content) noexcept { return lexer.load(std::forward<string_t>(content)); }

  public:
    /// @brief parse the VCD file
    /// @return OkStatus() if successful, various errors otherwise
    inline Status parse();

  private:
    inline parse_error_t        parse_value_changes();
    inline parse_error_t        parse_header();
    inline parse_error_t        parse_version();
    inline parse_error_t        parse_comments();
    inline parse_error_t        parse_timescale();
    inline parse_error_t        parse_scope_fwd(scope *);
    inline parse_error_t        parse_body();
    inline parse_error_t        parse_dumpvars();
    inline parse_error_t        parse_module(scope *);
    inline parse_error_t        parse_variable(const scope *);
    inline expected_t<change_t> parse_change();

  private:
    value_type   &vcd;
    lexer_t       lexer;
    string_view_t token;
  };

public:
  using json_t        = nlohmann::json;
  using parser_t      = net::ancillarycat::waver::value_change_dump::parser;
  using lexer_t       = net::ancillarycat::waver::lexer</* default templat arguments */>;
  using path_t        = std::filesystem::path;
  using string_t      = std::string;
  using string_view_t = std::string_view;
  using expected_t    = StatusOr<value_change_dump>;

public:
  inline explicit constexpr value_change_dump() = default;

  inline constexpr value_change_dump(const value_change_dump &);
  inline           value_change_dump(value_change_dump &&) noexcept;

  inline value_change_dump           &operator=(value_change_dump &&) noexcept;
  inline constexpr value_change_dump &operator=(const value_change_dump &);

  inline constexpr ~value_change_dump() noexcept = default;

public:
  /// @brief convert the value change dump to a json object
  /// @param self this object
  /// @return a json object
  WAVER_NODISCARD inline constexpr json_t as_json(this auto &&self) noexcept { return self; }

public:
  /// @brief parse the VCD file
  /// @param source the path to the file
  /// @return OkStatus() if successful, various errors otherwise
  WAVER_NODISCARD inline static expected_t parse(auto &&source)
    requires std::same_as<std::remove_cvref_t<decltype(source)>, path_t> or
    std::same_as<std::remove_cvref_t<decltype(source)>, string_t>
  {
    auto vcd    = value_change_dump{};
    auto parser = parser_t{vcd};
    if (auto res = parser.load(std::forward<std::remove_cvref_t<decltype(source)>>(source)); res != OkStatus())
      return {res};
    if (auto res = parser.parse(); res != OkStatus())
      return {res};
    return {vcd};
  }

public:
  /// @brief friend function to convert the value change dump to json
  /// @param j the json object
  /// @param vcd the value change dump to serialize
  friend void to_json(json_t &j, const value_change_dump &vcd) {
    to_json(j, vcd.header);
    to_json(j, vcd.dumpvars);
    to_json(j, vcd.value_changes);
  }

public:
  /// @brief Represents the header of the VCD file
  header header;
  /// @brief Represents the dumpvars of the VCD file
  dumpvars dumpvars;
  /// @brief Represents the value changes of the VCD file
  value_changes value_changes;
};
} // namespace net::ancillarycat::waver
//////////////////////////////////////////////////////////////////////////////
///				 Implementation
//////////////////////////////////////////////////////////////////////////////
namespace net::ancillarycat::waver {
inline constexpr value_change_dump::value_change_dump(const value_change_dump &rhs) {
  header        = rhs.header;
  dumpvars      = rhs.dumpvars;
  value_changes = rhs.value_changes;
}
inline value_change_dump::value_change_dump(value_change_dump &&rhs) noexcept {
  header        = std::move(rhs.header);
  dumpvars      = std::move(rhs.dumpvars);
  value_changes = std::move(rhs.value_changes);
}
inline value_change_dump &value_change_dump::operator=(value_change_dump &&rhs) noexcept {
  header        = std::move(rhs.header);
  dumpvars      = std::move(rhs.dumpvars);
  value_changes = std::move(rhs.value_changes);
  return *this;
}
inline constexpr value_change_dump &value_change_dump::operator=(const value_change_dump &rhs) = default;


enum WAVER_NODISCARD value_change_dump::parser::parse_error : std::uint8_t {
  // success
  kSuccess = 0,
  // unrecoverable errors
  kUnexpectedEndOfFile = 1,
  kInvalidScope        = 2,
  kInvalidSignalWidth  = 3,
  kUnknownKeyword      = 4,
  // recoverable errors
  kInvalidSignalType,
  kInvalidTimestamp,
  // unknown error
  kUnknown = std::numeric_limits<std::uint8_t>::max(),
};
inline Status value_change_dump::parser::parse() {
  if (const auto res = lexer.lex(); res != OkStatus())
    return res;
  if (lexer.is_empty())
    return NotFoundError("no tokens to parse");
  // we have a bunch of tokens, now we can parse them
  token = lexer.front();
  if (const auto res = parse_header(); res != parse_error_t::kSuccess)
    return InvalidArgumentError("Failed to parse header" + std::string(token.begin(), token.end()));
  else
    // token was at `$enddefinitions`, so does lexer.current(); call
    // lexer.consume() should also yield `$enddefinitions`
    lexer.consume(2); // token was at `$end` now

  token = lexer.current(); // token should be the first token after `$end`
  // auto _ = parse_body();
  if (const auto res = parse_body(); res != parse_error_t::kSuccess)
    return InvalidArgumentError("Failed to parse body at token" + std::string(token.begin(), token.end()));

  return OkStatus();
}

inline value_change_dump::parser::parse_error_t value_change_dump::parser::parse_dumpvars() {
  WAVER_PRECONDITION(token == keywords::$dumpvars);

  lexer.consume(2); // consume $dumpvars
  for (token = lexer.current(); token != keywords::$end; token = lexer.current()) {
    auto maybe_change = parse_change();
    if (not maybe_change)
      return maybe_change.error();
    vcd.dumpvars.changes.emplace_back(std::move(*maybe_change));
  }
  return kSuccess;
}
inline auto value_change_dump::parser::parse_change()
  -> std::expected<change_t, value_change_dump::parser::parse_error_t> {

  if (token.size() == 2)
    // means it's a one-bit signal
    return change_t{identifier_t{token.back()}, ports_value_t{token.front()}};

  // it's a multi-bit signal
  ports_value_t value = {token.begin(), token.end()};
  if (token = lexer.consume(); token == empty_sv)
    return std::unexpected(parse_error_t::kUnexpectedEndOfFile);
  token                   = lexer.current();
  identifier_t identifier = {token.begin(), token.end()};


  return change_t{identifier, value};
}
inline value_change_dump::parser::parse_error_t value_change_dump::parser::parse_value_changes() {
  for (/*token = lexer.current()*/; token != lexer.back(); token = lexer.current()) {
    if (token.starts_with('#')) {
      timestamp timestamp;
      if (const auto [_, ec] = std::from_chars(token.data() + 1, token.data() + token.size(), timestamp.time);
          ec != std::errc())
        return parse_error_t::kInvalidTimestamp;

      token = lexer.consume(2);
      for (token = lexer.current(); token != lexer.back() && not token.starts_with('#'); token = lexer.current()) {
        auto maybe_change = parse_change();
        if (not maybe_change)
          return maybe_change.error();
        timestamp.changes.insert(std::move(*maybe_change));

        token = lexer.consume();
      }
      vcd.value_changes.timestamps.emplace_back(std::move(timestamp));
    }
  }
  return parse_error_t::kSuccess;
}
inline value_change_dump::parser::parse_error_t value_change_dump::parser::parse_header() {
  WAVER_PRECONDITION(lexer.current() == lexer.front());

  for (/*token = lexer.front()*/; token != keywords::$enddefinitions; token = lexer.current()) {
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
  WAVER_POSTCONDITION(lexer.current() == keywords::$enddefinitions);
  return parse_error_t::kSuccess;
}
inline value_change_dump::parser::parse_error_t value_change_dump::parser::parse_version() {
  WAVER_PRECONDITION(token == keywords::$version);

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
inline value_change_dump::parser::parse_error_t value_change_dump::parser::parse_comments() {
  WAVER_PRECONDITION(token == keywords::$comment);

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
inline value_change_dump::parser::parse_error_t value_change_dump::parser::parse_timescale() {
  WAVER_PRECONDITION(token == keywords::$timescale);

  lexer.consume(); // consume $timescale

  // currently do nothing but consume the token
  token = lexer.consume();
  while (token != lexer.back()) {
    if (token == keywords::$end)
      return parse_error_t::kSuccess; // cursor has passed the `$end`, i.e.,
                                      // now the cursor is the one after the
                                      // `$end`
    token = lexer.consume();
  }
  return parse_error_t::kUnexpectedEndOfFile;
}
inline value_change_dump::parser::parse_error_t
value_change_dump::parser::parse_scope_fwd(scope *parent) { // NOLINT(misc-no-recursion)
  WAVER_PRECONDITION(token == keywords::$scope);

  auto current_scope = std::make_shared<scope>();
  if (parent == nullptr) {
    current_scope = vcd.header.scopes.emplace_back(new scope{});
  }

  lexer.consume(); // consume $scope

  // currently do nothing but consume the token
  token = lexer.current();
  while (token != lexer.back()) {
    if (token == keywords::$upscope) {
      lexer.consume();
      token = lexer.consume();
      if (token = lexer.consume(); token != keywords::$end)
        return parse_error_t::kUnknownKeyword;
      if (parent != nullptr)
        parent->subscopes.emplace_back(std::move(current_scope));
      else
        vcd.header.scopes.emplace_back(std::move(current_scope));
      return parse_error_t::kSuccess;
    }
    if (token == keywords::module) {
      if (auto res = parse_module(current_scope.get()); res != parse_error_t::kSuccess)
        return res;
      if (parent != nullptr)
        parent->subscopes.emplace_back(std::move(current_scope));
      // else // already added in the upper lines
      // 	vcd.header.scopes.emplace_back(std::move(current_scope));
      return parse_error_t::kSuccess;
    }
  }

  return parse_error_t::kUnexpectedEndOfFile;
}
inline value_change_dump::parser::parse_error_t
value_change_dump::parser::parse_module(scope *current_scope) { // NOLINT(misc-no-recursion)
  WAVER_PRECONDITION(token == keywords::module);

  lexer.consume(); // consume `module` keyword
  token = lexer.consume(); // module name
  // auto current_module = std::make_shared<module>();

  if (token != lexer.back())
    current_scope->name = {token.begin(), token.end()};
  else
    return parse_error_t::kUnexpectedEndOfFile;

  token = lexer.consume();
  if (token != keywords::$end)
    return parse_error_t::kInvalidScope;

  current_scope->data = std::make_shared<module>();

  for (token = lexer.current(); token != keywords::$upscope; token = lexer.current()) {
    if (token == keywords::$var) {
      if (auto res = parse_variable(current_scope); res != parse_error_t::kSuccess)
        return res;
    }
    if (token == keywords::$scope) {
      if (auto res = parse_scope_fwd(current_scope); res != parse_error_t::kSuccess)
        return res;
    }
  }
  lexer.consume();
  if (token = lexer.consume(); token != keywords::$end)
    return parse_error_t::kInvalidScope;

  return parse_error_t::kSuccess;
}
inline value_change_dump::parser::parse_error_t value_change_dump::parser::parse_variable(const scope *current_scope) {
  WAVER_PRECONDITION(token == keywords::$var);
  WAVER_PRECONDITION(current_scope->data->get_type() == scope_value_base::scope_type::kModule);

  lexer.consume(); // consume $var

  token            = lexer.consume();
  auto signal_type = port::kUnknown;
  if (token == "wire"sv)
    signal_type = port::kWire;
  else if (token == "reg"sv)
    signal_type = port::kRegistor;
  else
    return parse_error_t::kInvalidSignalType;

  token = lexer.consume();
  size_t signal_width;
  if (const auto [_, ec] = std::from_chars(token.data(), token.data() + token.size(), signal_width); ec != std::errc())
    return parse_error_t::kInvalidSignalWidth;

  token = lexer.consume();
  if (token.size() != 1)
    return parse_error_t::kInvalidSignalWidth;
  identifier_t identifier = {token[0]};

  token            = lexer.consume();
  std::string name = {token.begin(), token.end()};

  // bad implementation, need to be fixed
  std::string reference;
  for (; token != keywords::$end; token = lexer.consume()) {
    // reference += token; // fixme
  }
  std::dynamic_pointer_cast<module>(current_scope->data)
    ->ports.emplace_back(signal_type, signal_width, std::move(identifier), std::move(name), std::move(reference));
  return parse_error_t::kSuccess; /// token should be the one after `$end`
}

inline value_change_dump::parser::parse_error_t value_change_dump::parser::parse_body() {
  for (; token != lexer.back(); token = lexer.current()) {
    if (token.starts_with('#')) {
      // parse value changes
      if (auto res = parse_value_changes(); res != parse_error_t::kSuccess)
        return res;
      else
        (void)res; // temporary do nothing
    } else if (token == keywords::$dumpvars) {
      // parse dumpvars
      if (auto res = parse_dumpvars(); res != parse_error_t::kSuccess)
        return res;
      else
        (void)res;
    }
    // else if (token == keywords::$dumpall) {
    // 	// parse dumpall
    // } else if (token == keywords::$dumpon) {
    // 	// parse dumpon
    // } else if (token == keywords::$dumpoff) {
    // 	// parse dumpoff
    // }
  }

  return parse_error_t::kSuccess;
}

} // namespace net::ancillarycat::waver
