#pragma once
#ifdef WAVER_USE_BOOST_CONTRACT
#include <boost/contract/check.hpp>
#include <boost/contract/function.hpp>
#endif
#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <print>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#include "config.hpp"
#include "contract.hpp"
#include "variadic.h"
#include "vcd_fwd.hpp"


namespace net::ancillarycat::waver {
/// @brief Represents the identifier of a port
/// @note the identifier is a 1 or 2 character string
class port {
  friend class value_change_dump;
  friend class module;
  friend inline void to_json(json_t &j, const module &module);

public:
  using json_t        = nlohmann::json;
  using string_t      = std::string;
  using string_view_t = std::string_view;
  using size_t        = std::size_t;

public:
  /// @brief Represents the type of the port
  enum type : std::uint8_t;

public:
  inline explicit constexpr port(const type type, const size_t width, identifier_t identifier, std::string name,
                                 std::string reference) noexcept :
      type(type), width(width), identifier(std::move(identifier)), name(std::move(name)),
      reference(std::move(reference)) {}
  inline constexpr port(const port &) noexcept = default;
  inline constexpr port(port &&rhs) noexcept :
      type(rhs.type), width(rhs.width), identifier(std::move(rhs.identifier)), name(std::move(rhs.name)),
      reference(std::move(rhs.reference)) {}
  inline constexpr virtual ~port() noexcept = default;

  /// @brief friend function to convert the port to json, i.e., serialize it
  /// @param j the json object
  /// @param port the port to serialize
  /// @note the json object will be an object with the port name as the key
  friend inline void to_json(port::json_t &j, const port &port) {
    // clang-format off
		j = port::json_t{{port.name, // <- key, value vvv
			{
					 {"type", port.type},
					 {"width", port.width},
					 {"identifier", port.identifier},
					 {"reference", port.reference}
			}
		}};
    // clang-format on
    WAVER_POSTCONDITION(j.is_object());
  }

public:
  enum type : std::uint8_t {
    kUnknown  = 0,
    kInput    = 1,
    kOutput   = 2,
    kInout    = kInput | kOutput, // 3
    kWire     = 4,
    kRegistor = 8,
  };

private:
  type         type = kUnknown;
  size_t       width;
  identifier_t identifier; // 1 or 2 characters
  string_t     name;
  string_t     reference; // [4:0], a[0], a[4:0]
};

/*!
 * 	@brief Represents the base class for all scope values
 * 	@interface scope_value_base
 */
class scope_value_base {
  friend class value_change_dump;
  friend class scope;

public:
  using json_t        = nlohmann::json;
  using string_t      = std::string;
  using string_view_t = std::string_view;
  /// @brief Represents the type of the scope.
  enum scope_type : std::uint8_t;

public:
  inline explicit constexpr scope_value_base() = default;
  virtual inline constexpr ~scope_value_base() = default;
  inline constexpr scope_value_base(const scope_value_base &) {} // NOLINT(*-use-equals-default)
  inline constexpr scope_value_base(scope_value_base &&) noexcept {}
  WAVER_NODISCARD WAVER_FORCEINLINE constexpr scope_type get_type() const noexcept { return get_type_impl(); }

private:
  WAVER_NODISCARD WAVER_FORCEINLINE virtual constexpr scope_type get_type_impl() const noexcept = 0;

public:
  enum scope_type : std::uint8_t {
    kUnknown = 0,
    kModule  = 1,
    kTask    = 2,
  };
};

/// @brief Represents a module in a VCD file
class module : public scope_value_base {
  friend class value_change_dump;
  friend class scope;

public:
  using ports_t = std::vector<port>;

public:
  inline explicit constexpr module() : scope_value_base() {}
  inline constexpr module(const module &rhs) noexcept : scope_value_base(), ports(rhs.ports) {}
  inline constexpr module(module &&rhs) noexcept : scope_value_base(), ports(std::move(rhs.ports)) {}
  inline virtual constexpr ~module() noexcept override = default;

private:
  ports_t ports;

private:
  WAVER_FORCEINLINE WAVER_NODISCARD virtual constexpr scope_type get_type_impl() const noexcept override { return scope_type::kModule; }

private:
  friend inline void to_json(json_t &j, const module &module) {
    std::ranges::for_each(module.ports, [&](auto &&port) {
      auto port_json = json_t{};
      to_json(port_json, port);
      WAVER_RUNTIME_ASSERT(port_json.is_object());
      WAVER_RUNTIME_ASSERT(port_json.front() == port_json.back());
      j["ports"].merge_patch(port_json);
    });
  WAVER_POSTCONDITION(j.is_object());
  }

};
class task : public scope_value_base {
  friend class value_change_dump;
  friend class scope;

public:
  inline explicit constexpr task() : scope_value_base() {}
  inline constexpr task(const task &) noexcept : scope_value_base() {}
  inline constexpr task(task &&) noexcept : scope_value_base() {}
  inline virtual constexpr ~task() noexcept override = default;
private:
  WAVER_FORCEINLINE WAVER_NODISCARD virtual scope_type get_type_impl() const noexcept override { return scope_type::kTask; }
  // todo: implement task

private:
  friend inline void to_json(json_t &j, const task &task) {
    WAVER_POSTCONDITION(j.is_object());

    j.emplace_back("task");
  }
};

class scope {
  friend class value_change_dump;

public:
  using json_t        = nlohmann::json;
  using string_t      = std::string;
  using string_view_t = std::string_view;
  using scopes_t      = std::vector<std::shared_ptr<scope>>;
  using data_ptr_t    = std::shared_ptr<scope_value_base>;
  using module_t      = module;
  using task_t        = task;

public:
  inline explicit constexpr scope() = default;
  inline scope(const scope &rhs) noexcept : // NOLINT(*-use-equals-default)
      name(rhs.name), subscopes(rhs.subscopes), data(rhs.data) {}

