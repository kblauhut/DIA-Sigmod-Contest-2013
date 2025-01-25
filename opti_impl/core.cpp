#include "../include/core.h"
#include "helpers.cpp"
#include "query_matching.cpp"
#include "threadworker.cpp"
#include "trie.cpp"

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <thread>
#include <vector>

struct Query {
  QueryID query_id;
  MatchType match_type;
  unsigned int match_dist;
  std::vector<std::string> query_words;
};

struct Document {
  DocID doc_id;
  unsigned int num_res;
  QueryID *query_ids;
};

ThreadWorker threadworker(std::thread::hardware_concurrency());
std::vector<Query> queries;
std::vector<Document> docs;

ErrorCode InitializeIndex() { return EC_SUCCESS; }

ErrorCode DestroyIndex() { return EC_SUCCESS; }

ErrorCode StartQuery(QueryID query_id, const char *query_str,
                     MatchType match_type, unsigned int match_dist) {
  Query query = {
      .query_id = query_id,
      .match_type = match_type,
      .match_dist = match_dist,
  };

  ForEveryWord(query_str, ([&](const char *query_word, int query_word_len) {
                 query.query_words.push_back(
                     std::string(query_word, query_word_len));
               }));

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
  if (docs.size() == 0) {
    return EC_NO_AVAIL_RES;
  };

  *p_doc_id = docs[0].doc_id;
  *p_num_res = docs[0].num_res;
  *p_query_ids = docs[0].query_ids;
  docs.erase(docs.begin());
  return EC_SUCCESS;
}

std::atomic<int> current_query_idx(0);
void ProcessQueries(std::vector<std::string> &doc_words, Trie &trie,
                    int *matching_queries) {
  size_t query_size = queries.size();

  while (true) {
    int query_idx = current_query_idx++;
    if (query_idx >= query_size) {
      break;
    }

    MatchQuery(doc_words, std::ref(queries[query_idx].query_words),
               queries[query_idx].match_dist, queries[query_idx].match_type,
               trie, queries[query_idx].query_id, matching_queries);
  }
}

ErrorCode MatchDocument(DocID doc_id, const char *doc_str) {
  current_query_idx = 0;
  int max_query_id = 0;
  for (const auto &query : queries) {
    max_query_id = fmax(max_query_id, query.query_id);
  }

  Trie trie = Trie();
  std::vector<std::string> document_words;

  ForEveryWord(doc_str, [&](const char *doc_word, int doc_word_len) {
    trie.insert(doc_word, doc_word_len);
    document_words.push_back(std::string(doc_word, doc_word_len));
  });

  int *matching_queries = new int[max_query_id];
  memset(matching_queries, 0, max_query_id * sizeof(int));

  for (size_t i = 0; i < std::thread::hardware_concurrency(); i++) {
    threadworker.add_task([&]() {
      ProcessQueries(std::ref(document_words), std::ref(trie),
                     matching_queries);
    });
  }

  threadworker.wait_for_all();

  // // Without threading
  // for (size_t i = 0; i < queries.size(); i++) {
  //   MatchQuery(std::ref(document_words), std::ref(queries[i].query_words),
  //              queries[i].match_dist, queries[i].match_type,
  //              std::ref<Trie>(trie), queries[i].query_id, matching_queries);
  // }

  std::vector<QueryID> query_ids;
  for (int i = 0; i < max_query_id; i++) {
    if (matching_queries[i]) {
      query_ids.push_back(i + 1);
    }
  }

  Document doc = {
      .doc_id = doc_id,
      .num_res = static_cast<unsigned int>(query_ids.size()),
      .query_ids = nullptr,
  };

  if (doc.num_res) {
    size_t byte_cnt = doc.num_res * sizeof(unsigned int);
    doc.query_ids = (unsigned int *)malloc(byte_cnt);
    memcpy(doc.query_ids, query_ids.data(), byte_cnt);
  }

  docs.push_back(doc);

  return EC_SUCCESS;
}
