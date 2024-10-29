
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <print>
#include <string>
#include <vector>
struct person {
	std::string						name;
	int										age;
	std::filesystem::path home; // `fake`
	friend void						to_json(nlohmann::json &j, const person &person) {
		j = nlohmann::json{{"name", person.name}, {"age", person.age}, {"home", person.home.string()}};
	}
};
using json = nlohmann::json;
void test();
int	 main();

int main() {
	nlohmann::json j1 = {"123"};
	nlohmann::json j2 = {123, "123"};
	nlohmann::json j3 = {123, "123", 123.123};
	nlohmann::json j4 = {123, "123", 123.123, true}; // array
	nlohmann::json j5 = {123, {"123", "asdfsfd"}};// array, value, subarray
	nlohmann::json j6 = {"qwe", {"123", "asdfsfd"}}; // ditto
	nlohmann::json j7 = {{"qwe", {"123", "asdfsfd"}}}; // object, key-array pair
	std::println("j1: {}", j1.dump(4));
	std::println("j2: {}", j2.dump(4));
	std::println("j3: {}", j3.dump(4));
	std::println("j4: {}", j4.dump(4));
	std::println("j5: {}", j5.dump(4));
	std::println("j6: {}", j6.dump(4));
	std::println("j7: {}", j7.dump(4));

	auto j_array = nlohmann::json::array();
	j_array.emplace_back("123");
	nlohmann::json j10 = {{"wer", j_array}};
	std::println("j10: {}", j10.dump(4));
}


void test() {
	std::vector<std::pair<int, std::string>> r = {{1, "hello"}, {2, "world"}};

	auto q = r;

	// Create a JSON object
	json j;

	/// @note emplace a pair => array, pair.first and pair.second is NOT a key-value pair
	///				j["changes"] = 1; // This is a key-value pair
	///				j["changes"] = {"2","hello"}; // This is a key-array pair

	// Create a JSON array for the vector and assign it to "changes"
	json changes;
	for (const auto &item : r) {
		json pairJson;
		pairJson[std::to_string(item.first)] = item.second; // Using the first as a string key
		changes.emplace_back(pairJson);
	}
	j["changes"]	= changes; // Assign the array to the key "changes"
	j["changes2"] = r; // Assign the vector to the key "changes2"
	// j.emplace_back(q);
	// Print the JSON
	std::cout << j.dump(4) << std::endl;

	std::println("{}", nlohmann::json::meta().dump(2));
}
