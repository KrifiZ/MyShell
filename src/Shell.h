#pragma once
#include <functional>
#include <map>
#include <unordered_map>
#include <string_view>
#include <string>
#include <optional>

struct Command {
	std::string name;
	std::vector<std::string> argv;
	std::optional<std::string> stdoutFile;
	std::optional<std::string> stderrFile;
	explicit Command(std::string_view line);
};

std::vector<std::string> splitArgs(std::string_view sv);



class Shell {
public:
	Shell();
	void run();
private:
	using Handler = std::function<void(const Command&)>;

	void execute(const Command& cmd);
	void runExternal(const Command& cmd);
	bool isBuiltin(std::string_view name) const;

	void builtinEcho(const Command& cmd);
	void builtinExit(const Command& cmd);
	void builtinType(const Command& cmd);
	void builtinPwd(const Command& cmd);
	void builtinCd(const Command& cmd);

	std::unordered_map<std::string, Handler> builtins_;
	bool running_ = true;
};
