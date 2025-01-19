#include "trie.h"

TrieNode::TrieNode() : isEndOfWord(false) {}

Trie::~Trie() { deleteTrie(root); }

Trie::Trie() { root = new TrieNode(); }

void Trie::insert(const char *word, int len) {
  TrieNode *current = root;

  for (int i = 0; i < len; i++) {
    char ch = word[i];
    if (current->children.find(ch) == current->children.end()) {
      current->children[ch] = new TrieNode();
    }
    current = current->children[ch];
  }
  current->isEndOfWord = true;
}

void Trie::deleteTrie(TrieNode *node) {
  if (!node)
    return;
  for (auto &pair : node->children) {
    deleteTrie(pair.second);
  }
  delete node;
}

bool Trie::search(const char *word, int len) {
  TrieNode *current = root;

  for (int i = 0; i < len; i++) {
    char ch = word[i];

    if (current->children.find(ch) == current->children.end()) {
      return false;
    }
    current = current->children[ch];
  }

  // Check if this is the end of a valid word
  return current->isEndOfWord;
}
