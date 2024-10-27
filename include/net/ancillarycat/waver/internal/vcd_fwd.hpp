#pragma once
#include <absl/status/status.h>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "config.hpp"
namespace net::ancillarycat::waver {
static constexpr inline auto empty_sv = ""sv;
template <typename PathType, typename StringType, typename InputStreamType, typename StringStreamType>
class file_reader;
class port;
class scope;
class module;
class task;
class timestamp;
class version;
class date;
class timescale;
class header;
class value_changes;
class value_change_dump;
class dumpvars;
using ports_value_t = std::string;
using change_t			= std::pair<identifier_t, ports_value_t>;
} // namespace net::ancillarycat::waver
