#include <iostream>
#include <string>

int main() {
	// Flush after every std::cout / std:cerr
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	for (;;) {
		std::cout << "$ ";

		std::string buf{};
		std::cin >> buf;

		if (buf == "exit") {
			return 0;
		}
		else if (buf == "echo") {
			std::cout << buf << '\n';
		}
		else {
			std::cout << buf << ": " << "command not found\n";

		}
	}
}
