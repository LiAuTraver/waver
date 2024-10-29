#pragma once
#include <csignal>
#include <cstdio>
#include "variadic.h"
#include <iostream>
#include <print>

#ifdef __clang__
#define WAVER_FORCEINLINE [[clang::always_inline]]
#define WAVER_DEBUG_BREAK __builtin_debugtrap();
#define WAVER_FUNCTION_NAME __PRETTY_FUNCTION__
#elifdef __GNUC__
#define WAVER_FORCEINLINE [[gnu::always_inline]]
#define WAVER_DEBUG_BREAK __builtin_trap();
#define WAVER_FUNCTION_NAME __PRETTY_FUNCTION__
#elifdef _MSC_VER
#define WAVER_FORCEINLINE [[msvc::forceinline]]
#define WAVER_DEBUG_BREAK __debugbreak();
#define WAVER_FUNCTION_NAME __FUNCSIG__
#else
#define WAVER_FORCEINLINE inline
#define WAVER_DEBUG_BREAK raise(SIGTRAP);
#define WAVER_FUNCTION_NAME __func__
#endif
#define WAVER_AMBIGUOUS_ELSE_BLOCKER                                                                                   \
	switch (0)                                                                                                           \
	case 0:                                                                                                              \
	default:

#define WAVER_RUNTIME_DEBUG_RAISE WAVER_DEBUG_BREAK
#define WAVER_PRINT_ERROR_MSG_IMPL_SINGLE(x)                                                                           \
	std::println(stderr,                                                                                                 \
							 "in file \"{}\" function \'{}\' line {}:\nConstraints not satisfied:\n\tExpected the value of \'{}\' "  \
							 "to be true.\n",                                                                                        \
							 __FILE__, WAVER_FUNCTION_NAME, __LINE__, #x);
#define WAVER_PRINT_ERROR_MSG_IMPL_BINARY(x, y)                                                                        \
	std::println(stderr,                                                                                                 \
							 "in file \"{}\" function \'{}\' line {}:\nConstraints not satisfied:\n\tExpected: {} equals to "        \
							 "{};\nbut actually {} appears to be {},\n         and {} appears to be {}.\n",                          \
							 __FILE__, WAVER_FUNCTION_NAME, __LINE__, #x, #y, #x, x, #y, y);
#define WAVER_PRINT_ERROR_MSG_IMPL_1(x) WAVER_PRINT_ERROR_MSG_IMPL_SINGLE(x)
#define WAVER_PRINT_ERROR_MSG_IMPL_2(x, y) WAVER_PRINT_ERROR_MSG_IMPL_BINARY(x, y)
// flush the stderr to make sure the error message to be shown before SIGTRAP was raised.
#define WAVER_PRINT_ERROR_MSG(...)                                                                                     \
	do {                                                                                                                 \
		WAVER_PRINT_ERROR_MSG_IMPL(__VA_ARGS__, 2, 1)(__VA_ARGS__);                                                        \
		fflush(stderr);                                                                                                    \
		WAVER_RUNTIME_DEBUG_RAISE                                                                                          \
	} while (false);
#define WAVER_PRINT_ERROR_MSG_IMPL(_1, _2, N, ...) WAVER_PRINT_ERROR_MSG_IMPL_##N
#define WAVER_RUNTIME_REQUIRE_IMPL_EQUAL(x, y)                                                                         \
	WAVER_AMBIGUOUS_ELSE_BLOCKER                                                                                         \
	if ((x) == (y))                                                                                                      \
		;                                                                                                                  \
	else {                                                                                                               \
		WAVER_PRINT_ERROR_MSG(x, y)                                                                                        \
	}
#define WAVER_RUNTIME_REQUIRE_IMPL_SATISFY(x)                                                                          \
	WAVER_AMBIGUOUS_ELSE_BLOCKER                                                                                         \
	if (x)                                                                                                               \
		;                                                                                                                  \
	else {                                                                                                               \
		WAVER_PRINT_ERROR_MSG(x)                                                                                           \
	}
#define WAVER_RUNTIME_REQUIRE_IMPL_2(x, y) WAVER_RUNTIME_REQUIRE_IMPL_EQUAL(x, y)
#define WAVER_RUNTIME_REQUIRE_IMPL_1(x) WAVER_RUNTIME_REQUIRE_IMPL_SATISFY(x)
#if WAVER_DEBUG_ENABLED
#define WAVER_RUNTIME_REQUIRE_IMPL(...) WAVER__VFUNC(WAVER_RUNTIME_REQUIRE_IMPL, __VA_ARGS__)
#else
// if debug is turned off, do nothing.
#define WAVER_RUNTIME_REQUIRE_IMPL(...)
#endif

#define WAVER_RUNTIME_ASSERT(...) WAVER_RUNTIME_REQUIRE_IMPL(__VA_ARGS__);
