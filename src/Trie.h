#pragma once
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class Trie {
public:
	void insert(std::string_view word);
	std::vector<std::string> withPrefix(std::string_view prefix) const;
private:
	struct Node {
		std::map<char, std::unique_ptr<Node>> children;
		bool isWord{ false };
	};
	Node root;
	static void collect(const Node* node, std::string& word, std::vector<std::string>& out);
};
