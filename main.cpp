#include <fmt/core.h>
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
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

	std::ifstream config_file_handler(rackspaceconfig::rackspace_config_filename);
	if (!config_file_handler.good()) {
		fmt::println("Could not open {}", rackspaceconfig::rackspace_config_filename);
		return 1;
	}
	json data = json::parse(config_file_handler);
	config_file_handler.close();
	rackspaceconfig::config conf = data.get<rackspaceconfig::config>();

	fmt::println("CONFIG: username {}; api_key {}", conf.get_username(), conf.get_api_key());

	nlohmann::json access_info;
	if (get_rackspace_access_information(&conf, &access_info, error) > 0) {
		fmt::println("Erro chamando get_rackspace_access_information: {}", error);
		return 1;
	}

	return 0;
}
