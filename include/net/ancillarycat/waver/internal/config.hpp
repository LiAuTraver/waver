#pragma once
#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <string>
#include <string_view>

#ifndef AC_NODISCARD
#define AC_NODISCARD [[nodiscard]]
#endif
namespace net::ancillarycat::waver {
using absl::AlreadyExistsError;
using absl::InvalidArgumentError;
using absl::NotFoundError;
using absl::OkStatus;
using absl::Status;
using absl::StatusCode;
using absl::StatusOr;
using identifier_t = std::string;
using time_t			 = size_t;
using namespace std::string_view_literals;
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
