#include <fmt/core.h>
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include "compiled_with.h"
#include "git_hash.h"
#include "remote.hpp"
#include "config.hpp"
#include <string>
extern const char *build_date;
extern const char *rackspace_access_filename;

int main() {
	fmt::println("Hello, World!");
	fmt::println("Compiled With: {}", COMPILED_WITH);
	fmt::println("Git: {} {}", GIT_REV, GIT_BRANCH);
	fmt::println("Build Time: {}", build_date);

	char error[1024] = { 0 };

	std::ifstream f(rackspaceconfig::rackspace_config_filename);
	if (!f.good()) {
		fmt::println("Could not open {}", rackspaceconfig::rackspace_config_filename);
		return 1;
	}
	json data = json::parse(f);
	f.close();
	rackspaceconfig::config conf = data.get<rackspaceconfig::config>();

	fmt::println("CONFIG: username {}; api_key {}", conf.get_username(), conf.get_api_key());

	f.open(rackspaceconfig::rackspace_access_filename);
	int tries = 3;
	while (!f.good() && tries-- > 0) {
		std::string response;
		if (authenticate_to_rackspace_cloud(&conf, response, error) > 0) {
			fmt::println("Erro chamando authenticate_to_rackspace_cloud: {}", error);
			return 1;
		}

		std::ofstream access_filename(rackspaceconfig::rackspace_access_filename);
		access_filename << response;
		access_filename.close();
		f.open(rackspaceconfig::rackspace_access_filename);
	}

	if (!f.good()) {
		fmt::println("Giving up on file {}", rackspaceconfig::rackspace_access_filename);
		return 1;
	}

	f.close();
	return 0;
}
