#ifndef _CONFIG_HPP_
#define _CONFIG_HPP_

#include <string>
#include <nlohmann/json.hpp>
#include "nlohmann/detail/macro_scope.hpp"

namespace rackspaceconfig {
class config {
   private:
	std::string username;
	std::string api_key;

   public:
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(config, username, api_key)

	char *get_username() { return (char *)username.c_str(); }

	char *get_api_key() { return (char *)api_key.c_str(); }
};

inline constexpr const char *identity_url = "https://identity.api.rackspacecloud.com/v2.0/tokens";

inline constexpr const char *authenticate_json_char =
	R"({"auth":{"RAX-KSKEY:apiKeyCredentials":{"username":"yourUserName","apiKey":"yourApiKey"}}})";

inline constexpr const char *rackspace_access_filename = "rackspace_access.json";
inline constexpr const char *rackspace_config_filename = "config.json";
} // namespace rackspaceconfig
#endif
