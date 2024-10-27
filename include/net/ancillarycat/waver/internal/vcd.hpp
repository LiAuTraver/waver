#pragma once
#include <absl/strings/string_view.h>
#include <algorithm>
#include <boost/contract.hpp>
#include <boost/contract/check.hpp>
#include <boost/contract/function.hpp>
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
#include "vcd.hpp"
#include "vcdfwd.hpp"
namespace net::ancillarycat::waver {
struct port {
	enum type {
		kUnknown	= 0,
		kInput		= 1,
		kOutput		= 2,
		kInout		= kInput | kOutput, // 3
		kWire			= 4,
		kRegistor = 8,
	};
	type				 type;
	size_t			 width;
	identifier_t identifier; // 1 or 2 characters
	std::string	 name;
	std::string	 reference; // [4:0], a[0], a[4:0]
};
class scope {
public:
	std::vector<std::shared_ptr<scope>> subscopes{};
	inline constexpr std::string_view		get_name() const noexcept { return get_name_impl(); }

public:
	inline constexpr					scope() = default;
	inline virtual constexpr ~scope() = default;

private:
	[[clang::always_inline]] constexpr virtual std::string_view get_name_impl() const noexcept = 0;
};
class module : public scope {
public:
	module &set_name(const std::string_view name) noexcept {
		this->name = name;
		return *this;
	}
	std::vector<port> ports;

public:
	inline constexpr					module() = default;
	inline explicit constexpr module(const std::string_view name) noexcept : name(name) {}
	inline virtual constexpr ~module() override = default;

private:
	virtual std::string_view get_name_impl() const noexcept override { return name; }
	std::string							 name; // dont use string_view here! name will be `\0ame`, dunno why
};
class task : public scope {};

struct timestamp {
	time_t								time;
	std::vector<change_t> changes;
};

struct version {};
struct date {};
struct timescale {};

/// @brief Represents the header part of a VCD file, which contains the module
/// definitions, timescale, and date
struct header {
	std::vector<std::shared_ptr<scope>> scopes;
	version															version;
	date																date;
	timescale														timescale;
};
/// @brief Represents the value change part of a VCD file
struct value_changes {
	std::vector<timestamp> timestamps;
};

/// @brief initial value of ports
struct dumpvars {
	std::vector<change_t> changes;
};


/// @brief Represents a Value Change Dump (VCD) file
class value_change_dump {
	class parser {
	public:
		inline constexpr explicit parser(value_change_dump &vcd) noexcept : vcd(vcd) {}

		inline constexpr	parser(const parser &)				= delete;
		inline constexpr	parser(parser &&rhs) noexcept = delete;
		inline constexpr ~parser() noexcept							= default;

		inline constexpr parser &operator=(const parser &)				= delete;
		inline constexpr parser &operator=(parser &&rhs) noexcept = delete;


	public:
		enum AC_NODISCARD parse_error : std::uint8_t;
		using value_type			= value_change_dump;
		using lexer_t					= lexer;
		using parse_error_t		= parse_error;
		using reference				= value_type &;
		using const_reference = const value_type &;
		using pointer					= value_type *;
		using const_pointer		= const value_type *;
		using size_type				= lexer_t::size_type;
		using string_t				= std::string;

	public:
		AC_NODISCARD inline constexpr reference				get() noexcept { return vcd; }
		AC_NODISCARD inline constexpr const_reference get() const noexcept { return vcd; }
		AC_NODISCARD inline constexpr pointer					data() const noexcept { return &vcd; }
		AC_NODISCARD inline constexpr const_pointer		data() noexcept { return &vcd; }
		inline Status load(const std::filesystem::path &filepath) noexcept { return lexer.load(filepath); }
		inline Status load(string_t &&content) noexcept { return lexer.load(std::forward<string_t>(content)); }

	public:
		Status parse();

	private:
		parse_error_t parse_value_changes();
		parse_error_t parse_header();
		parse_error_t parse_version();
		parse_error_t parse_comments();
		parse_error_t parse_timescale();
		parse_error_t parse_scope_fwd(scope *);
		parse_error_t parse_module(scope *);
		parse_error_t parse_variable(module *);
		parse_error_t parse_body();
		parse_error_t parse_dumpvars();
		auto					parse_change() -> std::expected<change_t, parse_error_t>;

	private:
		value_type			&vcd;
		lexer_t					 lexer;
		std::string_view token;
	};

public:
	using json_t				= nlohmann::json;
	using parser_t			= net::ancillarycat::waver::value_change_dump::parser;
	using lexer_t				= net::ancillarycat::waver::lexer;
	using path_t				= std::filesystem::path;
	using string_t			= std::string;
	using string_view_t = std::string_view;
	using expected_t		= StatusOr<value_change_dump>;

public:
	inline explicit constexpr value_change_dump() = default;

	inline constexpr value_change_dump(const value_change_dump &);
	inline constexpr value_change_dump(value_change_dump &&) noexcept;

	inline constexpr value_change_dump &operator=(value_change_dump &&) noexcept;
	inline constexpr value_change_dump &operator=(const value_change_dump &);

