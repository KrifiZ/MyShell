#include <iostream>
#include <string>

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
			if ((args == "echo" || args == "exit" || args == "type") 
				&& args.find(' ') == std::string_view::npos) {
				std::cout << args << " is a shell builtin\n";
			}
			else {
				std::cout << args << ": " << "command not found\n";
			}
		}
		else {
			std::cout << buf << ": " << "command not found\n";

		}
	}
}
