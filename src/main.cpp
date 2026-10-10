#include <cstdlib> 
#include <sstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

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



std::vector<char*> retrieveArgs(std::string_view sv) {
	std::vector<char*> args{};
	std::size_t start = 0;
	while (start < sv.size()) {
		if (sv[start] == ' ') {
			start++;
			continue;
		}

		size_t end = sv.find(' ', start);
		if (end == std::string_view::npos) {
			end = sv.size();
		}
		args.emplace_back(sv.substr(), end - start);
		start = end;
	}
	return args;
}

std::vector<std::string> splitArgs(std::string_view sv) {
	std::vector<std::string> out{};
	std::istringstream in{ std::string(sv) };
	std::string tok{};
	while (in >> tok) out.push_back(tok);
	return out;
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
			if (args.find(' ') != std::string_view::npos) {
				std::cout << command << ": " << "command not found\n";
			}
			else if (args == "echo" || args == "exit" || args == "type")
			{
				std::cout << args << " is a shell builtin\n";
			}
			else {
				std::string path = searchExecutable(args);
				if (path == "") {
					std::cout << args << ": " << "not found\n";
				}
				else {
					std::cout << args << " is " << path << '\n';
				}

			}
		}
		else {

			std::string exec = searchExecutable(command);
			if (exec.empty()) {
				std::cout << command << ": command not found\n";
				continue;
			}
			pid_t pid = fork();
			if (pid < 0) {
				perror("fork failed");
				return 1;
			}
			if (pid == 0) {
				std::vector<std::string> tokens = splitArgs(line);
				std::vector<char*> argv{};
				for (auto& t : tokens) argv.push_back(t.data());
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
#include <cstdlib> 
#include <sstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

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



std::vector<char*> retrieveArgs(std::string_view sv) {
	std::vector<char*> args{};
	std::size_t start = 0;
	while (start < sv.size()) {
		if (sv[start] == ' ') {
			start++;
			continue;
		}

		size_t end = sv.find(' ', start);
		if (end == std::string_view::npos) {
			end = sv.size();
		}
		args.emplace_back(sv.substr(), end - start);
		start = end;
	}
	return args;
}

std::vector<std::string> splitArgs(std::string_view sv) {
	std::vector<std::string> out{};
	std::istringstream in{ std::string(sv) };
	std::string tok{};
	while (in >> tok) out.push_back(tok);
	return out;
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
			if (args.find(' ') != std::string_view::npos) {
				std::cout << command << ": " << "command not found\n";
			}
			else if (args == "echo" || args == "exit" || args == "type")
			{
				std::cout << args << " is a shell builtin\n";
			}
			else {
				std::string path = searchExecutable(args);
				if (path == "") {
					std::cout << args << ": " << "not found\n";
				}
				else {
					std::cout << args << " is " << path << '\n';
				}

			}
		}
		else {
			std::string exec = searchExecutable(command);
			if (exec.empty()) {
				std::cout << command << ": command not found\n";
				continue;
			}
			pid_t pid = fork();
			if (pid < 0) {
				perror("fork failed");
				return 1;
			}
			if (pid == 0) {
				std::vector<std::string> tokens = splitArgs(line);
				std::vector<char*> argv{};
				for (auto& t : tokens) argv.push_back(t.data());
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
