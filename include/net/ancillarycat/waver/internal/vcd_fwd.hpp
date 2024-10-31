/******************************************************************************
 *
 * @file vcd_fwd.hpp
 *
 * @brief forward declarations for the VCD parser.
 *
 *****************************************************************************/
#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "config.hpp"
namespace net::ancillarycat::waver {
template <typename PathType, typename StringType, typename InputStreamType, typename StringStreamType>
class file_reader;

template <typename StringType, typename StringViewType, typename PathType, typename BooleanType, typename StatusType>
class lexer;

class port;
class scope_value_base;
class module;
class task;
class scope;
class timestamp;
class version;
class date;
class timescale;
class header;
class value_change_dump;
class dumpvars;

class value_changes;

using ports_value_t                      = std::string;
using string_t                           = std::string;
using change_t                           = std::pair<identifier_t, ports_value_t>;
} // namespace net::ancillarycat::waver
