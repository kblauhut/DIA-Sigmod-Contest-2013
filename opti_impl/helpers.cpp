#include <functional>
using namespace std;

bool SomeWord(const char *str, function<bool(const char *, int len)> callback) {
  int word_start_idx = 0;
  int string_len = strlen(str);
  for (int i = 0; i < string_len; i++) {
    if (str[i] != ' ') {
      continue;
    }

    int word_len = i - word_start_idx;
    if (callback(str + word_start_idx, word_len)) {
      return true;
    };
    word_start_idx = i + 1;
  }

  if (callback(str + word_start_idx, string_len - word_start_idx)) {
    return true;
  };

  return false;
}

bool EveryWord(const char *str,
               function<bool(const char *, int len)> callback) {
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

  if (!callback(str + word_start_idx, string_len - word_start_idx)) {
    return false;
  };

  return true;
}