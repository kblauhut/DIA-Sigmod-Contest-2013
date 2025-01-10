#include "../include/core.h"

// TODO: Add optimized implementation of the Levenshtein distance using Myers'
// algorithm.
int LevenshteinDistance(const char *a, int na, const char *b, int nb) {
  int oo = 0x7FFFFFFF;

  int T[2][MAX_WORD_LENGTH + 1];

  int ia, ib;

  int cur = 0;
  ia = 0;

  for (ib = 0; ib <= nb; ib++)
    T[cur][ib] = ib;

  cur = 1 - cur;

  for (ia = 1; ia <= na; ia++) {
    for (ib = 0; ib <= nb; ib++)
      T[cur][ib] = oo;

    int ib_st = 0;
    int ib_en = nb;

    if (ib_st == 0) {
      ib = 0;
      T[cur][ib] = ia;
      ib_st++;
    }

    for (ib = ib_st; ib <= ib_en; ib++) {
      int ret = oo;

      int d1 = T[1 - cur][ib] + 1;
      int d2 = T[cur][ib - 1] + 1;
      int d3 = T[1 - cur][ib - 1];
      if (a[ia - 1] != b[ib - 1])
        d3++;

      if (d1 < ret)
        ret = d1;
      if (d2 < ret)
        ret = d2;
      if (d3 < ret)
        ret = d3;

      T[cur][ib] = ret;
    }

    cur = 1 - cur;
  }

  int ret = T[1 - cur][nb];

  return ret;
}
