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

bool EveryWord(const char *str,
               std::function<bool(const char *, int len)> callback) {
  int word_start_idx = 0;
  int string_len = strlen(str);
  for (int i = 0; i < string_len; i++) {
    if (str[i] != ' ') {
      continue;
    }

    int word_len = i - word_start_idx;
    if (!callback(str + word_start_idx, word_len)) {
      return false;
    };
    word_start_idx = i + 1;
  }

  return callback(str + word_start_idx, string_len - word_start_idx);
}
