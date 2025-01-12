#include "../include/core.h"
#include <algorithm>
#include <math.h>
#include <string>

int min_of_three(int a, int b, int c) {
  int ab_min = b ^ ((a ^ b) & -(a < b));     // Minimum of a and b
  return c ^ ((ab_min ^ c) & -(ab_min < c)); // Minimum of ab_min and c
}

// My first attempt at implementing the Levenshtein distance algorithm
// Didn't realize that the modulo operation was so damn slow
int LevenshteinDistanceBadImpl(const char *q_wrd, int q_wrd_len,
                               const char *d_wrd, int d_wrd_len) {
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

int LevenshteinDistance(const char *q_wrd, int q_wrd_len, const char *d_wrd,
                        int d_wrd_len) {
  int BUFF[2][q_wrd_len + 1];

  int *prev_row = BUFF[0];
  int *curr_row = BUFF[1];

  // Initialize first row
  for (int j = 0; j <= q_wrd_len; j++) {
    prev_row[j] = j;
  }

  for (int i = 1; i <= d_wrd_len; i++) {
    curr_row[0] = i;
    for (int j = 1; j <= q_wrd_len; j++) {
      int sub_cost = (q_wrd[j - 1] != d_wrd[i - 1]);

      curr_row[j] = std::min({
          prev_row[j] + 1,           // Deletion
          curr_row[j - 1] + 1,       // Insertion
          prev_row[j - 1] + sub_cost // Substitution
      });
    }

    std::swap(prev_row, curr_row);
  }

  return prev_row[q_wrd_len];
}
