#include <cstdlib> 
#include <sstream>
#include <iostream>
#include <string>
#include <filesystem>

std::string searchExecutable(std::string_view exec) {
	const char* path = std::getenv("PATH");

	if (path == nullptr) {
		return "";
	}

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

int main() {
	// Flush after every std::cout / std:cerr
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	for (;;) {
		std::cout << "$ ";

		std::string buf{};
		std::getline(std::cin, buf);

		const std::string_view line{ buf };
		const auto space = line.find(' ');
		const std::string_view command = line.substr(0, space);

		std::string_view args{};
		if (space == std::string_view::npos) {
			args = std::string_view{};
		}
		else {
			args = line.substr(space + 1);
		}

		if (command == "exit") {
			return 0;
		}
		else if (command == "echo") {
			std::cout << args << '\n';
		}
		else if (command == "type") {
			if (args.find(' ') != std::string_view::npos){
				std::cout << command << ": " << "command not found\n";
			}
			else if (args == "echo" || args == "exit" || args == "type") 
				 {
				std::cout << args << " is a shell builtin\n";
			}
			else {
				std::string path = searchExecutable(args);
				if (path == "") {
					std::cout << args << "is" << "not found\n";
				}
				else {
					std::cout << args << " is " << path << '\n';
				}
				
			}
		}
		else {
			std::cout << buf << ": " << "command not found\n";

		}
	}
}
