#pragma once
#ifndef _RACKSPACE_HPP_
#define _RACKSPACE_HPP_
#include "config.hpp"
#include <nlohmann/json.hpp>
int get_rackspace_access_information(rackspaceconfig::config *conf, nlohmann::json *access_info, char *error);

#endif
