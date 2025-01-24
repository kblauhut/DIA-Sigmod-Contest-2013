#include "../include/core.h"
#include "hamming_simd.cpp"
#include "helpers.h"
#include "levenshtein_myers.cpp"
#include "trie.h"
#include <cmath>
#include <cstdio>

void MatchQuery(const char *doc_str, int doc_str_len, const char *query_str,
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
      return SomeWord(doc_str, doc_str_len,
                      [&](const char *doc_word, int doc_word_len) {
                        if (doc_word_len != query_word_len) {
                          return false;
                        }

                        return hamming_simd(query_word, doc_word,
                                            doc_word_len) <= match_dist;
                      });
    };
    break;
  case MT_EDIT_DIST:
    callback = [&](const char *query_word, int query_word_len) {
      return SomeWord(
          doc_str, doc_str_len, [&](const char *doc_word, int doc_word_len) {
            bool reachable_by_insert_delete =
                abs(doc_word_len - query_word_len) <= match_dist;

            if (!reachable_by_insert_delete) {
              return false;
            }

            return LevenshteinMyers32(doc_word, doc_word_len, query_word,
                                      query_word_len) <= match_dist;
          });
    };
    break;
  default:
    fprintf(stderr, "Unknown match type: %d\n", match_type);
    throw std::runtime_error("Unknown match type");
  }

  matching_queries[query_id - 1] = EveryWord(query_str, callback);
}
