#pragma once
#include <absl/status/status.h>
#include <string>
#include <string_view>
namespace net::ancillarycat::waver {
using absl::AlreadyExistsError;
using absl::InvalidArgumentError;
using absl::NotFoundError;
using absl::OkStatus;
using absl::Status;
using absl::StatusCode;
using identifier_t = std::string;
using time_t			 = size_t;
using namespace std::string_view_literals;
} // namespace net::ancillarycat::waver

namespace net::ancillarycat::waver::keywords {
static constinit inline const auto $version				 = "$version"sv;
static constinit inline const auto $date					 = "$date"sv;
static constinit inline const auto $timescale			 = "$timescale"sv;
static constinit inline const auto $scope					 = "$scope"sv;
static constinit inline const auto $upscope				 = "$upscope"sv;
static constinit inline const auto $var						 = "$var"sv;
static constinit inline const auto $comment				 = "$comment"sv;
static constinit inline const auto $dumpvars			 = "$dumpvars"sv;
static constinit inline const auto $dumpall				 = "$dumpall"sv;
static constinit inline const auto $dumpon				 = "$dumpon"sv;
static constinit inline const auto $dumpoff				 = "$dumpoff"sv;
static constinit inline const auto $end						 = "$end"sv;
static constinit inline const auto $enddefinitions = "$enddefinitions"sv;

static constinit inline const auto module = "module"sv;
static constinit inline const auto wire		= "wire"sv;
static constinit inline const auto reg		= "reg"sv;
} // namespace net::ancillarycat::waver::keywords
