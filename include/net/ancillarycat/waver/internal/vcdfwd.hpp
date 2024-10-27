#pragma once
#include <absl/status/status.h>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include "config.hpp"
namespace net::ancillarycat::waver {
static constexpr inline auto empty_sv = ""sv;
class file_reader;
struct port;
class scope;
class module;
class task;
struct timestamp;
struct version;
struct date;
struct timescale;
struct header;
struct value_changes;
class value_change_dump;
struct dumpvars;
using ports_value_t = std::string;
using change_t			= std::pair<identifier_t, ports_value_t>;
} // namespace net::ancillarycat::waver
