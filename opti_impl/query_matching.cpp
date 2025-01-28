#include "../include/core.h"
#include "efficient_trie.h"
#include "hamming_simd.cpp"
#include "levenshtein_myers.cpp"
#include "levenshtein_myers16x8.cpp"
#include <cstddef>
#include <cstdio>
#include <vector>

static bool MatchesHamming(std::vector<std::string> &doc_words,
                           std::vector<std::string> &query_words,
                           int match_dist) {
  for (const auto &query_word : query_words) {
    bool match = false;
    char *query_word_c_ptr = (char *)query_word.c_str();
    int query_word_len = query_word.size();

    for (const auto &doc_word : doc_words) {
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

static bool RunLevenshtein32(const char *q_wrd, int q_wrd_len,
                             const std::vector<std::string> &doc_words,
                             int match_dist

) {
  Myers32x4Input input = {
      .q_wrd = q_wrd,
      .q_wrd_len = q_wrd_len,
      .d_wrds = {q_wrd, q_wrd, q_wrd, q_wrd},
      .d_wrd_lens = {0, 0, 0, 0},
  };

  int idx_slot = 0;

  for (size_t i = 0; i < doc_words.size(); i++) {
    if (abs((int)doc_words[i].size() - q_wrd_len) > match_dist) {
      continue;
    }

    input.d_wrd_lens[idx_slot] = doc_words[i].size();
    input.d_wrds[idx_slot] = doc_words[i].c_str();

    idx_slot++;
    if (idx_slot < 4) {
      continue;
    }

    auto arr = LevenshteinMyers32x4Simd(input);

    for (int j = 0; j < idx_slot; j++) {
      if (arr[j] <= match_dist)
        return true;
    }

    idx_slot = 0;
  }

  // Flush the remaining words
  auto arr = LevenshteinMyers32x4Simd(input);
  for (int i = 0; i < idx_slot; i++) {
    if (arr[i] <= match_dist) {
      return true;
    }
  }

  return false;
}

static bool RunLevenshtein16(const char *q_wrd, int q_wrd_len,
                             const std::vector<std::string> &doc_words,
                             int match_dist) {
  Myers16x8Input input = {
      .q_wrd = q_wrd,
      .q_wrd_len = q_wrd_len,
      .d_wrds = {q_wrd, q_wrd, q_wrd, q_wrd, q_wrd, q_wrd, q_wrd, q_wrd},
      .d_wrd_lens = {0, 0, 0, 0, 0, 0, 0, 0},
  };

  int idx_slot = 0;

  for (size_t i = 0; i < doc_words.size(); i++) {
    if (abs((int)doc_words[i].size() - q_wrd_len) > match_dist) {
      continue;
    }

    input.d_wrd_lens[idx_slot] = doc_words[i].size();
    input.d_wrds[idx_slot] = doc_words[i].c_str();

    idx_slot++;
    if (idx_slot < 8) {
      continue;
    }

    auto arr = LevenshteinMyers16x8Simd(input);

    for (int j = 0; j < idx_slot; j++) {
      if (arr[j] <= match_dist)
        return true;
    }

    idx_slot = 0;
  }

  // Flush the remaining words
  auto arr = LevenshteinMyers16x8Simd(input);
  for (int i = 0; i < idx_slot; i++) {
    if (arr[i] <= match_dist)
      return true;
  }

  return false;
}

static bool MatchesLevenshtein(std::vector<std::string> &doc_words,
                               int max_doc_word_len,
                               std::vector<std::string> &query_words,
                               int match_dist, XTrie &trie) {
  for (const auto &query_word : query_words) {
    char *query_word_c_ptr = (char *)query_word.c_str();
    int query_word_len = query_word.size();

    // Cheap check if the word is in the trie and thus already matches
    // seems to be a bit faster with this
    if (trie.search(query_word_c_ptr, query_word_len)) {
      continue;
    }

    if (query_word_len <= 16 && max_doc_word_len <= 16) {
      if (!RunLevenshtein16(query_word_c_ptr, query_word_len, doc_words,
                            match_dist)) {
        return false;
      }
    }

    if (!RunLevenshtein32(query_word_c_ptr, query_word_len, doc_words,
                          match_dist)) {
      return false;
    }
  }
  return true;
}

void MatchQuery(std::vector<std::string> &doc_words, int max_doc_word_len,
                std::vector<std::string> &query_words, int match_dist,
                MatchType match_type, XTrie &trie, int query_id,
                int *matching_queries) {
  int query_index = query_id - 1;

  switch (match_type) {
  case MT_EXACT_MATCH:
    matching_queries[query_index] = true;
    for (const auto &query_word : query_words) {
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
    matching_queries[query_index] = MatchesLevenshtein(
        doc_words, max_doc_word_len, query_words, match_dist, trie);
    return;
  default:
    fprintf(stderr, "Unknown match type: %d\n", match_type);
    matching_queries[query_index] = false;
    return;
  }
}
