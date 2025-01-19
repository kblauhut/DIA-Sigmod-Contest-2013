#include "../include/core.h"
#include "helpers.cpp"
#include "query_matching.cpp"
#include "threadworker.cpp"
#include "trie.cpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <future>
#include <map>
#include <thread>
#include <unordered_set>
#include <vector>

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

ThreadWorker threadworker(std::thread::hardware_concurrency());
std::vector<Query> queries;
std::vector<Document> docs;
std::map<std::string, std::unordered_set<QueryID>> word_map;

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

  // if (match_type == MT_EXACT_MATCH) {
  //   ForEveryWord(query_str, [&](const char *word, int len) {
  //     std::string word_capped = std::string(word, len);
  //     word_map[word_capped].insert(query_id);
  //   });
  // }

  return EC_SUCCESS;
}

ErrorCode EndQuery(QueryID query_id) {
  unsigned int i, n = queries.size();
  for (i = 0; i < n; i++) {
    if (queries[i].query_id == query_id) {
      // Query query = queries[i];

      // if (query.match_type == MT_EXACT_MATCH) {
      //   ForEveryWord(query.str, [&](const char *word, int len) {
      //     std::string word_capped = std::string(word, len);
      //     word_map[word_capped].erase(query_id);
      //   });
      // }

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

ErrorCode MatchDocument(DocID doc_id, const char *doc_str) {
  Trie trie = Trie();

  ForEveryWord(doc_str,
               [&](const char *word, int len) { trie.insert(word, len); });
  // std::unordered_set<QueryID> exclude_query_ids;
  // for (auto &entry : word_map) {
  //   if (entry.second.size() < 7 // Seems to be a good threshold for now
  //   ) {
  //     continue;
  //   }

  //   bool matches =
  //       trie.search(entry.first.c_str(), strlen(entry.first.c_str()));
  //   if (!matches) {
  //     exclude_query_ids.insert(entry.second.begin(), entry.second.end());
  //   }
  // }

  int max_query_id = 0;
  for (const auto &query : queries) {
    max_query_id =
        max_query_id > query.query_id ? max_query_id : query.query_id;
  }
  int *matching_queries = new int[max_query_id];
  // Initialize all queries to false
  memset(matching_queries, 0, max_query_id * sizeof(int));
  int doc_str_len = strlen(doc_str);

  size_t batchSize = 16; // The number of queries to process in each task
  for (size_t i = 0; i < queries.size(); i += batchSize) {
    threadworker.add_task([&, i]() {
      for (size_t j = 0; j < batchSize && (i + j) < queries.size(); ++j) {
        MatchQuery(doc_str, doc_str_len, queries[i + j].str,
                   queries[i + j].match_dist, queries[i + j].match_type,
                   std::ref<Trie>(trie), queries[i + j].query_id,
                   matching_queries);
      }
    });
  }

  threadworker.wait_for_all();

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