  inline constexpr scope &operator=(const scope &other) noexcept {
    if (this == &other)
      return *this;
    name      = other.name;
    subscopes = other.subscopes;
    data      = other.data;
    return *this;
  }

  inline scope(scope &&rhs) noexcept :
      name(std::move(rhs.name)), subscopes(std::move(rhs.subscopes)), data(std::move(rhs.data)) {}
  inline constexpr scope &operator=(scope &&other) noexcept {
    if (this == &other)
      return *this;
    name      = std::move(other.name);
    subscopes = std::move(other.subscopes);
    data      = std::move(other.data);
    return *this;
  }
  inline constexpr ~scope() noexcept = default;

private:
  /// @remark because it holds a shared_ptr, the to_json dinstincts with others.
  friend inline void to_json(json_t &j, const scope &scope) {

    auto subscopes_json = json_t{};
    std::ranges::for_each(scope.subscopes, [&](auto &&subscope) { to_json(subscopes_json, *subscope); });
    auto data_json = json_t{};
    switch (scope.data->get_type()) {
    case scope_value_base::scope_type::kModule: {
      j["type"]   = "module";
      auto module = std::dynamic_pointer_cast<module_t>(scope.data);
      to_json(data_json, *module);
      break;
    }
    case scope_value_base::scope_type::kTask: {
      j["type"] = "task";
      auto task = std::dynamic_pointer_cast<task_t>(scope.data);
      to_json(data_json, *task);
      break;
    }
    default: {
      j["type"] = "unknown";
      break;
    }
    }
    j["name"]      = scope.name;
    j["data"]      = data_json; //! <- data json shall be an array
    j["subscopes"] = subscopes_json;

    WAVER_POSTCONDITION(j.is_object());
  }

private:
  string_t   name;
  scopes_t   subscopes;
  data_ptr_t data;
};

class timestamp {
  friend class value_change_dump;

public:
  using changes_t = std::unordered_map<identifier_t, ports_value_t>;
  using time_t    = size_t;
  using json_t    = nlohmann::json;


public:
  inline explicit constexpr timestamp() = default;
  inline timestamp(const time_t time, changes_t changes) noexcept : time(time), changes(std::move(changes)) {}
  inline constexpr timestamp(const timestamp &rhs) = default;
  inline timestamp(timestamp &&rhs) noexcept {
    time    = rhs.time;
    changes = std::move(rhs.changes);
  }
  inline constexpr timestamp &operator=(const timestamp &rhs) = default;
  inline timestamp           &operator=(timestamp &&rhs) noexcept {
    time    = rhs.time;
    changes = std::move(rhs.changes);
    return *this;
  }
  inline constexpr virtual ~timestamp() noexcept = default;

private:
  time_t    time = 0;
  changes_t changes;

private:
  /// @note strengthened
  friend void to_json(json_t &j, const timestamp &timestamp) {
    j[std::to_string(timestamp.time)].merge_patch(timestamp.changes);

    WAVER_POSTCONDITION(j.is_object());
  }
};

class version {
  friend class value_change_dump;
  using json_t   = nlohmann::json;
  using string_t = std::string;

public:
  inline constexpr explicit version()          = default;
  inline constexpr version(const version &rhs) = default;
  inline constexpr version(version &&rhs) noexcept { description = std::move(rhs.description); }
  inline constexpr version &operator=(const version &rhs) = default;
  inline constexpr version &operator=(version &&rhs) noexcept {
    description = std::move(rhs.description);
    return *this;
  }

