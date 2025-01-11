#include "../include/core.h"
#include <algorithm>
#include <math.h>
#include <string>

int CalcVecIdx(int vec_len, int curr, int offset) {
  int offset_mod = offset % vec_len;
  return (vec_len + offset_mod + curr) % vec_len;
}

int LevenshteinDistance(const char *q_wrd, int q_wrd_len, const char *d_wrd,
                        int d_wrd_len) {
  int vec_len = q_wrd_len + 2;
  int vec[vec_len];

  // Initialize the first column with [0, 1, 2, 3, ... q_wrd_len, 0]
  for (int j = 0; j < q_wrd_len + 1; j++) {
    vec[j] = j;
  }
  vec[q_wrd_len + 1] = 0;

  int act_idx = q_wrd_len + 1;

  // For every row
  for (int i = 1; i < d_wrd_len + 1; i++) {
    // For every column
    for (int j = 0; j < q_wrd_len + 1; j++) {
      if (j == 0) {
        vec[act_idx] = i;
      } else {
        char q_char = q_wrd[j - 1];
        char d_char = d_wrd[i - 1];
        int match = q_char == d_char;

        int top_idx = CalcVecIdx(vec_len, act_idx, -1);
        int top = vec[top_idx];

        int left_idx = CalcVecIdx(vec_len, act_idx, -(vec_len - 1));
        int left = vec[left_idx];

        int top_left_idx = CalcVecIdx(vec_len, act_idx, -vec_len);
        int top_left = vec[top_left_idx];

        vec[act_idx] = std::min(top + 1, std::min(left + 1, top_left + !match));
      }

      act_idx = CalcVecIdx(vec_len, act_idx, 1);
    }
  }

  return vec[CalcVecIdx(vec_len, act_idx, -1)];
}
