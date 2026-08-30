#include <fmt/core.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>
#include <chrono>
#include <future>
#include <string>
#include <iomanip>
#include <sstream>
#include <memory>
#include <charconv>
#include "compiled_with.h"
#include "git_hash.h"
#include "config.hpp"
#include "rackspace.hpp"
#include "getopt.h"
#include <utility>
#include <string.h>

extern const char *build_date;
extern const char *rackspace_access_filename;

int main(int argc, char *argv[]) {
	fmt::println("Compiled With: {}", COMPILED_WITH);
	fmt::println("Git: {} {}", GIT_REV, GIT_BRANCH);
	fmt::println("Build Time: {}", build_date);

	int opt;
	int longindex;
	char error[1024] = { 0 };

	char dest_folder[2048] = { 0 };
	char container[1024] = { 0 };
	size_t max_files = 5;

	constexpr option rackspace_options[] = { { "dest_folder", required_argument, nullptr, 'd' },
											 { "container", required_argument, nullptr, 'c' },
											 { "max_files", required_argument, nullptr, 'm' },
											 { "help", required_argument, nullptr, 'h' },
											 { nullptr, 0 } };

	while ((opt = getopt_long(argc, argv, "d:c:m:h:", rackspace_options, &longindex)) != -1) {
		switch (opt) {
			case 'd': std::strcpy(dest_folder, optarg); break;
			case 'c': std::strcpy(container, optarg); break;
			case 'm': std::from_chars(optarg, optarg + strlen(optarg), max_files); break;
		}
	}

	fmt::println("OPTIONS:");
	fmt::println("dest_folder: {}", dest_folder);
	fmt::println("container:   {}", container);
	fmt::println("max_files:   {}", max_files);

	struct stat sb;

	if (stat(dest_folder, &sb) != 0) {
		fmt::println("Folder {} does not exist", dest_folder);
		return 1;
	}

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

	std::vector<std::string> the_good_files_list;
	std::regex get_file_date(R"(^([^.]*)\.([^.]*))");
	for (const auto &file : file_list) {
		std::smatch match;
		if (std::regex_search(file, match, get_file_date)) {
			if (match[2].str() < twenty_four_hours_ago) {
				the_good_files_list.push_back(file);
				// fmt::println("we will download and then delete this file: {}<==>{}", match[1].str(), match[2].str());
			}
		}
	}

	std::vector<std::future<std::pair<int, std::unique_ptr<char[]>>>> my_futures;

	size_t counter = 0;
	for (auto const &filename : the_good_files_list) {
		counter++;
		fmt::println("                  REQUESTING {:03} {}", counter, filename);
		my_futures.push_back(std::async(
			std::launch::async,
			[&](rackspaceconfig::cloudfiles_info *cloudfiles_info, std::string &container, std::string &filename,
				char *destination_folder, size_t order) -> std::pair<int, std::unique_ptr<char[]>> {
				std::unique_ptr<char[]> my_error = std::make_unique<char[]>(1024);
				strcpy_s(my_error.get(), 1024, filename.c_str());
				strcat_s(my_error.get(), 1024 - strlen(my_error.get()), " OK!");
				return { download_and_delete_rackspace_file(cloudfiles_info, container, filename, (char *)dest_folder,
															order, my_error.get()),
						 std::move(my_error) };
			},
			&cloudfiles_info, std::string(container), (std::string &)filename, (char *)dest_folder, counter));
		if (counter >= max_files) break;
	}

	for (auto &f : my_futures) {
		std::pair<int, std::unique_ptr<char[]>> result = f.get();
		fmt::println("[{}] {}", result.first, result.second.get());
	}

	return 0;
}
