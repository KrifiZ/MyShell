#include "Shell.h"
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
	argv.assign(std::make_move_iterator(tokens.begin() + 1),
		std::make_move_iterator(tokens.end()));
};

Shell::Shell() {
	builtins_["echo"] = [this](const Command& c) {builtinEcho(c);};
	builtins_["exit"] = [this](const Command& c) {builtinExit(c);};
	builtins_["type"] = [this](const Command& c) {builtinType(c);};
	builtins_["pwd"] = [this](const Command& c) {builtinPwd(c);};
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
	if (auto it = builtins_.find(cmd.name); it != builtins_.end()) {
		it->second(cmd);
	}
	else {
		runExternal(cmd);
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