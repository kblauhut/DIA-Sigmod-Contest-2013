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
  Query query;
  query.query_id = query_id;
  strcpy(query.str, query_str);
  query.match_type = match_type;
  query.match_dist = match_dist;
  // Add this query to the active query set
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

ErrorCode MatchDocument(DocID doc_id, const char *doc_str) {
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
