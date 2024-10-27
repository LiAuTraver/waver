#pragma once
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "config.hpp"
#include "vcd_fwd.hpp"

namespace net::ancillarycat::waver {
class vcd_meta_element {
	friend class value_change_dump;

public:
	using json_t				= nlohmann::json;
	using string_t			= std::string;
	using string_view_t = std::string_view;

public:
	inline constexpr					vcd_meta_element()														 = default;
	inline constexpr					vcd_meta_element(const vcd_meta_element &)		 = default;
	inline constexpr					vcd_meta_element(vcd_meta_element &&) noexcept = default;
	inline constexpr virtual ~vcd_meta_element() noexcept										 = default;

private:
	// friend void to_json(json_t &) = 0;
};

class port : public vcd_meta_element {
	friend class value_change_dump;

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
	inline constexpr virtual ~port() noexcept override = default;


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

class scope : public vcd_meta_element {
	friend class value_change_dump;

public:
	using scopes_t = std::vector<std::shared_ptr<scope>>;

public:
	inline constexpr					scope()					 = default;
	inline virtual constexpr ~scope() override = default;

public:
	inline json_t to_json() const { return to_json_impl(); }

private:
	friend inline void to_json(json_t &j, const scope &scope) { j = scope.to_json(); }

private:
	inline virtual json_t to_json_impl() const = 0;

protected:
	scopes_t subscopes;
};
class module : public scope {
	friend class value_change_dump;

public:
	using ports_t = std::vector<port>;

public:
	ports_t ports;

public:
	inline constexpr					module() = default;
	inline explicit constexpr module(const string_view_t name) noexcept : scope(), name(name) {}
	inline virtual constexpr ~module() override = default;

private:
	// fixme: implement to_json
	friend void to_json(json_t &j, const module &module) {
		auto ports_json = json_t{};
		std::ranges::for_each(module.ports, [&](auto &&port) { to_json(ports_json, port); });
		j[module.name] = ports_json;
	}

	virtual inline  json_t to_json_impl() const override {
		auto ports_json = json_t{};
		std::ranges::for_each(ports, [&](const port &port) { net::ancillarycat::waver::to_json(ports_json, port); });
		return json_t{{name, {ports_json}}};
	}

private:
	string_t name; // dont use string_view here! name will be `\0ame`, dunno why
};
class task : public scope {
	friend class value_change_dump;

public:
	inline constexpr					task()					= default;
	inline virtual constexpr ~task() override = default;

private:
	// todo: implement task
	friend void to_json(json_t &j, const task &task) { j["task"] = task.name; }
	virtual inline json_t to_json_impl() const override { return json_t{{"task", {name}}}; }
private:
	// todo, implement task
	string_t name;
};

class timestamp : public vcd_meta_element {
	friend class value_change_dump;

public:
	using changes_t = std::vector<change_t>;
	using time_t		= size_t;

public:
	inline explicit constexpr timestamp() = default;
	inline constexpr timestamp(const time_t time, changes_t changes) noexcept : time(time), changes(std::move(changes)) {}
	inline constexpr timestamp(const timestamp &rhs) : time(rhs.time), changes(rhs.changes) {}
	inline constexpr timestamp(timestamp &&rhs) noexcept {
		time		= rhs.time;
		changes = std::move(rhs.changes);
	}
	inline constexpr timestamp &operator=(const timestamp &rhs) {
		time		= rhs.time;
		changes = rhs.changes;
		return *this;
	}
	inline constexpr timestamp &operator=(timestamp &&rhs) noexcept {
		time		= rhs.time;
		changes = std::move(rhs.changes);
		return *this;
	}
	inline constexpr virtual ~timestamp() noexcept override = default;

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

class version : public vcd_meta_element {
	friend class value_change_dump;

public:
	inline constexpr explicit version() = default;
	inline constexpr					version(const version &rhs) : description(rhs.description) {}
	inline constexpr					version(version &&rhs) noexcept { description = std::move(rhs.description); }
	inline constexpr version &operator=(const version &rhs) {
		description = rhs.description;
		return *this;
	}
	inline constexpr version &operator=(version &&rhs) noexcept {
		description = std::move(rhs.description);
		return *this;
	}

