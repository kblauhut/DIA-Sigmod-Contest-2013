#include "../include/core.h"
#include "hamming_simd.cpp"
#include "helpers.h"
#include "levenshtein_myers.cpp"
#include "trie.h"

bool WordMatchHammingDist(const char *doc_str, int doc_str_len,
                          const char *query_word, int query_word_len,
                          unsigned int match_dist) {
  return SomeWord(doc_str, doc_str_len, [&](const char *word, int len) {
    if (len != query_word_len) {
      return false;
    }

    return hamming_simd(query_word, word, len) <= match_dist;
  });
}

bool WordMatchEditDist(const char *doc_str, int doc_str_len,
                       const char *query_word, int query_word_len,
                       unsigned int match_dist) {
  return SomeWord(doc_str, doc_str_len, [&](const char *word, int len) {
    return (abs(len - query_word_len) <= match_dist) &&
           LevenshteinMyers32(word, len, query_word, query_word_len) <=
               match_dist;
  });
}

void MatchQuery(const char *doc_str, int doc_str_len, const char *query_str,
                int match_dist, MatchType match_type, Trie &trie, int query_id,
                int *matching_queries) {
  std::function<bool(const char *, int len)> callback;

  switch (match_type) {
  case MT_EXACT_MATCH:
    callback = [&](const char *query_word, int len) {
      return trie.search(query_word, len);
    };
    break;
  case MT_HAMMING_DIST:
    callback = [&](const char *query_word, int len) {
      return WordMatchHammingDist(doc_str, doc_str_len, query_word, len,
                                  match_dist);
    };
    break;
  case MT_EDIT_DIST:
    callback = [&](const char *query_word, int len) {
      return WordMatchEditDist(doc_str, doc_str_len, query_word, len,
                               match_dist);
    };
    break;
  default:
    fprintf(stderr, "Unknown match type: %d\n", match_type);
    throw std::runtime_error("Unknown match type");
  }

  matching_queries[query_id - 1] = EveryWord(query_str, callback);
}
