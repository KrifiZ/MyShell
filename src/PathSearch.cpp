#include <cstdlib> 
#include <sstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <cstring>
#include "PathSearch.h"

namespace {
	std::string findExeByPath(const char* path, std::string_view exec) {
		std::istringstream in{ path };
		std::string dir{};

		while (std::getline(in, dir, ':')) {
			std::filesystem::path candidate = std::filesystem::path{ dir } / exec;
			std::error_code ec;
			if (std::filesystem::is_regular_file(candidate, ec)) {
				auto perms = std::filesystem::status(candidate, ec).permissions();
				if ((perms & std::filesystem::perms::owner_exec) != std::filesystem::perms::none) {
					return candidate.string();
				}
			}
		}
		return "";
	}
}

namespace path {
	std::string searchExecutable(std::string_view exec) {
		const char* path = std::getenv("PATH");

		if (path == nullptr) {
			return "";
		}

		const auto result = findExeByPath(path, exec);

		return result;
	}
}

namespace path {
	std::vector<std::string> listExecutables() {
		std::vector<std::string> out{};
		const char* path = getenv("PATH");

		if (path == nullptr) {
			return out;
		}

		std::istringstream in{ path };
		std::string dir{};

		while (std::getline(in, dir, ':')) {
			if (!std::filesystem::is_directory(dir)) {
				continue;
			}
			std::error_code ec;

			for (auto const& entry : std::filesystem::directory_iterator(dir, ec)) {
				if (ec) {
					continue;
				}
				if (std::filesystem::is_regular_file(entry, ec)) {
					auto perms = std::filesystem::status(entry, ec).permissions();
					if ((perms & std::filesystem::perms::owner_exec) != std::filesystem::perms::none) {
						out.push_back(entry.path().filename().string());
					}
				}
			}
		}
		return out;
	}
}