	inline constexpr ~value_change_dump() noexcept = default;

public:
	inline static expected_t parse(auto &&source)
		requires std::same_as<std::remove_cvref_t<decltype(source)>, path_t> or
						 std::same_as<std::remove_cvref_t<decltype(source)>, string_t>
	{
		auto vcd		= value_change_dump{};
		auto parser = parser_t{vcd};
		if (auto res = parser.load(std::forward<std::remove_cvref_t<decltype(source)>>(source)); res != OkStatus())
			return {res};
		if (auto res = parser.parse(); res != OkStatus())
			return {res};
		return vcd;
	}
	inline json_t to_json() const {}

public:
	header				header;
	dumpvars			dumpvars;
	value_changes value_changes;
};
//////////////////////////////////////////////////////////////////////////////
///				 Implementation
//////////////////////////////////////////////////////////////////////////////
} // namespace net::ancillarycat::waver
namespace net::ancillarycat::waver {
constexpr value_change_dump::value_change_dump(const value_change_dump &rhs) {
	header				= rhs.header;
	dumpvars			= rhs.dumpvars;
	value_changes = rhs.value_changes;
}
constexpr value_change_dump::value_change_dump(value_change_dump &&rhs) noexcept {
	header				= std::move(rhs.header);
	dumpvars			= std::move(rhs.dumpvars);
	value_changes = std::move(rhs.value_changes);
}
constexpr value_change_dump &value_change_dump::operator=(value_change_dump &&rhs) noexcept {
	header				= std::move(rhs.header);
	dumpvars			= std::move(rhs.dumpvars);
	value_changes = std::move(rhs.value_changes);
	return *this;
}
constexpr value_change_dump &value_change_dump::operator=(const value_change_dump &rhs) = default;


enum AC_NODISCARD value_change_dump::parser::parse_error : std::uint8_t {
	// success
	kSuccess = 0,
	// unrecoverable errors
	kUnexpectedEndOfFile = 1,
	kInvalidScope				 = 2,
	kInvalidSignalWidth	 = 3,
	kUnknownKeyword			 = 4,
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
	boost::contract::check c = boost::contract::function().precondition([&]() { return token == keywords::$dumpvars; });

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
				timestamp.changes.emplace_back(std::move(*maybe_change));

				token = lexer.consume();
			}
			vcd.value_changes.timestamps.emplace_back(std::move(timestamp));
		}
	}
	return parse_error_t::kSuccess;
}
inline value_change_dump::parser::parse_error_t value_change_dump::parser::parse_header() {
	// clang-format off
    boost::contract::check c = boost::contract::function()
            .precondition([&] { return lexer.current() == lexer.front(); })
            .postcondition([&] { return lexer.current() == keywords::$enddefinitions; });
	// clang-format on

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
	return parse_error_t::kSuccess;
}
inline value_change_dump::parser::parse_error_t value_change_dump::parser::parse_version() {
	boost::contract::check c =
		boost::contract::function().precondition([&] { return lexer.current() == keywords::$version; });

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
	boost::contract::check c =
		boost::contract::function().precondition([&] { return lexer.current() == keywords::$comment; });

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
	boost::contract::check c =
		boost::contract::function().precondition([&] { return lexer.current() == keywords::$timescale; });

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
inline value_change_dump::parser::parse_error_t
value_change_dump::parser::parse_scope_fwd(scope *parent) { // NOLINT(misc-no-recursion)
	boost::contract::check c =
		boost::contract::function().precondition([&] { return lexer.current() == keywords::$scope; });

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
			if (auto res = parse_module(parent); res != parse_error_t::kSuccess)
				return res;
			return parse_error_t::kSuccess;
		}
	}

	return parse_error_t::kUnexpectedEndOfFile;
}
inline value_change_dump::parser::parse_error_t
value_change_dump::parser::parse_module(scope *parent) { // NOLINT(misc-no-recursion)
	boost::contract::check c =
		boost::contract::function().precondition([&] { return lexer.current() == keywords::module; });

	lexer.consume(); // consume `module` keyword
	token								= lexer.consume(); // module name
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

	for (token = lexer.current(); token != keywords::$upscope; token = lexer.current()) {
		if (token == keywords::$var) {
			if (auto res = parse_variable(current_module.get()); res != parse_error_t::kSuccess)
				return res;
		}
		if (token == keywords::$scope) {
			if (auto res = parse_scope_fwd(current_module.get()); res != parse_error_t::kSuccess)
				return res;
		}
	}
	// token = `$upscope`
	lexer.consume();
	token = lexer.consume();
	if (token != keywords::$end)
		return parse_error_t::kInvalidScope;

	if (parent)
		parent->subscopes.emplace_back(current_module);
	else // if no parent, then it's the top module
		vcd.header.scopes.emplace_back(current_module);

	return parse_error_t::kSuccess;
}
inline value_change_dump::parser::parse_error_t value_change_dump::parser::parse_variable(module *current_module) {
	boost::contract::check c =
		boost::contract::function().precondition([&] { return lexer.current() == keywords::$var; });

	lexer.consume(); // consume $var

	token						 = lexer.consume();
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

	token						 = lexer.consume();
	std::string name = {token.begin(), token.end()};

	// bad implementation, need to be fixed
	std::string reference;
	for (; token != keywords::$end; token = lexer.consume()) {
		reference += token;
	}
	current_module->ports.emplace_back(port{signal_type, signal_width, identifier, name, reference});
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
		} else if (token == keywords::$dumpall) {
			// parse dumpall
		} else if (token == keywords::$dumpon) {
			// parse dumpon
		} else if (token == keywords::$dumpoff) {
			// parse dumpoff
		}
	}

	return parse_error_t::kSuccess;
}

} // namespace net::ancillarycat::waver