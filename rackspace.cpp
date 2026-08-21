#include "rackspace.hpp"
#include "remote.hpp"
#include <iomanip>
#include <fstream>
#include <sstream>
#include <fmt/core.h>

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

int get_rackspace_access_information(rackspaceconfig::config *conf, nlohmann::json *access_info, char *error) {
	std::ifstream access_file_handler{};
	access_file_handler.open(rackspaceconfig::rackspace_access_filename);
	bool is_expired = true;
	int is_expired_tries = 3;
	while (is_expired && is_expired_tries-- > 0) {
		int tries = 3;
		while (!access_file_handler.good() && tries-- > 0) {
			std::string response;
			if (authenticate_to_rackspace_cloud(conf, response, error) > 0) {
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

		*access_info = nlohmann::json::parse(access_file_handler);
		access_file_handler.close();

		std::string expires = (*access_info)["access"]["token"]["expires"].get<std::string>();

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

int get_rackspace_cloudfiles_info(nlohmann::json *access_info, rackspaceconfig::cloudfiles_info *cloudfiles_info,
								  char *error) {
	cloudfiles_info->access_token = (*access_info)["access"]["token"]["id"].get<std::string>();

	auto serviceCatalog = (*access_info)["access"]["serviceCatalog"].get<std::vector<nlohmann::json>>();

	for (const auto &service : serviceCatalog) {
		if (service["name"] == "cloudFiles") {
			for (const auto &endpoint : service["endpoints"]) {
				if (endpoint["region"] == "ORD") {
					cloudfiles_info->region = endpoint["region"].get<std::string>();
					cloudfiles_info->public_url = endpoint["publicURL"].get<std::string>();
					return 0;
				}
			}
		}
	}
	return 0;
}
