/**************************************************************************************
 * @file config.hpp
 * @brief This file contains the configuration for the Waver library.
 * @copyright see @file waver.hpp
 *************************************************************************************/

#pragma once
#include <absl/status/status.h>
#include <absl/status/statusor.h>
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

#ifndef WAVER_IS_JSON_OBJECT
/*!
 *  @def WAVER_IS_JSON_OBJECT
 *
 *  @brief expends to nothing. this macro is used to indicate that
 * 					the current object is a JSON object when it comes to @code to_json@endcode method;
 *					which means the const reference should be treated as a JSON object.
 *
 *  @example
 * 	@code
 * 			class foo {
 * 					bar b;
 * 					baz z;
 * 					friend void WAVER_IS_JSON_OBJECT to_json(json_t &j, const foo &f) {
 * 								j["myfoo"] = {{"bar", b}, {"baz", z}};
 * 					}
 * 			};
 *  @endcode
 *
 */
#define WAVER_IS_JSON_OBJECT // nothing
#endif

#ifndef WAVER_IS_JSON_ARRAY
/*!
 * @def WAVER_IS_JSON_ARRAY
 *
 * @brief expends to nothing. this macro is used to indicate that
 * 				the current object is a JSON array when it comes to @code to_json@endcode method;
 * 				which means the const reference should be treated as a JSON array.
 *
 * @example
 * @code
 * 		class foo {
 * 				std::vector<bar> b;
 *
 * 				friend void WAVER_IS_JSON_ARRAY to_json(json_t &j, const foo &f) {
 * 						j = {b[0], b[1], b[2], ...};
 * 				}
 * 		};
 * @endcode
 *
 * */
#define WAVER_IS_JSON_ARRAY // nothing
#endif

#ifdef WAVER_RUNTIME_ASSERT
#define WAVER_ASSERT(expr) assert(expr)
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
using absl::StatusOr;
using identifier_t = std::string;
using json_t			 = nlohmann::json;

// using time_t			 = size_t;
using namespace std::string_view_literals;
static constexpr inline auto empty_sv = ""sv;
} // namespace net::ancillarycat::waver

namespace net::ancillarycat::waver::keywords {
static inline constexpr auto $version				 = "$version"sv;
static inline constexpr auto $date					 = "$date"sv;
static inline constexpr auto $timescale			 = "$timescale"sv;
static inline constexpr auto $scope					 = "$scope"sv;
static inline constexpr auto $upscope				 = "$upscope"sv;
static inline constexpr auto $var						 = "$var"sv;
static inline constexpr auto $comment				 = "$comment"sv;
static inline constexpr auto $dumpvars			 = "$dumpvars"sv;
static inline constexpr auto $dumpall				 = "$dumpall"sv;
static inline constexpr auto $dumpon				 = "$dumpon"sv;
static inline constexpr auto $dumpoff				 = "$dumpoff"sv;
static inline constexpr auto $end						 = "$end"sv;
static inline constexpr auto $enddefinitions = "$enddefinitions"sv;

static inline constexpr auto module = "module"sv;
static inline constexpr auto task		= "task"sv;
static inline constexpr auto wire		= "wire"sv;
static inline constexpr auto reg		= "reg"sv;
} // namespace net::ancillarycat::waver::keywords

namespace net::ancillarycat::waver::utils {}
