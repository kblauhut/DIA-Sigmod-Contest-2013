#include "../include/core.h"
#include "helpers.cpp"
#include "query_matching.cpp"
#include "threadpool.cpp"

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

ThreadPool pool(std::thread::hardware_concurrency());
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

  if (match_type == MT_EXACT_MATCH) {
    ForEveryWord(query_str, [&](const char *word, int len) {
      word_map[word].insert(query_id);
    });
  }

  return EC_SUCCESS;
}

ErrorCode EndQuery(QueryID query_id) {
  unsigned int i, n = queries.size();
  for (i = 0; i < n; i++) {
    if (queries[i].query_id == query_id) {
      Query query = queries[i];

      if (query.match_type == MT_EXACT_MATCH) {
        ForEveryWord(query.str, [&](const char *word, int len) {
          word_map[word].erase(query_id);
        });
      }

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
  std::vector<QueryID> query_ids;
  std::vector<std::tuple<std::string, std::future<bool>>> exclude_futures;
  std::vector<std::tuple<QueryID, std::future<bool>>> futures;
  std::unordered_set<QueryID> exclude_query_ids;

  for (auto &entry : word_map) {
    if (entry.second.size() < 7) { // Seems to be a good threshold for now
      continue;
    }

    exclude_futures.emplace_back(entry.first, pool.enqueue(MatchQuery, doc_str,
                                                           entry.first.c_str(),
                                                           0, MT_EXACT_MATCH));
  }

  for (auto &f : exclude_futures) {
    std::string word = std::get<0>(f);
    bool matches = std::get<1>(f).get();
    if (!matches) {
      exclude_query_ids.insert(word_map[word].begin(), word_map[word].end());
    }
  }

  for (const auto &query : queries) {
    if (exclude_query_ids.find(query.query_id) != exclude_query_ids.end()) {
      continue;
    }

    futures.emplace_back(query.query_id,
                         pool.enqueue(MatchQuery, doc_str, query.str,
                                      query.match_dist, query.match_type));
  }

  for (auto &f : futures) {
    int query_id = std::get<0>(f);
    bool matching_query = std::get<1>(f).get();
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
    size_t byte_cnt = doc.num_res * sizeof(unsigned int);
    doc.query_ids = (unsigned int *)malloc(byte_cnt);
    memcpy(doc.query_ids, query_ids.data(), byte_cnt);
  }

  docs.push_back(doc);

  return EC_SUCCESS;
}