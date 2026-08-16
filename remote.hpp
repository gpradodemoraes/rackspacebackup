#ifndef _REMOTE_HPP_
#define _REMOTE_HPP_
#include "config.hpp"

int authenticate_to_rackspace_cloud(rackspaceconfig::config *config, std::string &response, char *errorstring);
#endif
