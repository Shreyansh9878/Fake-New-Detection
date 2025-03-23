#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <sstream>

class TrieNode {
public:
    std::unordered_map<char, TrieNode*> children;
    bool isEndOfWord;

    TrieNode() : isEndOfWord(false) {}
};

class Trie {
public:
    Trie() {
        root = new TrieNode();
        // Insert all stop words into the Trie
        for (const std::string& word : stopWords) {
            insert(word);
        }
    }

    void insert(const std::string& word) {
        TrieNode* node = root;
        for (char ch : word) {
            if (node->children.find(ch) == node->children.end()) {
                node->children[ch] = new TrieNode();
            }
            node = node->children[ch];
        }
        node->isEndOfWord = true;
    }

    bool search(const std::string& word) {
        TrieNode* node = root;
        for (char ch : word) {
            if (node->children.find(ch) == node->children.end()) {
                return false;
            }
            node = node->children[ch];
        }
        return node != nullptr && node->isEndOfWord;
    }

    // List of stop words
    static const std::vector<std::string> stopWords;

private:
    TrieNode* root;
};

// Define the list of stop words
const std::vector<std::string> Trie::stopWords = {
    "do", "the", "so", "at", "and", "when", "did", "they", "in", "does", "seem", 
    "beyond", "upon", "we", "ask", "about", "us", "such", "a", "of", "its", "for", 
    "will", "it"
};

std::vector<std::string> tokenize(const std::string& text, Trie& stopWordsTrie) {
    std::vector<std::string> tokens;
    std::stringstream ss(text);
    std::string token;
    std::unordered_set<std::string> uniqueTokens;

    while (ss >> token) {
        // Remove punctuation
        token.erase(std::remove_if(token.begin(), token.end(), ::ispunct), token.end());
        // Convert to lowercase
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);

        // Check if the token is a stop word or already added
        if (!stopWordsTrie.search(token) && uniqueTokens.find(token) == uniqueTokens.end()) {
            tokens.push_back(token);
            uniqueTokens.insert(token);
        }
    }

    return tokens;
}

#endif // TOKENIZER_H