	inline virtual constexpr ~version() override = default;

private:
	friend void to_json(json_t &j, const version &version) { j["version"] = version.description; }

private:
	string_t description;
};
class date : public vcd_meta_element {
	friend class value_change_dump;

public:
	inline explicit constexpr date()					= default;
	inline virtual constexpr ~date() override = default;
	inline constexpr					date(const date &rhs) : time_point(rhs.time_point) {}
	inline constexpr					date(date &&rhs) noexcept { time_point = std::move(rhs.time_point); }
	inline constexpr date		 &operator=(const date &rhs) {
		 time_point = rhs.time_point;
		 return *this;
	}
	inline constexpr date &operator=(date &&rhs) noexcept {
		time_point = std::move(rhs.time_point);
		return *this;
	}

private:
	friend void to_json(json_t &j, const date &date) { j["date"] = date.time_point; }

private:
	// todo, implement date
	string_t time_point;
};
class timescale : public vcd_meta_element {
	friend class value_change_dump;

public:
	using time_t = string_t;

public:
	inline explicit constexpr		timescale()					 = default;
	inline virtual constexpr ~	timescale() override = default;
	inline constexpr						timescale(const timescale &rhs) : time(rhs.time) {}
	inline constexpr						timescale(timescale &&rhs) noexcept { time = std::move(rhs.time); }
	inline constexpr timescale &operator=(const timescale &rhs) {
		time = rhs.time;
		return *this;
	}
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
class header : public vcd_meta_element {
	friend class value_change_dump;

public:
	using scopes_t = std::vector<std::shared_ptr<scope>>;

public:
	inline explicit constexpr header() = default;
	inline constexpr					header(const header &rhs) :
			scopes(rhs.scopes), version(rhs.version), date(rhs.date), timescale(rhs.timescale) {}
	inline constexpr header(header &&rhs) noexcept {
		scopes		= std::move(rhs.scopes);
		version		= rhs.version;
		date			= rhs.date;
		timescale = rhs.timescale;
	}
	inline constexpr header &operator=(const header &rhs) {
		scopes		= rhs.scopes;
		version		= rhs.version;
		date			= rhs.date;
		timescale = rhs.timescale;
		return *this;
	}
	inline constexpr header &operator=(header &&rhs) noexcept {
		scopes		= std::move(rhs.scopes);
		version		= rhs.version;
		date			= rhs.date;
		timescale = rhs.timescale;
		return *this;
	}
	inline constexpr virtual ~header() noexcept override = default;

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
	scopes_t	scopes;
	version		version;
	date			date;
	timescale timescale;
};
/// @brief Represents the value change part of a VCD file
class value_changes : public vcd_meta_element {
	friend class value_change_dump;

public:
	using timestamps_t = std::vector<timestamp>;

public:
	inline explicit constexpr value_changes() = default;
	inline constexpr					value_changes(const value_changes &rhs) : timestamps(rhs.timestamps) {}
	inline constexpr					value_changes(value_changes &&rhs) noexcept { timestamps = std::move(rhs.timestamps); }
	inline constexpr value_changes &operator=(const value_changes &rhs) {
		timestamps = rhs.timestamps;
		return *this;
	}
	inline constexpr value_changes &operator=(value_changes &&rhs) noexcept {
		timestamps = std::move(rhs.timestamps);
		return *this;
	}
	inline constexpr virtual ~value_changes() noexcept override = default;

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
class dumpvars : public vcd_meta_element {
	friend value_change_dump;

public:
	using changes_t = std::vector<change_t>;

private:
	changes_t changes;

public:
	inline explicit constexpr dumpvars() = default;
	inline constexpr					dumpvars(const dumpvars &rhs) : changes(rhs.changes) {}
	inline constexpr					dumpvars(dumpvars &&rhs) noexcept { changes = std::move(rhs.changes); }
	inline constexpr auto			operator=(const dumpvars &rhs) -> dumpvars		 &{
		changes = rhs.changes;
		return *this;
	}
	inline constexpr auto operator=(dumpvars &&rhs) noexcept -> dumpvars & {
		changes = std::move(rhs.changes);
		return *this;
	}
	inline constexpr virtual ~dumpvars() noexcept override = default;

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
