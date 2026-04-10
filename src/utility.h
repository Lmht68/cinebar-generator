#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <functional>

using ProgressUpdateCbk = std::function<void(size_t current, size_t total)>;
using ProgressCbk = std::function<void()>;

#endif