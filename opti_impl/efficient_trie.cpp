#include "efficient_trie.h"
#include <memory>

XTrieNode::XTrieNode() : is_end(false), is_invalid(false) {}

XTrie::XTrie() {
  nodes = std::make_unique<std::array<XTrieNode, MAX_DOC_LENGTH>>();
  root = &nodes->at(node_index++);
}

void XTrie::insert(const char *word, int len) {
  XTrieNode *current = root;

  for (int i = 0; i < len; i++) {
    char ch = word[i];

    if (current->is_invalid) {
      current->children.clear();

      // This could be faster but there is a bug in this snippet and clearing
      // the children seems to be enough
      // for (auto &pair : current->children) {
      //   pair.second->is_invalid = true;
      //   pair.second->is_end = false;
      // }
    }

    if (current->children.find(ch) == current->children.end()) {
      current->children[ch] = &nodes->at(node_index++);
    }

    current->is_invalid = false;
    current = current->children[ch];
  }

  current->is_invalid = false;
  current->is_end = true;
}

bool XTrie::search(const char *word, int len) {
  XTrieNode *current = root;

  for (int i = 0; i < len; i++) {
    if (current->is_invalid) {
      return false;
    }

    char ch = word[i];

    if (current->children.find(ch) == current->children.end()) {
      return false;
    }
    current = current->children[ch];
  }

  return current->is_end && !current->is_invalid;
}

void XTrie::invalidate_trie() { root->is_invalid = true; }
