#pragma once
#include "config.hpp"
#include <absl/status/status.h>
#include <filesystem>
#include <fstream>
#include <memory.h>
#include <string>
#include <string_view>
#include <vector>

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
struct value_change_dump;

struct port {
  enum type {
    kUnknown = 0,
    kInput = 1,
    kOutput = 2,
    kInout = kInput | kOutput, // 3
    kWire = 4,
    kRegistor = 8,
  };
  type type;
  size_t width;
  identifier_t identifier; // 1 or 2 characters
  std::string name;
  std::string reference; // [4:0], a[0], a[4:0]
};
class scope {
public:
  std::vector<std::shared_ptr<scope>> subscopes;
  inline constexpr std::string_view get_name() const noexcept {
    return get_name_impl();
  }

public:
  inline constexpr scope() = default;
  inline virtual constexpr ~scope() = default;

private:
  [[clang::always_inline]] constexpr virtual std::string_view
  get_name_impl() const noexcept = 0;
  // static constinit inline const auto name = "<unnamed scope>"sv;
};
class module : public scope {
public:
  module &set_name(const std::string_view name) noexcept {
    this->name = name;
    return *this;
  }
  std::vector<port> ports;

public:
  inline constexpr module() = default;
  inline explicit constexpr module(const std::string_view name) noexcept
      : name(name) {}
  inline virtual constexpr ~module() = default;

private:
  virtual std::string_view get_name_impl() const noexcept override {
    return name;
  }
  std::string
      name; // dont use string_view here! name will be `\0ame`, dunno why
};
class task : public scope {};

struct timestamp {
  using value_t = std::string;
  using change = std::pair<identifier_t, value_t>;

  time_t time;
  std::vector<change> changes;
};

struct version {};
struct date {};
struct timescale {};

/// @brief Represents the header part of a VCD file, which contains the module
/// definitions, timescale, and date
struct header {
  std::vector<std::shared_ptr<scope>> scopes;
  version version;
  date date;
  timescale timescale;
};
/// @brief Represents the value change part of a VCD file
struct value_changes {
  std::vector<timestamp> timestamps;
};

/// @brief Represents a Value Change Dump (VCD) file
struct value_change_dump {
  header header;
  value_changes value_changes;
};
} // namespace net::ancillarycat::waver