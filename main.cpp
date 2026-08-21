#include <fmt/core.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>
#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>
#include "compiled_with.h"
#include "git_hash.h"
#include "config.hpp"
#include "rackspace.hpp"

extern const char *build_date;
extern const char *rackspace_access_filename;

int main() {
	fmt::println("Hello, World!");
	fmt::println("Compiled With: {}", COMPILED_WITH);
	fmt::println("Git: {} {}", GIT_REV, GIT_BRANCH);
	fmt::println("Build Time: {}", build_date);

	char error[1024] = { 0 };

	auto in_the_past = std::chrono::system_clock::now() - std::chrono::hours(24);
	std::time_t time = std::chrono::system_clock::to_time_t(in_the_past);

	std::tm tm_local;
#if defined(_WIN32)
	localtime_s(&tm_local, &time);
#else
	localtime_r(&time, &tm_local);
#endif

	std::ostringstream oss;
	oss << std::put_time(&tm_local, "%Y-%m-%d-%H-%M-%S");
	std::string twenty_four_hours_ago = oss.str();

	std::ifstream config_file_handler(rackspaceconfig::rackspace_config_filename);
	if (!config_file_handler.good()) {
		fmt::println("Could not open {}", rackspaceconfig::rackspace_config_filename);
		return 1;
	}
	nlohmann::json data = nlohmann::json::parse(config_file_handler);
	config_file_handler.close();
	rackspaceconfig::config conf = data.get<rackspaceconfig::config>();

	fmt::println("CONFIG: username {}; api_key {}", conf.get_username(), conf.get_api_key());

	nlohmann::json access_info;
	if (get_rackspace_access_information(&conf, &access_info, error) > 0) {
		fmt::println("Erro chamando get_rackspace_access_information: {}", error);
		return 1;
	}
	rackspaceconfig::cloudfiles_info cloudfiles_info;
	if (get_rackspace_cloudfiles_info(&access_info, &cloudfiles_info, "ORD", error) > 0) {
		fmt::println("Erro chamando get_rackspace_cloudfiles_info: {}", error);
		return 1;
	}
	std::vector<std::string> file_list;
	if (get_rackspace_container_list_of_files(&cloudfiles_info, std::string("sqlitebackup"), &file_list, error) > 0) {
		fmt::println("Erro chamando get_container_list_of_files: {}", error);
		return 1;
	}

	std::regex get_file_date(R"(^([^.]*)\.([^.]*))");
	for (const auto &file : file_list) {
		std::smatch match;
		if (std::regex_search(file, match, get_file_date)) {
			if (match[2].str() < twenty_four_hours_ago) {
				fmt::println("we will download and then delete this file: {}<==>{}", match[1].str(), match[2].str());
			}
		}
	}

	return 0;
}
