#include "../include/core.h"
#include "hamming_simd.cpp"
#include "helpers.h"
#include "levenshtein_myers.cpp"
#include "trie.h"
#include <cstdio>
#include <vector>

void MatchQuery(std::vector<std::string> &doc_words, const char *query_str,
                int match_dist, MatchType match_type, Trie &trie, int query_id,
                int *matching_queries) {
  std::function<bool(const char *, int len)> callback;

  switch (match_type) {
  case MT_EXACT_MATCH:
    callback = [&](const char *query_word, int query_word_len) {
      return trie.search(query_word, query_word_len);
    };
    break;
  case MT_HAMMING_DIST:
    callback = [&](const char *query_word, int query_word_len) {
      for (auto doc_word : doc_words) {
        if (doc_word.size() != query_word_len) {
          continue;
        }

        if (hamming_simd(query_word, doc_word.c_str(), query_word_len) <=
            match_dist) {
          return true;
        }
      }

      return false;
    };
    break;
  case MT_EDIT_DIST:
    callback = [&](const char *query_word, int query_word_len) {
      for (auto doc_word : doc_words) {
        if (abs((int)doc_word.size() - query_word_len) > match_dist) {
          continue;
        }

        if (LevenshteinMyers32(doc_word.c_str(), doc_word.size(), query_word,
                               query_word_len) <= match_dist) {
          return true;
        }
      }
      return false;
    };
    break;
  default:
    fprintf(stderr, "Unknown match type: %d\n", match_type);
    throw std::runtime_error("Unknown match type");
  }

  matching_queries[query_id - 1] = EveryWord(query_str, callback);
}
