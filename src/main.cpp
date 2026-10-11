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

std::string searchExecutable(std::string_view exec) {
	const char* path = std::getenv("PATH");

	if (path == nullptr) {
		return "";
	}

	const auto result = findExeByPath(path, exec);

	return result;
}

std::vector<std::string> splitArgs(std::string_view sv) {
	std::vector<std::string> out{};
	std::istringstream in{ std::string(sv) };
	std::string tok{};
	while (in >> tok) out.push_back(tok);
	return out;
}

struct Command {
	std::string name;
	std::vector<std::string> argv;
	explicit Command(std::string_view line) {
		std::vector<std::string> tokens = splitArgs(line);
		if (tokens.empty()) return;

		name = std::move(tokens.front());
		argv.assign(std::make_move_iterator(tokens.begin() + 1),
			std::make_move_iterator(tokens.end()));
	}
};

int main() {
	// Flush after every std::cout / std:cerr
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	for (;;) {
		std::cout << "$ ";

		std::string buf{};
		std::getline(std::cin, buf);

		Command command{ buf };

		if (command.name.empty()) {
			continue;  
		}

		if (command.name == "exit") {
			return 0;
		}
		else if (command.name == "echo") {
			for (std::size_t i = 0; i < command.argv.size(); ++i) {
				if (i > 0) std::cout << ' ';
				std::cout << command.argv[i];
			}
			std::cout << '\n';
		}
		else if (command.name == "type") {
			for (const auto& arg : command.argv) {
				if (arg == "echo" || arg == "exit" || arg == "type") {
					std::cout << arg << " is a shell builtin\n";
					continue;
				}

				std::string path = searchExecutable(arg);
				if (path.empty()) {
					std::cout << arg << ": not found\n";
				}
				else {
					std::cout << arg << " is " << path << '\n';
				}
			}
		}
		else {

			std::string exec = searchExecutable(command.name);
			if (exec.empty()) {
				std::cout << command.name << ": command not found\n";
				continue;
			}
			pid_t pid = fork();
			if (pid < 0) {
				perror("fork failed");
				return 1;
			}
			if (pid == 0) {
				std::vector<char*> argv{};
				argv.push_back(command.name.data());
				for (auto& a : command.argv) argv.push_back(a.data());
				argv.push_back(nullptr);

				execv(exec.c_str(), argv.data());
				perror("execv failed");
				_exit(1);
			}
			else {
				int status;
				waitpid(pid, &status, 0);
			}
		}
	}
}