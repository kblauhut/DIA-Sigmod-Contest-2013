#include "../include/core.h"
#include "hamming_simd.cpp"
#include "levenshtein_myers.cpp"
#include "trie.h"

#include <cstdio>
#include <vector>

static bool MatchesHamming(std::vector<std::string> &doc_words,
                           std::vector<std::string> &query_words,
                           int match_dist) {
  for (auto query_word : query_words) {
    bool match = false;
    char *query_word_c_ptr = (char *)query_word.c_str();
    int query_word_len = query_word.size();

    for (auto doc_word : doc_words) {
      if (query_word_len != doc_word.size()) {
        continue;
      }

      if (hamming_simd(query_word_c_ptr, doc_word.c_str(), query_word_len) <=
          match_dist) {
        match = true;
      }
    }

    if (!match)
      return false;
  }

  return true;
}

static bool MatchesLevenshtein(std::vector<std::string> &doc_words,
                               std::vector<std::string> &query_words,
                               int match_dist, Trie &trie) {
  for (auto query_word : query_words) {
    bool match = false;

    char *query_word_c_ptr = (char *)query_word.c_str();
    int query_word_len = query_word.size();

    // Cheap check if the word is in the trie and thus already matches
    // seems to be a bit faster with this
    if (trie.search(query_word_c_ptr, query_word_len)) {
      continue;
    }

    for (auto doc_word : doc_words) {
      int doc_word_len = doc_word.size();
      if (abs(doc_word_len - query_word_len) > match_dist) {
        continue;
      }

      int levenshtein_dist = LevenshteinMyers32(
          query_word_c_ptr, query_word_len, doc_word.c_str(), doc_word_len);

      if (levenshtein_dist <= match_dist) {
        match = true;
        break;
      }
    }

    if (!match)
      return false;
  }

  return true;
}

void MatchQuery(std::vector<std::string> &doc_words,
                std::vector<std::string> &query_words, int match_dist,
                MatchType match_type, Trie &trie, int query_id,
                int *matching_queries) {
  int query_index = query_id - 1;

  switch (match_type) {
  case MT_EXACT_MATCH:
    matching_queries[query_index] = true;
    for (auto query_word : query_words) {
      if (!trie.search(query_word.c_str(), query_word.size())) {
        matching_queries[query_index] = false;
        return;
      }
    }
    return;
  case MT_HAMMING_DIST:
    matching_queries[query_index] =
        MatchesHamming(doc_words, query_words, match_dist);
    return;
  case MT_EDIT_DIST:
    matching_queries[query_index] =
        MatchesLevenshtein(doc_words, query_words, match_dist, trie);
    return;
  default:
    fprintf(stderr, "Unknown match type: %d\n", match_type);
    matching_queries[query_index] = false;
    return;
  }
}
