#include "../include/core.h"
#include "helpers.h"
#include "levenshtein_myers.cpp"
#include "trie.h"

#include <cstring>
#include <functional>

bool WordMatchHammingDist(const char *doc_str, const char *query_word,
                          int query_word_len, unsigned int match_dist) {
  return SomeWord(doc_str, [&](const char *word, int len) {
    if (len != query_word_len) {
      return false;
    }

    unsigned int num_mismatches = 0;
    for (int i = 0; i < len; i++) {
      if (word[i] != query_word[i]) {
        num_mismatches++;
      }
      if (num_mismatches > match_dist) {
        return false;
      }
    }

    return true;
  });
}

bool WordMatchEditDist(const char *doc_str, const char *query_word,
                       int query_word_len, unsigned int match_dist) {
  return SomeWord(doc_str, [&](const char *word, int len) {
    return (abs(len - query_word_len) <= match_dist) &&
           LevenshteinMyers32(word, len, query_word, query_word_len) <=
               match_dist;
  });
}

bool MatchQuery(const char *doc_str, const char *query_str, int match_dist,
                MatchType match_type, Trie &trie) {

  std::function<bool(const char *, int len)> callback;

  switch (match_type) {
  case MT_EXACT_MATCH:
    callback = [&](const char *query_word, int len) {
      return trie.search(query_word, len);
    };
    break;
  case MT_HAMMING_DIST:
    callback = [&](const char *query_word, int len) {
      return WordMatchHammingDist(doc_str, query_word, len, match_dist);
    };
    break;
  case MT_EDIT_DIST:
    callback = [&](const char *query_word, int len) {
      return WordMatchEditDist(doc_str, query_word, len, match_dist);
    };
    break;
  default:
    fprintf(stderr, "Unknown match type: %d\n", match_type);
    return false;
  }

  return EveryWord(query_str, callback);
}