#include <cstddef>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>

namespace net::ancillarycat::waver {
struct Signal {
  std::string type;
  size_t width;
  std::string identifier;
  std::string name;
};

struct ValueChange {
  std::string identifier;
  std::string value;
};

class Parser {
public:
  void parse(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Unable to open file: " << filename << std::endl;
      return;
    }

    std::string line;
    std::stack<std::string> module_stack;
    while (std::getline(file, line)) {
      line = trim(line);
      std::istringstream iss(line);
      std::string temp;
      iss >> temp;

      if (temp == "$scope") {
        std::string module_type, module_name;
        iss >> module_type >> module_name >> temp;
        if (module_type == "module" && temp == "$end") {
          module_stack.push(module_name);
        }
      } else if (temp == "$upscope") {
        iss >> temp;
        if (temp == "$end" && !module_stack.empty()) {
          module_stack.pop();
        }
      } else if (temp == "$var") {
        if (!module_stack.empty()) {
          parse_variable(line, module_stack.top());
        }
      } else if (temp == "$enddefinitions") {
        // test, print the stack
				while (!module_stack.empty()) {
					std::cout << module_stack.top() << std::endl;
				}
				break;
      }
    }

    while (std::getline(file, line)) {
      line = trim(line);
      parse_value_change(line);
    }

    file.close();
  }

  void print_parsed_data() {
    std::cout << "Variables:" << std::endl;
    for (const auto &[module, vars] : module_variables) {
      std::cout << "Module: " << module << std::endl;
      for (const auto &[identifier, signal] : vars) {
        std::cout << "  " << identifier << ": " << signal.name << " ("
                  << signal.width << " bits)" << std::endl;
      }
    }

    std::cout << "\nSignal Values:" << std::endl;
    for (const auto &[time, changes] : signal_values) {
      std::cout << "Time: " << time << std::endl;
      for (const auto &[identifier, value] : changes) {
        std::cout << "  " << identifier << ": " << value << std::endl;
      }
    }
  }

  nlohmann::json to_json() {
    nlohmann::json json_data;

    // Convert variables and submodules to JSON
    for (const auto &[module, vars] : module_variables) {
      auto &module_object = json_data["modules"][module];
      for (const auto &[identifier, signal] : vars) {
        module_object["variables"][identifier] = {
            {"type", signal.type},
            {"size", std::to_string(signal.width)},
            {"reference", signal.name}};
      }
      // Add submodules to the module object
      for (const auto &submodule : module_submodules[module]) {
        module_object["submodules"].push_back(submodule);
      }
    }

    // Convert signal values to JSON
    for (const auto &[time, changes] : signal_values) {
      for (const auto &[identifier, value] : changes) {
        json_data["signal_values"][std::to_string(time)][identifier] = value;
      }
    }

    return json_data;
  }

private:
  std::unordered_map<std::string, std::unordered_map<std::string, Signal>>
      module_variables;
  std::map<int, std::unordered_map<std::string, std::string>> signal_values;
  std::unordered_map<std::string, std::vector<std::string>> module_submodules;
  int current_time = 0;

  std::string trim(const std::string &str) {
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos)
      return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, last - first + 1);
  }

  void parse_variable(const std::string &line, const std::string &module_name) {
    std::istringstream iss(line);
    std::string temp, type, identifier, name;
    long long maybe_width;
    iss >> temp >> type >> maybe_width >> identifier >> name;
    if (maybe_width < 0)
      maybe_width = std::numeric_limits<long long>::quiet_NaN();
    module_variables[module_name][identifier] = {
        type, static_cast<size_t>(maybe_width), identifier, name};
  }

  void parse_value_change(const std::string &line) {
    if (line[0] == '#') {
      current_time = std::stoi(line.substr(1));
    } else if (line[0] == 'b') {
      size_t space_pos = line.find(' ');
      std::string value = line.substr(1, space_pos - 1);
      std::string identifier = line.substr(space_pos + 1);
      signal_values[current_time][identifier] = value;
    } else if (line[0] == '0' || line[0] == '1' || line[0] == 'x' ||
               line[0] == 'z') {
      std::string value = line.substr(0, 1);
      std::string identifier = line.substr(1);
      signal_values[current_time][identifier] = value;
    }
  }
};

} // namespace net::ancillarycat::waver
