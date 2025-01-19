#include <functional>

void ForEveryWord(const char *str,
                  std::function<void(const char *, int len)> callback);
bool SomeWord(const char *str,
              std::function<bool(const char *, int len)> callback);
bool EveryWord(const char *str,
               std::function<bool(const char *, int len)> callback);