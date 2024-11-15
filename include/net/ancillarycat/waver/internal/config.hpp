/**************************************************************************************
 * @file config.hpp
 * @brief This file contains the configuration for the Waver library.
 * @copyright see @file waver.hpp
 *************************************************************************************/

#pragma once
#include <absl/status/status.h>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <string_view>

/*!
 * @defgroup WaverPredefinedMacros
 *
 * @{
 */

#ifndef WAVER_TO_STRING
/*!
 * @brief a helper macro to expand the argument to a string literal.
 *
 * @def WAVER_TO_STRING_IMPL
 *
 * @param [in] x the argument to be expanded to a string literal.
 */
#define WAVER_TO_STRING_IMPL(x) #x
/*!
 * @brief Expands to a string literal of the argument.
 *
 * @def WAVER_TO_STRING
 *
 * @param [in] x the argument to be expanded to a string literal.
 */
#define WAVER_TO_STRING(x) WAVER_TO_STRING_IMPL(x)
#endif

#ifndef WAVER_NODISCARD
#define WAVER_NODISCARD [[nodiscard]]
#endif

/*!
 * @}
 */

namespace net::ancillarycat::waver {
using absl::AlreadyExistsError;
using absl::InvalidArgumentError;
using absl::NotFoundError;
using absl::OkStatus;
using absl::Status;
using absl::StatusCode;

using identifier_t = std::string;
using json_t       = nlohmann::json;

using namespace std::string_view_literals;
static constexpr inline auto empty_sv = ""sv;
} // namespace net::ancillarycat::waver

namespace net::ancillarycat::waver::keywords {
static inline constexpr auto $version        = "$version"sv;
static inline constexpr auto $date           = "$date"sv;
static inline constexpr auto $timescale      = "$timescale"sv;
static inline constexpr auto $scope          = "$scope"sv;
static inline constexpr auto $upscope        = "$upscope"sv;
static inline constexpr auto $var            = "$var"sv;
static inline constexpr auto $comment        = "$comment"sv;
static inline constexpr auto $dumpvars       = "$dumpvars"sv;
static inline constexpr auto $dumpall        = "$dumpall"sv;
static inline constexpr auto $dumpon         = "$dumpon"sv;
static inline constexpr auto $dumpoff        = "$dumpoff"sv;
static inline constexpr auto $end            = "$end"sv;
static inline constexpr auto $enddefinitions = "$enddefinitions"sv;

static inline constexpr auto module = "module"sv;
static inline constexpr auto task   = "task"sv;
static inline constexpr auto wire   = "wire"sv;
static inline constexpr auto reg    = "reg"sv;
} // namespace net::ancillarycat::waver::keywords
