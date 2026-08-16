#include <fmt/core.h>
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include "compiled_with.h"
#include "git_hash.h"
#include "remote.hpp"
#include "config.hpp"
extern const char *build_date;

int main() {
	fmt::println("Hello, World!");
	fmt::println("Compiled With: {}", COMPILED_WITH);
	fmt::println("Git: {} {}", GIT_REV, GIT_BRANCH);
	fmt::println("Build Time: {}", build_date);

	char error[1024] = { 0 };

	std::ifstream f("config.json");
	json data = json::parse(f);
	rackspaceconfig::config conf = data.get<rackspaceconfig::config>();

	fmt::println("CONFIG: username {}; api_key {}", conf.get_username(), conf.get_api_key());

	// return authenticate_to_rackspace_cloud(&conf,error);
	return 0;
}
