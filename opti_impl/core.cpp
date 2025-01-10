#include "../include/core.h"
#include "helpers.cpp"
#include "levenshtein.cpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <future>
#include <thread>
#include <vector>

using namespace std;

struct Query {
  QueryID query_id;
  char str[MAX_QUERY_LENGTH];
  MatchType match_type;
  unsigned int match_dist;
};

struct Document {
  DocID doc_id;
  unsigned int num_res;
  QueryID *query_ids;
};

vector<Query> queries;
vector<Document> docs;

ErrorCode InitializeIndex() { return EC_SUCCESS; }

ErrorCode DestroyIndex() { return EC_SUCCESS; }

ErrorCode StartQuery(QueryID query_id, const char *query_str,
                     MatchType match_type, unsigned int match_dist) {
  Query query = {
      .query_id = query_id,
      .match_type = match_type,
      .match_dist = match_dist,
  };
  strcpy(query.str, query_str);
  queries.push_back(query);
  return EC_SUCCESS;
}

ErrorCode EndQuery(QueryID query_id) {
  unsigned int i, n = queries.size();
  for (i = 0; i < n; i++) {
    if (queries[i].query_id == query_id) {
      queries.erase(queries.begin() + i);
      break;
    }
  }
  return EC_SUCCESS;
}

ErrorCode GetNextAvailRes(DocID *p_doc_id, unsigned int *p_num_res,
                          QueryID **p_query_ids) {
  *p_doc_id = 0;
  *p_num_res = 0;
  *p_query_ids = 0;
  if (docs.size() == 0)
    return EC_NO_AVAIL_RES;
  *p_doc_id = docs[0].doc_id;
  *p_num_res = docs[0].num_res;
  *p_query_ids = docs[0].query_ids;
  docs.erase(docs.begin());
  return EC_SUCCESS;
}

bool WordMatchExact(const char *doc_str, const char *query_word,
                    int query_word_len) {
  return SomeWord(doc_str, [&](const char *word, int len) {
    return len == query_word_len && strncmp(word, query_word, len) == 0;
  });
}

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
    if (LevenshteinDistance(word, len, query_word, query_word_len) <=
        match_dist) {
      return true;
    }
    return false;
  });
}

bool MatchQuery(const char *doc_str, const char *query_str, int match_dist,
                MatchType match_type) {
  return EveryWord(query_str, [&](const char *query_word, int len) {
    switch (match_type) {
    case MT_EXACT_MATCH:
      return WordMatchExact(doc_str, query_word, len);
    case MT_HAMMING_DIST:
      return WordMatchHammingDist(doc_str, query_word, len, match_dist);
    case MT_EDIT_DIST:
      return WordMatchEditDist(doc_str, query_word, len, match_dist);
    default:
      fprintf(stderr, "Unknown match type: %d\n", match_type);
      return false;
    };
  });
}

ErrorCode MatchDocument(DocID doc_id, const char *doc_str) {
  vector<QueryID> query_ids;
  vector<tuple<QueryID, future<bool>>> futures;

  for (const auto &query : queries) {
    futures.emplace_back(query.query_id,
                         async(launch::async, MatchQuery, doc_str, query.str,
                               query.match_dist, query.match_type));
  }

  for (auto &f : futures) {
    int query_id = get<0>(f);
    bool matching_query = get<1>(f).get();
    if (matching_query) {
      query_ids.push_back(query_id);
    }
  }

  Document doc = {
      .doc_id = doc_id,
      .num_res = static_cast<unsigned int>(query_ids.size()),
      .query_ids = nullptr,
  };

  if (doc.num_res) {
    doc.query_ids = (unsigned int *)malloc(doc.num_res * sizeof(unsigned int));
  }

  for (int i = 0; i < doc.num_res; i++) {
    doc.query_ids[i] = query_ids[i];
  }

  docs.push_back(doc);

  return EC_SUCCESS;
}