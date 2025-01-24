#ifndef TRIE_H
#define TRIE_H

#include <iostream>
#include <string>
#include <unordered_map>

class TrieNode {
public:
  TrieNode();
  std::unordered_map<char, TrieNode *> children;
  bool isEndOfWord;
};

class Trie {
private:
  TrieNode *root;

public:
  Trie();
  ~Trie();
  void insert(const char *word, int len);
  bool search(const char *word, int len);

private:
  void deleteTrie(TrieNode *node);
};

#endif
