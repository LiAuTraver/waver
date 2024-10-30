#pragma once
//////////////////////////////////////////////////////////////////////////
//! @file variadic.h
//! @brief Variadic macros for C and C++.
//!
//! This file provides macros to define and utilize variadic functions
//! in C and C++. It enables overloading of functions based on the
//! number of parameters provided.
//!
//! @note This is an internal header file, included by other library headers.
//!
//! @see https://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments
//!
//! @code
//! // Example usage:
//! #define foo(...) WAVER__VFUNC(foo, ##__VA_ARGS__)
//!
//! void foo_0();
//! void foo_1(int);
//! void foo_2(int, int);
//!
//! foo();            // Calls foo_0()
//! foo(42);         // Calls foo_1(42)
//! foo(42, 43);     // Calls foo_2(42, 43)
//! @endcode
//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
#pragma region variadic macros
// clang-format off
// Helper macros to count the number of arguments up to 63
#define WAVER_VFUNC_ARG_COUNT_IMPL( \
		_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, \
		_10, _11, _12, _13, _14, _15, _16, _17, _18, \
		_19, _20, _21, _22, _23, _24, _25, _26, _27, \
		_28, _29, _30, _31, _32, _33, _34, _35, _36, \
		_37, _38, _39, _40, _41, _42, _43, _44, _45, \
		_46, _47, _48, _49, _50, _51, _52, _53, _54, \
		_55, _56, _57, _58, _59, _60, _61, _62, _63, \
		N, ... \
) N

#define WAVER_VFUNC_ARG_COUNT(...) WAVER_VFUNC_ARG_COUNT_IMPL(_, ##__VA_ARGS__, \
		63, 62, 61, 60, 59, 58, 57, 56, 55, 54, \
		53, 52, 51, 50, 49, 48, 47, 46, 45, 44, \
		43, 42, 41, 40, 39, 38, 37, 36, 35, 34, \
		33, 32, 31, 30, 29, 28, 27, 26, 25, 24, \
		23, 22, 21, 20, 19, 18, 17, 16, 15, 14, \
		13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, \
		0 \
)
// clang-format on
// Helper macros to concatenate function name and argument count
#define WAVER_VFUNC_CONCAT_IMPL(func, underscore, count) func##underscore##count
#define WAVER_VFUNC_CONCAT(func, count) WAVER_VFUNC_CONCAT_IMPL(func, _, count)

// Main macro to select the appropriate function
#define WAVER__VFUNC(func, ...) WAVER_VFUNC_CONCAT(func, WAVER_VFUNC_ARG_COUNT(__VA_ARGS__))(__VA_ARGS__) // NOLINT(bugprone-reserved-identifier)

#ifndef __COUNTER__
#define __COUNTER__ __LINE__
#define WAVER_COUNTER __COUNTER__
#else
#define WAVER_COUNTER __COUNTER__
#endif

// Helper macros to expand __COUNTER__
#define EXPAND_COUNTER_HELPER(prefix, underscore, counter) prefix##underscore##counter
#define EXPAND_COUNTER(name, counter) EXPAND_COUNTER_HELPER(name, _, counter)

#define WAVER_EXPAND_COUNTER(name) EXPAND_COUNTER(name, WAVER_COUNTER)

#pragma endregion
#ifdef __cplusplus
}
#endif
