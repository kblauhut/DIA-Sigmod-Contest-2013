#include "efficient_trie.h"
#include <memory>

XTrieNode::XTrieNode() : is_end(false), is_invalid(false) {}

XTrie::XTrie() {
  nodes = std::make_unique<std::array<XTrieNode, MAX_NODES>>();
  root = &nodes->at(node_index++);
}

void XTrie::insert(const char *word, int len) {
  XTrieNode *current = root;

  for (int i = 0; i < len; i++) {
    char ch = word[i];

    if (current->children.find(ch) == current->children.end()) {
      auto &node = nodes->at(node_index++);

      if (node.is_invalid) {
        node.children.clear();
        node.is_invalid = false;
      }

      current->children[ch] = &node;
    }

    current = current->children[ch];
  }

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

void XTrie::invalidate_trie() {
  root->children.clear();
  root->is_end = false;
  root->is_invalid = false;

  if (node_index < MAX_NODE_IDX) {
    return;
  }

  for (int i = 1; i < node_index; i++) {
    nodes->at(i).is_invalid = true;
    nodes->at(i).is_end = false;
  }

  node_index = 1;
}
