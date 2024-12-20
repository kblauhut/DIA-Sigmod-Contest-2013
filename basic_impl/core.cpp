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

ErrorCode MatchDocument(DocID doc_id, const char *doc_str) {
  vector<unsigned int> query_ids;

  for (vector<Query>::iterator query = begin(queries); query != end(queries);
       ++query) {
    switch (query->match_type) {
    case MT_EXACT_MATCH:
      break;
    case MT_HAMMING_DIST:
      break;
    case MT_EDIT_DIST:
      break;
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