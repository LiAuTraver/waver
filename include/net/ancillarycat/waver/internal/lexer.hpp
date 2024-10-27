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
#include "components.hpp"
#include "config.hpp"

namespace net::ancillarycat::waver {
class file_reader {
public:
	// inline explicit constexpr file_reader() noexcept = default;
	inline explicit constexpr file_reader(std::filesystem::path filepath) noexcept : filepath(std::move(filepath)) {}

	inline constexpr ~file_reader() noexcept = default;

public:
	AC_NODISCARD inline std::string get_contents() const {
		std::ifstream file(filepath);
		if (not file)
			return std::string{};
		std::stringstream ss;
		ss << file.rdbuf();
		return ss.str();
	}

private:
	const std::filesystem::path filepath;
};

class lexer {
public:
	using size_type = std::string::size_type;
public:
	inline explicit constexpr lexer() = default;

	inline constexpr ~lexer() noexcept = default;

public:
	Status load(const std::filesystem::path &filepath);
	Status load(const std::string &content);
	Status lex();

	AC_NODISCARD std::string_view front() const noexcept;
	AC_NODISCARD std::string_view current() const /*noexcept*/;
	AC_NODISCARD std::string_view back() const noexcept;
	AC_NODISCARD bool							is_empty() const noexcept;

	/// @note get the next token and ADVANCE the cursor
	std::string_view consume(size_type step = 1);
	lexer						&print_tokens();

private:
	std::string contents;
	size_t			cursor = 0;
	/// @note non-owning views
	std::vector<std::string_view> token_views;
};

inline Status lexer::load(const std::filesystem::path &filepath) {
	if (not contents.empty())
		return AlreadyExistsError("File already loaded");
	file_reader reader(filepath);
	contents = reader.get_contents();
	if (contents.empty())
		return NotFoundError("Unable to open file: " + filepath.string());
	return OkStatus();
}
inline Status lexer::load(const std::string &content) {
	if (not contents.empty())
		return AlreadyExistsError("Content already loaded");
	contents = content;
	return OkStatus();
}
inline Status lexer::lex() {
	if (contents.empty())
		return NotFoundError("No content to lex");
	std::ranges::replace_if(contents, [](auto &&c) { return (c == '\r') || (c == '\n'); }, ' ');
	// clang-format off
		token_views = contents
			| std::ranges::views::split(' ')
			| std::ranges::views::filter([](auto &&token_view) { return not token_view.empty(); })
			| std::ranges::views::transform([](auto &&token_view) { return std::string_view(token_view.begin(), token_view.end()); })
			| std::ranges::to<std::vector<std::string_view>>();
	// clang-format on
	token_views.emplace_back(empty_sv);
	return OkStatus();
}
inline std::string_view lexer::front() const noexcept {

	boost::contract::check c = boost::contract::function().precondition([&] { return not token_views.empty(); });

	return token_views.front();
}
inline std::string_view lexer::current() const /*noexcept*/ {

	boost::contract::check c = boost::contract::function().precondition([&] { return cursor < token_views.size(); });

	return token_views[cursor];
}
inline std::string_view lexer::back() const noexcept {

	boost::contract::check c = boost::contract::function().precondition([&] { return not token_views.empty(); });

	return token_views.back();
}
inline std::string_view lexer::consume(const lexer::size_type step) {
	boost::contract::check c = boost::contract::function().precondition([&] { return cursor < token_views.size(); });

	auto token = token_views[cursor];
	cursor += step;
	return token;
}
inline lexer &lexer::print_tokens() {
	for (auto &&token : token_views)
		std::println("token: {}",
								 token); // cannot use `token.data()` since std::string_view
												 // is not null-terminated
	return *this;
}
inline bool lexer::is_empty() const noexcept { return token_views.empty(); }
} // namespace net::ancillarycat::waver
