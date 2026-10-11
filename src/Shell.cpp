#include "Shell.h"
#include <cstdlib> 
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <cstring>
#include "PathSearch.h"

std::vector<std::string> splitArgs(std::string_view sv) {
	std::vector<std::string> out{};
	std::istringstream in{ std::string(sv) };
	std::string tok{};
	while (in >> tok) out.push_back(tok);
	return out;
}

Command::Command(std::string_view line) {
	std::vector<std::string> tokens = splitArgs(line);
	if (tokens.empty()) return;

	name = std::move(tokens.front());

	for (std::size_t i = 1; i < tokens.size(); ++i) {
		if (tokens[i] == ">" || tokens[i] == "1>") {
			if (i + 1 < tokens.size()) {
				stdoutFile = tokens[i + 1];
				++i;
			}
			continue;
		}
		if (tokens[i] == "2>") {
			if (i + 1 < tokens.size()) {
				stderrFile = tokens[i + 1];
				++i;
			}
			continue;
		}
		argv.push_back(tokens[i]);
	}
};

Shell::Shell() {
	builtins_["echo"] = [this](const Command& c) {builtinEcho(c);};
	builtins_["exit"] = [this](const Command& c) {builtinExit(c);};
	builtins_["type"] = [this](const Command& c) {builtinType(c);};
	builtins_["pwd"] = [this](const Command& c) {builtinPwd(c);};
	builtins_["cd"] = [this](const Command& c) {builtinCd(c);};
}

void Shell::run() {
	while(this->running_) {
		std::cout << "$ ";

		std::string buf{};
		std::getline(std::cin, buf);

		Command command{ buf };

		if (command.name.empty()) {
			continue;
		}

		execute(command);
	}
}

void Shell::execute(const Command& cmd) {
	std::ofstream file;
	std::streambuf* old = nullptr;
	std::ofstream file2;
	std::streambuf* old2 = nullptr;

	if (cmd.stdoutFile) {
		file.open(*cmd.stdoutFile);
		old = std::cout.rdbuf(file.rdbuf());
	}

	if (cmd.stderrFile) {
		file2.open(*cmd.stderrFile);
		old2 = std::cerr.rdbuf(file2.rdbuf());
	}

	if (auto it = builtins_.find(cmd.name); it != builtins_.end()) {
		it->second(cmd);
	}
	else {
		runExternal(cmd);
	}

	if (cmd.stdoutFile) {
		std::cout.rdbuf(old);
	}
	if (cmd.stderrFile) {
		std::cerr.rdbuf(old2);
	}
}

void Shell::runExternal(const Command& cmd) {
	std::string exec = path::searchExecutable(cmd.name);
	if (exec.empty()) {
		std::cout << cmd.name << ": command not found\n";
		return;
	}
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork failed");
		running_ = false;
	}
	if (pid == 0) {
		std::vector<char*> argv{};
		argv.push_back(const_cast<char*>(cmd.name.data()));
		for (auto& a : cmd.argv) argv.push_back(const_cast<char*>(a.data()));
		argv.push_back(nullptr);

		// std::cout.rdbuf swap doesn't reach the exec'd process, redirect fd 1 itself
		if (cmd.stdoutFile) {
			int fd = open(cmd.stdoutFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (fd >= 0) {
				dup2(fd, STDOUT_FILENO);
				close(fd);
			}
		}


		if (cmd.stderrFile) {
			int fd = open(cmd.stderrFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (fd >= 0) {
				dup2(fd, STDERR_FILENO);
				close(fd);
			}
		}

		execv(exec.c_str(), argv.data());
		perror("execv failed");
		running_ = false;
	}
	else {
		int status;
		waitpid(pid, &status, 0);
	}
}


bool Shell::isBuiltin(std::string_view name) const {
	auto it = this->builtins_.find(name.data());
	return it != builtins_.end();
}

void Shell::builtinEcho(const Command& cmd) {
	for (std::size_t i = 0; i < cmd.argv.size(); ++i) {
		if (i > 0) std::cout << ' ';
		std::cout << cmd.argv[i];
	}
	std::cout << '\n';
}

void Shell::builtinExit(const Command& cmd) {
	this->running_ = false;
}

void Shell::builtinType(const Command& cmd) {
	for (const auto& arg : cmd.argv) {
		if (auto it = builtins_.find(arg); it != builtins_.end()) {
			std::cout << arg << " is a shell builtin\n";
			continue;
		}

		std::string path = path::searchExecutable(arg);
		if (path.empty()) {
			std::cout << arg << ": not found\n";
		}
		else {
			std::cout << arg << " is " << path << '\n';
		}
	}
}

void Shell::builtinPwd(const Command& cmd) {
	std::cout << std::filesystem::current_path().string() << '\n';
}

void Shell::builtinCd(const Command& cmd) {
	namespace fs = std::filesystem;

	std::string target;
	if (cmd.argv.size() < 1 || cmd.argv[0] == "~") {
		const char* home = std::getenv("HOME");
		if (!home) {
			return;
		}
		target = home;
	}
	else {
		target = cmd.argv[0];
	}

	std::error_code ec;
	fs::current_path(target, ec);
	if (ec) {
		std::cout << "cd: " << target << ": No such file or directory\n";
	}
}