  inline virtual constexpr ~version() = default;

private:
  friend void to_json(json_t &j, const version &version) { j["version"] = version.description; }

private:
  string_t description;
};

class date {
  friend class value_change_dump;
  using json_t   = nlohmann::json;
  using string_t = std::string;

public:
  inline explicit constexpr date()       = default;
  inline virtual constexpr ~date()       = default;
  inline constexpr date(const date &rhs) = default;
  inline constexpr date(date &&rhs) noexcept { time_point = std::move(rhs.time_point); }
  inline constexpr date &operator=(const date &rhs) = default;
  inline constexpr date &operator=(date &&rhs) noexcept {
    time_point = std::move(rhs.time_point);
    return *this;
  }

private:
  friend void to_json(json_t &j, const date &date) {
    WAVER_POSTCONDITION(j.is_object());

    if (not date.time_point.empty())
      j["date"] = date.time_point;
  }

private:
  // todo, implement date
  string_t time_point;
};
class timescale {
  friend class value_change_dump;
  using json_t   = nlohmann::json;
  using string_t = std::string;

public:
  using time_t = string_t;

public:
  inline explicit constexpr timescale()            = default;
  inline virtual constexpr ~timescale()            = default;
  inline constexpr timescale(const timescale &rhs) = default;
  inline constexpr timescale(timescale &&rhs) noexcept { time = std::move(rhs.time); }
  inline constexpr timescale &operator=(const timescale &rhs) = default;
  inline constexpr timescale &operator=(timescale &&rhs) noexcept {
    time = std::move(rhs.time);
    return *this;
  }

private:
  friend void to_json(json_t &j, const timescale &timescale) {
    if (not timescale.time.empty())
      j["timescale"] = timescale.time;
  }

private:
  string_t time;
};

/// @brief Represents the header part of a VCD file, which contains the module
/// definitions, timescale, and date
class header {
  friend class value_change_dump;
  using json_t   = nlohmann::json;
  using string_t = std::string;

public:
  using scopes_t = std::vector<std::shared_ptr<scope>>;

public:
  inline explicit constexpr header()      = default;
  inline constexpr header(const header &) = default;
  inline header(header &&rhs) noexcept {
    scopes    = std::move(rhs.scopes);
    version   = rhs.version;
    date      = rhs.date;
    timescale = rhs.timescale;
  }
  inline constexpr header &operator=(const header &) = default;
  inline header           &operator=(header &&rhs) noexcept {
    scopes    = std::move(rhs.scopes);
    version   = rhs.version;
    date      = rhs.date;
    timescale = rhs.timescale;
    return *this;
  }
  inline constexpr virtual ~header() noexcept = default;

private:
  friend void to_json(json_t &j, const header &header) {
    auto scopes_json = json_t{};
    std::ranges::for_each(header.scopes, [&](auto &&scope) { to_json(scopes_json, *scope); });
    j["scopes"] = scopes_json;
    to_json(j, header.version);
    to_json(j, header.date);
    to_json(j, header.timescale);
  }

private:
  scopes_t  scopes;
  version   version;
  date      date;
  timescale timescale;
};
/// @brief Represents the value change part of a VCD file
class value_changes {
  friend class value_change_dump;

public:
  using timestamps_t = std::vector<timestamp>;
  using json_t       = nlohmann::json;
  using string_t     = std::string;

public:
  inline explicit constexpr value_changes()             = default;
  inline constexpr value_changes(const value_changes &) = default;
  inline constexpr value_changes(value_changes &&rhs) noexcept { timestamps = std::move(rhs.timestamps); }
  inline constexpr value_changes &operator=(const value_changes &) = default;
  inline constexpr value_changes &operator=(value_changes &&rhs) noexcept {
    timestamps = std::move(rhs.timestamps);
    return *this;
  }
  inline constexpr virtual ~value_changes() noexcept = default;

private:
  friend void to_json(json_t &j, const value_changes &value_changes) {
    WAVER_POSTCONDITION(j.is_object());

    std::ranges::for_each(value_changes.timestamps, [&](auto &&timestamp) {
      auto timestamp_json = json_t{};
      to_json(timestamp_json, timestamp);
      WAVER_PRECONDITION(timestamp_json.is_object());
      WAVER_PRECONDITION(timestamp_json.front() == timestamp_json.back());
      j[WAVER_TO_STRING(value_changes)].merge_patch(timestamp_json);
    });
  }

private:
  timestamps_t timestamps;
};

/// @brief initial value of ports
class dumpvars {
  friend value_change_dump;

public:
  using changes_t = std::vector<change_t>;
  using json_t    = nlohmann::json;
  using string_t  = std::string;

private:
  changes_t changes;

public:
  inline explicit constexpr dumpvars()        = default;
  inline constexpr dumpvars(const dumpvars &) = default;
  inline constexpr dumpvars(dumpvars &&rhs) noexcept { changes = std::move(rhs.changes); }
  inline constexpr dumpvars &operator=(const dumpvars &) = default;
  inline constexpr dumpvars &operator=(dumpvars &&rhs) noexcept {
    changes = std::move(rhs.changes);
    return *this;
  }
  inline constexpr virtual ~dumpvars() noexcept = default;

private:
  friend void to_json(json_t &j, const dumpvars &dumpvars) {
    WAVER_POSTCONDITION(j.is_object());

    j[WAVER_TO_STRING(dumpvars)].emplace_back(dumpvars.changes);
  }
};
} // namespace net::ancillarycat::waver
