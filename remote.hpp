#ifndef _REMOTE_HPP_
#define _REMOTE_HPP_
#include "config.hpp"

int authenticate_to_rackspace_cloud(rackspaceconfig::config *config, std::string &response, char *errorstring);
int get_container_list_of_files(rackspaceconfig::cloudfiles_info *cloudfiles_info, std::string &container,
								std::string &response, char *errorstring);

int remote_download_file(rackspaceconfig::cloudfiles_info *cloudfiles_info, std::string &container,
						 std::string &filename, char *destination_folder, size_t order, char *errorstring);

int remote_delete_file(rackspaceconfig::cloudfiles_info *cloudfiles_info, std::string &container, std::string &filename,
					   size_t order, char *errorstring);

#endif
