#pragma once
#ifndef _RACKSPACE_HPP_
#define _RACKSPACE_HPP_
#include "config.hpp"
#include <nlohmann/json.hpp>
int get_rackspace_access_information(rackspaceconfig::config *conf, nlohmann::json *access_info, char *error);
int get_rackspace_cloudfiles_info(nlohmann::json *access_info, rackspaceconfig::cloudfiles_info *cloudfiles_info,
								  std::string region, char *error);
int get_rackspace_container_list_of_files(rackspaceconfig::cloudfiles_info *cloudfiles_info, std::string &container,
										  std::vector<std::string> *file_list, char *errorstring);

#endif
