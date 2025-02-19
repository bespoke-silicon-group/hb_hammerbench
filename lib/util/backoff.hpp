#ifndef UTIL_BACKOFF_HPP
#define UTIL_BACKOFF_HPP
#include <algorithm>
namespace util
{
template <int MAX, typename Condition>
void exponential_backoff(Condition && cond)
{
    int backoff = 1;
    while (cond())
    {
        for (int i = 0; i < backoff; i++);
        backoff = std::min(backoff << 1, MAX);
    }
}
}

#endif
