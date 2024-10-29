#pragma once
#include <absl/status/status.h>
#include <algorithm>
#include <boost/contract.hpp>
#include <boost/contract/check.hpp>
#include <cctype>
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
#include <utility>
#include <vector>
#include "config.hpp"
#include "vcd_fwd.hpp"

namespace net::ancillarycat::waver {
template <typename PathType = std::filesystem::path, typename StringType = std::string,
					typename InputStreamType = std::ifstream, typename OutputStringStreamType = std::ostringstream>
class file_reader {
public:
	using path_t					= PathType;
	using string_t				= StringType;
	using ifstream_t			= InputStreamType;
	using ostringstream_t = OutputStringStreamType;

public:
	inline explicit constexpr file_reader(path_t filepath) noexcept : filepath(std::move(filepath)) {}

	inline constexpr ~file_reader() noexcept = default;

public:
	WAVER_NODISCARD inline string_t get_contents() const {
		ifstream_t file(filepath);
		if (not file)
			return std::string{};
		ostringstream_t ss;
		ss << file.rdbuf();
		return ss.str();
	}

	WAVER_NODISCARD inline path_t path() const noexcept { return filepath; }

private:
	const path_t filepath;
};


template <typename StringType = std::string, typename StringViewType = std::string_view,
					typename PathType = std::filesystem::path, typename BooleanType = bool, typename StatusType = absl::Status>
class lexer {
public:
	using size_type			= typename StringType::size_type;
	using string_t			= StringType;
	using string_view_t = StringViewType;
	using path_t				= PathType;
	using boolean_t			= BooleanType;
	using status_t			= StatusType;
	using token_views_t = std::vector<string_view_t>;

public:
	inline explicit constexpr lexer() = default;

	inline constexpr lexer(const lexer &other)		 = delete;
	inline constexpr lexer(lexer &&other) noexcept = delete;

	inline constexpr lexer &operator=(const lexer &other)			= delete;
	inline constexpr lexer &operator=(lexer &&other) noexcept = delete;

	inline constexpr ~lexer() noexcept = default;

public:
	inline status_t load(const path_t &filepath) {
		if (not contents.empty())
			return AlreadyExistsError("File already loaded");
		file_reader reader(filepath);
		contents = reader.get_contents();
		if (contents.empty())
			return NotFoundError("Unable to open file: " + filepath.string());
		return OkStatus();
	}
	inline status_t load(string_t &&content) {
		if (not contents.empty())
			return AlreadyExistsError("Content already loaded");
		contents = std::move(content);
		return OkStatus();
	}
	inline status_t lex() {
		if (contents.empty())
			return NotFoundError("No content to lex");
		std::ranges::replace_if(contents, [](auto &&c) { return (c == '\r') || (c == '\n'); }, ' ');
		// clang-format off
		token_views = contents
			| std::ranges::views::split(' ')
			| std::ranges::views::filter([](auto &&token_view) { return not token_view.empty(); })
			| std::ranges::views::transform([](auto &&token_view) { return string_view_t(token_view.begin(), token_view.end()); }) // NOLINT(bugprone-dangling-handle)
			| std::ranges::to<token_views_t>();
		// clang-format on
		token_views.emplace_back(empty_sv);
		return OkStatus();
	}

	WAVER_NODISCARD inline string_view_t front() const noexcept {
		boost::contract::check c = boost::contract::function().precondition([&] { return not token_views.empty(); });

		return token_views.front();
	}
	WAVER_NODISCARD inline string_view_t current() const /*noexcept*/ {
		return cursor < token_views.size() ? token_views[cursor] : token_views.back();
	}
	WAVER_NODISCARD inline string_view_t back() const noexcept {

		boost::contract::check c = boost::contract::function().precondition([&] { return not token_views.empty(); });

		return token_views.back();
	}
	WAVER_NODISCARD inline boolean_t is_empty() const noexcept { return token_views.empty(); }

	///	@brief consume the current token
	///	@param [in] step the number of tokens to skip
	///	@return the current token
	/// @todo I admit this is a poor api design, but I've already used it in the parser, so I'm keeping it now.
	///	@note get the next token and ADVANCE the cursor;
	///				consume() will ALWAYS return the current token, no matter how big the step is;
	///				the step is the short hand for discarding the current token;
	///				if the return value was intended to be used, DONT use the step parameter
	inline string_view_t consume(size_type step = 1) {
		boost::contract::check c = boost::contract::function().precondition([&] { return cursor < token_views.size(); });

		auto token = token_views[cursor];
		cursor += step;
		return token;
	}
	inline lexer &print_tokens() {
		for (auto &&token : token_views)
			std::println("token: {}", token); // cannot use `token.data()` since string_view_t
		// is not null-terminated
		return *this;
	}

private:
	size_type cursor = 0;
	string_t	contents;
	/// @note non-owning views
	token_views_t token_views;
};
} // namespace net::ancillarycat::waver
