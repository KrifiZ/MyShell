#include "Trie.h"

void Trie::insert(std::string_view word) {
	Node* cur = &root;
	for (char c : word) {
		auto& child = cur->children[c];
		if (!child) child = std::make_unique<Node>();
		cur = child.get();
	}
	cur->isWord = true;
}

std::vector<std::string> Trie::withPrefix(std::string_view prefix) const {
	const Node* cur = &root;
	for (char c : prefix) {
		auto it = cur->children.find(c);
		if (it == cur->children.end()) return {};
		cur = it->second.get();
	}
	std::vector<std::string> out;
	std::string word{ prefix };
	collect(cur, word, out);
	return out;
}

void Trie::collect(const Node* node, std::string& word, std::vector<std::string>& out) {
	if (node->isWord) out.push_back(word);
	for (const auto& [c, child] : node->children) {
		word.push_back(c);
		collect(child.get(), word, out);
		word.pop_back();
	}
}
