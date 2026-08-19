#include <fmt/core.h>
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include "compiled_with.h"
#include "git_hash.h"
#include "remote.hpp"
#include "config.hpp"
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
extern const char *build_date;
extern const char *rackspace_access_filename;

struct ParsedTime {
	std::tm tm{};
	int milliseconds = 0;
};

static ParsedTime parseIso8601(const std::string &s) {
	ParsedTime result;
	std::istringstream ss(s);

	ss >> std::get_time(&result.tm, "%Y-%m-%dT%H:%M:%S");
	if (ss.fail()) {
		throw std::runtime_error("Failed to parse timestamp: " + s);
	}

	// Next char should be '.' if milliseconds are present
	if (ss.peek() == '.') {
		ss.ignore(); // skip '.'
		std::string msStr;
		while (std::isdigit(ss.peek())) {
			msStr += static_cast<char>(ss.get());
		}
		if (!msStr.empty()) {
			// Pad/truncate to 3 digits in case of different precision
			msStr.resize(3, '0');
			result.milliseconds = std::stoi(msStr);
		}
	}

	// Optional trailing 'Z' — just consumed, since we assume UTC
	return result;
}

int main() {
	fmt::println("Hello, World!");
	fmt::println("Compiled With: {}", COMPILED_WITH);
	fmt::println("Git: {} {}", GIT_REV, GIT_BRANCH);
	fmt::println("Build Time: {}", build_date);

	char error[1024] = { 0 };

	std::ifstream config_file_handler(rackspaceconfig::rackspace_config_filename);
	if (!config_file_handler.good()) {
		fmt::println("Could not open {}", rackspaceconfig::rackspace_config_filename);
		return 1;
	}
	json data = json::parse(config_file_handler);
	config_file_handler.close();
	rackspaceconfig::config conf = data.get<rackspaceconfig::config>();

	fmt::println("CONFIG: username {}; api_key {}", conf.get_username(), conf.get_api_key());

	std::ifstream access_file_handler;
	access_file_handler.open(rackspaceconfig::rackspace_access_filename);
	bool is_expired = true;
	int is_expired_tries = 3;
	while (is_expired && is_expired_tries-- > 0) {
		int tries = 3;
		while (!access_file_handler.good() && tries-- > 0) {
			std::string response;
			if (authenticate_to_rackspace_cloud(&conf, response, error) > 0) {
				fmt::println("Erro chamando authenticate_to_rackspace_cloud: {}", error);
				return 1;
			}

			std::ofstream access_filename(rackspaceconfig::rackspace_access_filename);
			access_filename << response;
			access_filename.close();
			access_file_handler.open(rackspaceconfig::rackspace_access_filename);
		}

		if (!access_file_handler.good()) {
			fmt::println("Giving up on file {}", rackspaceconfig::rackspace_access_filename);
			return 1;
		}

		json access_json_data = json::parse(access_file_handler);
		access_file_handler.close();

		std::string access_token = access_json_data["access"]["token"]["id"].get<std::string>();
		std::string expires = access_json_data["access"]["token"]["expires"].get<std::string>();

		ParsedTime pt = parseIso8601(expires);

#if defined(_WIN32)
		std::time_t expiresTime = _mkgmtime(&pt.tm);
#else
		std::time_t expiresTime = timegm(&pt.tm);
#endif
		std::time_t now = std::time(nullptr);

		if (now > expiresTime) {
			fmt::println("The token is expired. Must revalidate!");
			std::remove(rackspaceconfig::rackspace_access_filename);
		} else {
			is_expired = false;
		}
	}
	return 0;
}
