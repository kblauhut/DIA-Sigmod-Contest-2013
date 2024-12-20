#include "../include/core.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
using namespace std;

ErrorCode StartQuery(QueryID query_id, const char *query_str,
                     MatchType match_type, unsigned int match_dist) {
  return ErrorCode::EC_SUCCESS;
}

ErrorCode EndQuery(QueryID query_id) { return ErrorCode::EC_SUCCESS; }

ErrorCode MatchDocument(DocID doc_id, const char *doc_str) {
  return ErrorCode::EC_SUCCESS;
}

ErrorCode GetNextAvailRes(DocID *p_doc_id, unsigned int *p_num_res,
                          QueryID **p_query_ids) {
  return ErrorCode::EC_SUCCESS;
}
