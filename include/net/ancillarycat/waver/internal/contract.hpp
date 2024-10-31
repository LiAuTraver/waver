#pragma once
#include <csignal>
#include <cstdio>
#include <iostream>
#include <print>
#include "variadic.h"


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
#if WAVER_DEBUG_ENABLED
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
#ifdef WAVER_USE_BOOST_CONTRACT
#include <boost/contract.hpp>
#define WAVER_PRECONDITION_IMPL_1(x)                                                                                   \
  boost::contract::check WAVER_EXPAND_COUNTER(waver_boost_check_precondition_should_be_true) =                         \
    boost::contract::function().precondition([&]() -> bool { return (!!(x)); });
#define WAVER_PRECONDITION_IMPL_2(x, y)                                                                                \
  boost::contract::check WAVER_EXPAND_COUNTER(waver_boost_check_precondition_should_equal) =                           \
    boost::contract::function().precondition([&]() -> bool { return ((x) == (y)); });
#define WAVER_POSTCONDITION_IMPL_1(x)                                                                                  \
  boost::contract::check WAVER_EXPAND_COUNTER(waver_boost_check_postcondition_should_be_true) =                        \
    boost::contract::function().postcondition([&]() -> bool { return (!!(x)); });
#define WAVER_POSTCONDITION_IMPL_2(x, y)                                                                               \
  boost::contract::check WAVER_EXPAND_COUNTER(waver_boost_check_postcondition_should_equal) =                          \
    boost::contract::function().postcondition([&]() -> bool { return ((x) == (y)); });
#endif


#ifdef WAVER_USE_BOOST_CONTRACT
#define WAVER_RUNTIME_ASSERT(...) WAVER_RUNTIME_REQUIRE_IMPL(__VA_ARGS__);
#define WAVER_PRECONDITION(...) WAVER__VFUNC(WAVER_PRECONDITION_IMPL, __VA_ARGS__)
#define WAVER_POSTCONDITION(...) WAVER__VFUNC(WAVER_POSTCONDITION_IMPL, __VA_ARGS__)
#else
#define WAVER_RUNTIME_REQUIRE_IMPL_2(x, y) WAVER_RUNTIME_REQUIRE_IMPL_EQUAL(x, y)
#define WAVER_RUNTIME_REQUIRE_IMPL_1(x) WAVER_RUNTIME_REQUIRE_IMPL_SATISFY(x)
#define WAVER_RUNTIME_REQUIRE_IMPL(...) WAVER__VFUNC(WAVER_RUNTIME_REQUIRE_IMPL, __VA_ARGS__)
#define WAVER_RUNTIME_ASSERT(...) WAVER_RUNTIME_REQUIRE_IMPL(__VA_ARGS__);
#define WAVER_PRECONDITION(...) WAVER_RUNTIME_REQUIRE_IMPL(__VA_ARGS__)
#define WAVER_POSTCONDITION(...) WAVER_RUNTIME_REQUIRE_IMPL(__VA_ARGS__)
#endif

#else
// if debug was turned off, do nothing.
#define WAVER_RUNTIME_REQUIRE_IMPL(...)
#define WAVER_RUNTIME_ASSERT(...)
#define WAVER_PRECONDITION(...)
#define WAVER_POSTCONDITION(...)
#endif
