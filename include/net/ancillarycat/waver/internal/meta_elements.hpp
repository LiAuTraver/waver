#pragma once
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <utility>

#include "config.hpp"
#include "vcd_fwd.hpp"

namespace net::ancillarycat::waver {
class port {
	friend class value_change_dump;
	using json_t				= nlohmann::json;
	using string_t			= std::string;
	using string_view_t = std::string_view;

public:
	enum type : std::uint8_t {
		kUnknown	= 0,
		kInput		= 1,
		kOutput		= 2,
		kInout		= kInput | kOutput, // 3
		kWire			= 4,
		kRegistor = 8,
	};

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


	friend void to_json(json_t &j, const port &port);

private:
	type				 type = kUnknown;
	size_t			 width;
	identifier_t identifier; // 1 or 2 characters
	string_t		 name;
	string_t		 reference; // [4:0], a[0], a[4:0]
};
inline void to_json(port::json_t &j, const port &port) {
	// clang-format off
	j = port::json_t{{port.name, // key
		{
				 {"type", port.type},
				 {"width", port.width},
				 {"identifier", port.identifier},
				 {"reference", port.reference}
		}
	}};
	// clang-format on
}

class scope_value_base {
public:
	inline explicit constexpr scope_value_base() = default;
	virtual inline constexpr ~scope_value_base() = default;
	inline constexpr					scope_value_base(const scope_value_base &) {} // NOLINT(*-use-equals-default)
	inline constexpr					scope_value_base(scope_value_base &&) noexcept {}

public:
	using json_t				= nlohmann::json;
	using string_t			= std::string;
	using string_view_t = std::string_view;
	// inline constexpr scope_value_base &operator=(const scope_value_base &other) {}
	// inline constexpr scope_value_base &operator=(scope_value_base &&other) noexcept {}
};
class module : public scope_value_base {
	friend class value_change_dump;
	friend class scope;

public:
	using ports_t = std::vector<port>;

public:
	inline explicit constexpr module() : scope_value_base() {}
	inline constexpr					module(const module &rhs) noexcept : scope_value_base(), ports(rhs.ports) {}
	inline constexpr					module(module &&rhs) noexcept : scope_value_base(), ports(std::move(rhs.ports)) {}
	inline virtual constexpr ~module() noexcept override{}; // NOLINT(*-use-equals-default)

private:
	ports_t ports;
};

class task : public scope_value_base {
	friend class value_change_dump;
	friend class scope;

public:
	inline explicit constexpr task() : scope_value_base() {}
	inline constexpr					task(const task &rhs) noexcept : scope_value_base() {}
	inline constexpr					task(task &&rhs) noexcept : scope_value_base() {}
	inline virtual constexpr ~task() noexcept override{}; // NOLINT(*-use-equals-default)
	// todo: implement task
};

class scope {
	friend class value_change_dump;

public:
	using json_t				= nlohmann::json;
	using string_t			= std::string;
	using string_view_t = std::string_view;
	using scopes_t			= std::vector<std::shared_ptr<scope>>;
	using data_ptr_t		= std::shared_ptr<scope_value_base>;
	using module_t			= module;
	using task_t				= task;

public:
	inline explicit constexpr scope() = default;
	inline 				scope(const scope &rhs) noexcept : // NOLINT(*-use-equals-default)
			name(rhs.name), subscopes(rhs.subscopes), data(rhs.data) {}

	inline constexpr scope &operator=(const scope &other) noexcept {
		if (this == &other)
			return *this;
		name			= other.name;
		subscopes = other.subscopes;
		data			= other.data;
		return *this;
	}

