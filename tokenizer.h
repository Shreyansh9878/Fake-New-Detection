#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

class TrieNode {
public:
    std::unordered_map<char, TrieNode*> children;
    bool isEndOfWord;

    TrieNode() : isEndOfWord(false) {}
};

class Trie {
private:
    TrieNode* root;

    void deleteTrie(TrieNode* node) {
        if (!node) return;
        for (auto& pair : node->children) {
            deleteTrie(pair.second);
        }
        delete node;
    }

public:
    Trie() {
        root = new TrieNode();
    }

    ~Trie() {
        deleteTrie(root);
    }

    void insert(const std::string& word) {
        TrieNode* current = root;
        for (char c : word) {
            char lowerC = std::tolower(c); // Convert to lowercase
            if (!current->children[lowerC]) {
                current->children[lowerC] = new TrieNode();
            }
            current = current->children[lowerC];
        }
        current->isEndOfWord = true;
    }

    bool search(const std::string& word) {
        TrieNode* current = root;
        for (char c : word) {
            char lowerC = std::tolower(c); // Convert to lowercase
            if (!current->children.count(lowerC))
                return false;
            current = current->children[lowerC];
        }
        return current->isEndOfWord;
    }

    std::vector<std::string> tokenizeAndInsert(const std::string& text) {
        std::vector<std::string> tokens;
        std::stringstream ss(text);
        std::string word;
        while (ss >> word) {
            // Remove punctuation from the beginning and end of the word
            word.erase(word.begin(), std::find_if(word.begin(), word.end(), [](unsigned char ch)) {
                return !std::ispunct(ch);
            });
            word.erase(std::find_if(word.rbegin(), word.rend(), [](unsigned char ch) {
                return !std::ispunct(ch);
            }).base(), word.end());

            if (!word.empty()) {
                insert(word);
                tokens.push_back(word);
            }
        }
        return tokens;
    }
};

#endif
