#include <functional>

void ForEveryWord(const char *str,
                  std::function<void(const char *, int len)> callback);