	inline scope(scope &&rhs) noexcept :
			name(std::move(rhs.name)), subscopes(std::move(rhs.subscopes)), data(std::move(rhs.data)) {}
	inline constexpr scope &operator=(scope &&other) noexcept {
		if (this == &other)
			return *this;
		name			= std::move(other.name);
		subscopes = std::move(other.subscopes);
		data			= std::move(other.data);
		return *this;
	}
	inline constexpr ~scope() noexcept = default;

private:
	string_t	 name;
	scopes_t	 subscopes;
	data_ptr_t data;
};

class timestamp {
	friend class value_change_dump;

public:
	using changes_t = std::vector<change_t>;
	using time_t		= size_t;
	using json_t		= nlohmann::json;


public:
	inline explicit constexpr timestamp() = default;
	inline constexpr timestamp(const time_t time, changes_t changes) noexcept : time(time), changes(std::move(changes)) {}
	inline constexpr timestamp(const timestamp &rhs) = default;
	inline constexpr timestamp(timestamp &&rhs) noexcept {
		time		= rhs.time;
		changes = std::move(rhs.changes);
	}
	inline constexpr timestamp &operator=(const timestamp &rhs) = default;
	inline constexpr timestamp &operator=(timestamp &&rhs) noexcept {
		time		= rhs.time;
		changes = std::move(rhs.changes);
		return *this;
	}
	inline constexpr virtual ~timestamp() noexcept = default;

private:
	time_t		time = 0;
	changes_t changes;

private:
	// todo, implement timestamp
	friend void to_json(json_t &j, const timestamp &timestamp) {
		j["time"]					 = timestamp.time;
		auto changes_array = json_t{};
		std::ranges::for_each(timestamp.changes, [&](auto &&change) {
			json_t change_json;
			change_json[change.first] = change.second;
			changes_array.emplace_back(change_json);
		});
		j["changes"] = changes_array;
	}
};

class version {
	friend class value_change_dump;
	using json_t	 = nlohmann::json;
	using string_t = std::string;

public:
	inline constexpr explicit version()										= default;
	inline constexpr					version(const version &rhs) = default;
	inline constexpr					version(version &&rhs) noexcept { description = std::move(rhs.description); }
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
	using json_t	 = nlohmann::json;
	using string_t = std::string;

public:
	inline explicit constexpr date()								= default;
	inline virtual constexpr ~date()								= default;
	inline constexpr					date(const date &rhs) = default;
	inline constexpr					date(date &&rhs) noexcept { time_point = std::move(rhs.time_point); }
	inline constexpr date		 &operator=(const date &rhs) = default;
	inline constexpr date		 &operator=(date &&rhs) noexcept {
		 time_point = std::move(rhs.time_point);
		 return *this;
	}

private:
	friend void to_json(json_t &j, const date &date) { j["date"] = date.time_point; }

private:
	// todo, implement date
	string_t time_point;
};
class timescale {
	friend class value_change_dump;
	using json_t	 = nlohmann::json;
	using string_t = std::string;

public:
	using time_t = string_t;

public:
	inline explicit constexpr		timescale()											= default;
	inline virtual constexpr ~	timescale()											= default;
	inline constexpr						timescale(const timescale &rhs) = default;
	inline constexpr						timescale(timescale &&rhs) noexcept { time = std::move(rhs.time); }
	inline constexpr timescale &operator=(const timescale &rhs) = default;
	inline constexpr timescale &operator=(timescale &&rhs) noexcept {
		time = std::move(rhs.time);
		return *this;
	}

private:
	friend void to_json(json_t &j, const timescale &timescale) { j["timescale"] = timescale.time; }

private:
	string_t time;
};

/// @brief Represents the header part of a VCD file, which contains the module
/// definitions, timescale, and date
class header {
	friend class value_change_dump;
	using json_t	 = nlohmann::json;
	using string_t = std::string;

public:
	using scopes_t = std::vector<std::shared_ptr<scope>>;

public:
	inline explicit constexpr header()							 = default;
	inline constexpr					header(const header &) = default;
	inline 	header(header &&rhs) noexcept {
		 scopes		 = std::move(rhs.scopes);
		 version	 = rhs.version;
		 date			 = rhs.date;
		 timescale = rhs.timescale;
	}
	inline constexpr header &operator=(const header &) = default;
	inline header &operator=(header &&rhs) noexcept {
		scopes		= std::move(rhs.scopes);
		version		= rhs.version;
		date			= rhs.date;
		timescale = rhs.timescale;
		return *this;
	}
	inline constexpr virtual ~header() noexcept = default;

private:
	friend void to_json(json_t &j, const header &header) {
		auto scopes_json = json_t{};
		// std::ranges::for_each(header.scopes, [&](auto &&scope) { to_json(scopes_json, *scope); });
		j["scopes"] = scopes_json;
		to_json(j, header.version);
		to_json(j, header.date);
		to_json(j, header.timescale);
	}

private:
	scopes_t	scopes;
	version		version;
	date			date;
	timescale timescale;
};
/// @brief Represents the value change part of a VCD file
class value_changes {
	friend class value_change_dump;

public:
	using timestamps_t = std::vector<timestamp>;
	using json_t			 = nlohmann::json;
	using string_t		 = std::string;

public:
	inline explicit constexpr value_changes()											 = default;
	inline constexpr					value_changes(const value_changes &) = default;
	inline constexpr					value_changes(value_changes &&rhs) noexcept { timestamps = std::move(rhs.timestamps); }
	inline constexpr value_changes &operator=(const value_changes &) = default;
	inline constexpr value_changes &operator=(value_changes &&rhs) noexcept {
		timestamps = std::move(rhs.timestamps);
		return *this;
	}
	inline constexpr virtual ~value_changes() noexcept = default;

private:
	friend void to_json(json_t &j, const value_changes &value_changes) {
		auto timestamps_json = json_t{};
		std::ranges::for_each(value_changes.timestamps, [&](auto &&timestamp) { to_json(timestamps_json, timestamp); });
		j["timestamps"] = timestamps_json;
	}

private:
	timestamps_t timestamps;
};

/// @brief initial value of ports
class dumpvars {
	friend value_change_dump;

public:
	using changes_t = std::vector<change_t>;
	using json_t		= nlohmann::json;
	using string_t	= std::string;

private:
	changes_t changes;

public:
	inline explicit constexpr	 dumpvars()									= default;
	inline constexpr					 dumpvars(const dumpvars &) = default;
	inline constexpr					 dumpvars(dumpvars &&rhs) noexcept { changes = std::move(rhs.changes); }
	inline constexpr dumpvars &operator=(const dumpvars &) = default;
	inline constexpr auto			 operator=(dumpvars &&rhs) noexcept -> dumpvars			 &{
		 changes = std::move(rhs.changes);
		 return *this;
	}
	inline constexpr virtual ~dumpvars() noexcept = default;

private:
	friend void to_json(json_t &j, const dumpvars &dumpvars) {
		auto changes_json = json_t{};
		std::ranges::for_each(dumpvars.changes, [&](auto &&change) {
			json_t change_json;
			change_json[change.first] = change.second;
			changes_json.emplace_back(change_json);
		});
		j["dumpvars"] = changes_json;
	}
};
} // namespace net::ancillarycat::waver
