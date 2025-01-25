#ifndef EFFICIENT_TRIE_H
#define EFFICIENT_TRIE_H

#include "../include/core.h"
#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

class XTrieNode {
public:
  XTrieNode();
  std::unordered_map<char, XTrieNode *> children;
  bool is_end;
  bool is_invalid;
};

class XTrie {
private:
  XTrieNode *root;
  std::unique_ptr<std::array<XTrieNode, MAX_DOC_LENGTH>> nodes;
  size_t node_index = 0;

public:
  XTrie();
  // ~XTrie();
  void insert(const char *word, int len);
  bool search(const char *word, int len);
  void invalidate_trie();

private:
  void delete_trie(XTrieNode *node);
};

#endif
