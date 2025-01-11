#include "../include/core.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <math.h>
#include <string>

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

int CalcVecIdx(int vec_len, int curr, int offset) {
  int offset_mod = offset % vec_len;
  return (vec_len + offset_mod + curr) % vec_len;
}

int MyersLevenshteinDistance(const char *q_wrd, int q_wrd_len,
                             const char *d_wrd, int d_wrd_len) {
  int vec_len = q_wrd_len + 2;
  int vec[vec_len];

  // DEBUG PRINT
  // std::vector<std::vector<int>> MATRIX(q_wrd_len + 1,
  //                                      std::vector<int>(d_wrd_len + 1));
  // DEBUG PRINT

  // Initialize the first column with [0, 1, 2, 3, ... q_wrd_len, 0]
  for (int j = 0; j < q_wrd_len + 1; j++) {
    vec[j] = j;
    // MATRIX[j][0] = j; // DEBUG PRINT
  }
  vec[q_wrd_len + 1] = 0;

  int act_idx = q_wrd_len + 1;

  // For every row
  for (int i = 1; i < d_wrd_len + 1; i++) {
    // For every column
    for (int j = 0; j < q_wrd_len + 1; j++) {
      // printf("act_idx: %d\n", act_idx);

      if (j == 0) {
        vec[act_idx] = i;
      } else {
        char q_char = q_wrd[j - 1];
        char d_char = d_wrd[i - 1];
        int match = q_char == d_char;

        int top_idx = CalcVecIdx(vec_len, act_idx, -1);
        int top = vec[top_idx];

        // printf(" -(vec_len - 2): %d\n", -(vec_len - 2));

        int left_idx = CalcVecIdx(vec_len, act_idx, -(vec_len - 1));
        int left = vec[left_idx];

        int top_left_idx = CalcVecIdx(vec_len, act_idx, -vec_len);
        int top_left = vec[top_left_idx];

        // printf("top_idx: %d, left_idx: %d, top_left_idx: %d\n", top_idx,
        //        left_idx, top_left_idx);
        // printf("top: %d, left: %d, top_left: %d\n", top, left, top_left);

        vec[act_idx] = std::min(top + 1, std::min(left + 1, top_left + !match));
      }

      // MATRIX[j][i] = vec[act_idx]; // DEBUG PRINT
      act_idx = CalcVecIdx(vec_len, act_idx, 1);
    }
  }

  // DEBUG PRINT
  // std::cout << "    ";
  // for (int i = 0; i < d_wrd_len; ++i) {
  //   std::cout << d_wrd[i] << " ";
  // }
  // std::cout << std::endl;

  // for (int j = 0; j < q_wrd_len + 1; ++j) {
  //   if (j == 0) {
  //     std::cout << "  ";
  //   } else {
  //     std::cout << q_wrd[j - 1] << " ";
  //   }

  //   for (int i = 0; i < d_wrd_len + 1; ++i) {

  //     std::cout << MATRIX[j][i] << " ";
  //   }
  //   std::cout << std::endl;
  // }
  // DEBUG PRINT

  return vec[CalcVecIdx(vec_len, act_idx, -1)];
}
