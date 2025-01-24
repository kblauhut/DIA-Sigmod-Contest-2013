#include "helpers.h"

void ForEveryWord(const char *str,
                  std::function<void(const char *, int len)> callback) {
  int word_start_idx = 0;
  int string_len = strlen(str);
  for (int i = 0; i < string_len; i++) {
    if (str[i] != ' ') {
      continue;
    }

    int word_len = i - word_start_idx;
    callback(str + word_start_idx, word_len);
    word_start_idx = i + 1;
  }

  callback(str + word_start_idx, string_len - word_start_idx);
}
