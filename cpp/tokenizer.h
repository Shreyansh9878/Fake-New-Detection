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
        numWords = 0;

        // Insert all stop words and filler words into the Trie
        for (const std::string& word : removeWords) {
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
        if (!node->isEndOfWord) {
            node->isEndOfWord = true;
            numWords++;
        }
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

    int getNumWords() const {
        return numWords;
    }

    // List of stop words and filler words
    static const std::vector<std::string> removeWords;

private:
    TrieNode* root;
    int numWords;  // Track total number of words in Trie
};

// Define the list of stop words and filler words
const std::vector<std::string> Trie::removeWords = {
    "do", "the", "so", "at", "and", "when", "did", "they", "in", "does", "seem", 
    "beyond", "upon", "we", "ask", "about", "us", "such", "a", "of", "its", "for", 
    "will", "it", "has", "had",
    // Additional filler words from Python
    "um", "uh", "like", "you know", "so", "actually", "basically", "literally",
    "right", "okay", "well", "i mean", "just", "kind of", "sort of", "totally",
    "seriously", "honestly", "obviously", "really", "anyway", "thing", "stuff",
    "maybe", "perhaps", "alright", "yeah", "no", "yes", "hmm", "hey"
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

// Function to compare the percentage of words from a text in the Trie
double compareData(Trie& trie, const std::string& text) {
    std::vector<std::string> words = tokenize(text, trie);
    int matchCount = 0;

    for (const std::string& word : words) {
        if (trie.search(word)) {
            matchCount++;
        }
    }

    int numWords = trie.getNumWords();
    return (numWords == 0) ? 0.0 : static_cast<double>(matchCount) / numWords;
}

#endif // TOKENIZER_H
