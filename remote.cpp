#include <curl/curl.h>
#include <fmt/core.h>
#include <iostream>
#include "remote.hpp"

// State struct holding the file handle
struct ReadState {
	FILE *file;
};

// Read callback — curl calls this to pull chunks of data
static size_t read_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
	ReadState *state = static_cast<ReadState *>(userdata);

	if (!state->file) return CURL_READFUNC_ABORT;

	size_t bytes_read = fread(buffer, size, nitems, state->file);

	if (bytes_read == 0 && ferror(state->file)) {
		std::cerr << "Error reading file\n";
		return CURL_READFUNC_ABORT;
	}

	return bytes_read;
}

// Seek callback — curl calls this to rewind/seek the data source (e.g. on redirect or retry)
static int seek_callback(void *userdata, curl_off_t offset, int origin) {
	ReadState *state = static_cast<ReadState *>(userdata);

	if (!state->file) return CURL_SEEKFUNC_FAIL;

	// Map curl's origin values to fseek's whence values
	int whence;
	switch (origin) {
		case SEEK_SET: whence = SEEK_SET; break;
		case SEEK_CUR: whence = SEEK_CUR; break;
		case SEEK_END: whence = SEEK_END; break;
		default: return CURL_SEEKFUNC_FAIL;
	}

#ifdef _WIN32
	if (_fseeki64(state->file, (__int64)offset, whence) != 0)
#else
	if (fseeko(state->file, (off_t)offset, whence) != 0)
#endif
	{
		std::cerr << "Seek failed\n";
		return CURL_SEEKFUNC_FAIL;
	}

	return CURL_SEEKFUNC_OK;
}

// Free callback — curl calls this when the mime part is done
static void free_callback(void *userdata) {
	ReadState *state = static_cast<ReadState *>(userdata);

	if (state->file) {
		fclose(state->file);
		state->file = nullptr;
	}

	delete state;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *output) {
	size_t totalSize = size * nmemb;
	output->append((char *)contents, totalSize);
	return totalSize;
}

int authenticate_to_rackspace_cloud(rackspaceconfig::config *config, std::string &response, char *errorstring) {
	int retval = 0;
	std::string authenticate_json(rackspaceconfig::authenticate_json_char);
	std::string yourUserName("yourUserName");
	std::string yourApiKey("yourApiKey");
	size_t start_pos = authenticate_json.find(yourUserName);
	if (start_pos == std::string::npos) {
		strcpy_s(errorstring, 1024, "erro criando authenticate_json para yourUserName");
		return 1;
	}
	authenticate_json.replace(start_pos, yourUserName.length(), config->get_username());

	start_pos = authenticate_json.find(yourApiKey);
	if (start_pos == std::string::npos) {
		strcpy_s(errorstring, 1024, "erro criando authenticate_json para yourApiKey");
		return 1;
	}
	authenticate_json.replace(start_pos, yourApiKey.length(), config->get_api_key());

	CURL *curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, rackspaceconfig::identity_url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		struct curl_slist *headers = nullptr;
		headers = curl_slist_append(headers, "Content-type: application/json; charset=UTF-8");

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, authenticate_json.c_str());

		CURLcode res = curl_easy_perform(curl);
		long httpCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

		if (res == CURLE_OK && httpCode == 200 && response.size() > 0) {
			// we are in business!
		} else if (res != CURLE_OK) {
			strcpy_s(errorstring, 1024, curl_easy_strerror(res));
			retval = 1;
		}

		fmt::println("HTTP Status: {}", httpCode);
		fmt::println("RESPONSE: {}", response);

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);

	} else {
		strcpy_s(errorstring, 1024, "não foi possível criar o CURL");
		retval = 1;
	}
	return retval;
}

int get_container_list_of_files(rackspaceconfig::cloudfiles_info *cloudfiles_info, std::string &container,
								std::string &response, char *errorstring) {
	int retval = 0;
	std::string container_url = cloudfiles_info->public_url + "/" + container;

	CURL *curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, container_url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		struct curl_slist *headers = nullptr;
		std::string token = fmt::format("X-Auth-Token: {}", cloudfiles_info->access_token);
		headers = curl_slist_append(headers, token.c_str());

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		CURLcode res = curl_easy_perform(curl);
		long httpCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

		if (res == CURLE_OK && httpCode == 200 && response.size() > 0) {
			// we are in business!
		} else if (res != CURLE_OK) {
			strcpy_s(errorstring, 1024, curl_easy_strerror(res));
			retval = 1;
		}

		fmt::println("HTTP Status: {}", httpCode);
		fmt::println("RESPONSE: {}", response);

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);

	} else {
		strcpy_s(errorstring, 1024, "não foi possível criar o CURL");
		retval = 1;
	}
	return retval;
}
