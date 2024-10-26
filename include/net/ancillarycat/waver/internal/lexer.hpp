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

namespace net::ancillarycat::waver {
class file_reader {
public:
	// inline explicit constexpr file_reader() noexcept = default;
	inline explicit constexpr file_reader(std::filesystem::path filepath) noexcept : filepath(std::move(filepath)) {}

	inline constexpr ~file_reader() noexcept = default;

public:
	[[nodiscard]] inline std::optional<std::string> get_contents() const {
		std::ifstream file(filepath);
		if (not file)
			return std::nullopt;
		std::stringstream ss;
		ss << file.rdbuf();
		return std::make_optional(ss.str());
	}

private:
	const std::filesystem::path filepath;
};

class lexer {
public:
	inline explicit constexpr lexer() = default;

	inline constexpr ~lexer() noexcept = default;

public:
	Status load(const std::filesystem::path &filepath);
	Status load(const std::string &content);
	Status lex();

	[[nodiscard]] std::string_view front() const noexcept;
	[[nodiscard]] std::string_view current() const noexcept;
	[[nodiscard]] std::string_view back() const noexcept;
	[[nodiscard]] bool						 is_empty() const noexcept;

	/// @note get the next token and ADVANCE the cursor
	std::string_view consume();
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
	const auto	maybe_contents = reader.get_contents();
	if (not maybe_contents)
		return NotFoundError("Unable to open file: " + filepath.string());
	contents = *maybe_contents;
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
	return OkStatus();
}
inline std::string_view lexer::front() const noexcept {

	boost::contract::check c = boost::contract::function().precondition([&] { return not token_views.empty(); });

	return token_views.front();
}
inline std::string_view lexer::current() const noexcept {

	boost::contract::check c = boost::contract::function().precondition([&] { return cursor < token_views.size(); });

	return token_views[cursor];
}
inline std::string_view lexer::back() const noexcept {

	boost::contract::check c = boost::contract::function().precondition([&] { return not token_views.empty(); });

	return token_views.back();
}
inline std::string_view lexer::consume() {
	if (cursor >= token_views.size())
		return empty_sv;
	return token_views[cursor++];
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
