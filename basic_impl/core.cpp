#include "../include/core.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
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

bool WordMatchExact(const char *doc_str, const char *query_word) {
  // TODO: Implement this function
  return true;
}
bool WordMatchHammingDist(const char *doc_str, const char *query_word,
                          unsigned int match_dist) {
  // TODO: Implement this function
  return true;
}
bool WordMatchEditDist(const char *doc_str, const char *query_word,
                       unsigned int match_dist) {
  // TODO: Implement this function
  return true;
}

ErrorCode MatchDocument(DocID doc_id, const char *doc_str) {
  vector<unsigned int> query_ids;

  for (vector<Query>::iterator query = begin(queries); query != end(queries);
       ++query) {
    bool matching_query = true;
    unsigned int word_start_idx = 0;

    for (size_t i = 0; i < strlen(query->str); i++) {
      if (query->str[i] != ' ') {
        continue;
      }

      unsigned int word_len = i - word_start_idx;
      char *query_word = (char *)malloc(word_len + 1);
      strncpy(query_word, query->str + word_start_idx, word_len);
      query_word[word_len] = '\0';

      switch (query->match_type) {
      case MT_EXACT_MATCH:
        if (!WordMatchExact(doc_str, query_word)) {
          matching_query = false;
        }
        break;
      case MT_HAMMING_DIST:
        if (!WordMatchHammingDist(doc_str, query_word, query->match_dist)) {
          matching_query = false;
        }
        break;
      case MT_EDIT_DIST:
        if (!WordMatchEditDist(doc_str, query_word, query->match_dist)) {
          matching_query = false;
        }
        break;
      }

      word_start_idx = i + 1;
    }

    if (matching_query) {
      query_ids.push_back(query->query_id);
    }
  }

  Document doc = {
      .doc_id = doc_id,
      .num_res = query_ids.size(),
      .query_ids = 0,
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