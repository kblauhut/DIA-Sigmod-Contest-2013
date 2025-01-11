#include "../include/core.h"
#include <algorithm>
#include <math.h>
#include <string>

int min_of_three(int a, int b, int c) {
  int ab_min = b ^ ((a ^ b) & -(a < b));     // Minimum of a and b
  return c ^ ((ab_min ^ c) & -(ab_min < c)); // Minimum of ab_min and c
}

int LevenshteinDistance(const char *q_wrd, int q_wrd_len, const char *d_wrd,
                        int d_wrd_len) {
  int vec_len = q_wrd_len + 2;
  int vec[vec_len];

  // Initialize the first column with [0, 1, 2, 3, ... q_wrd_len, 0]
  for (int j = 0; j <= q_wrd_len; j++) {
    vec[j] = j;
  }
  vec[q_wrd_len + 1] = 0;

  int top_idx = q_wrd_len;
  int left_idx = 0;
  int top_left_idx = q_wrd_len + 1;

  // For every row
  for (int i = 1; i <= d_wrd_len; i++) {
    vec[top_left_idx] = i;
    top_idx = (top_idx + 1) % vec_len;
    left_idx = (left_idx + 1) % vec_len;
    top_left_idx = (top_left_idx + 1) % vec_len;

    // For every column
    for (int j = 1; j <= q_wrd_len; j++) {
      int act_idx = top_left_idx;

      int nomatch = q_wrd[j - 1] != d_wrd[i - 1];
      int top = vec[top_idx];
      int left = vec[left_idx];
      int top_left = vec[top_left_idx];

      vec[act_idx] = min_of_three(top + 1, left + 1, top_left + nomatch);

      top_idx = (top_idx + 1) % vec_len;
      left_idx = (left_idx + 1) % vec_len;
      top_left_idx = (top_left_idx + 1) % vec_len;
    }
  }

  return vec[top_idx];
}

int EditDistance(const char *a, int na, const char *b, int nb) {
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
