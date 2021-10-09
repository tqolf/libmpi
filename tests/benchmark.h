#undef BENCHER

#include <time.h>
#include <math.h>

#define BENCHER(prefix, todo, N, REPEAT)                                                                   \
    double prefix##avg, prefix##stddev;                                                                    \
    do {                                                                                                   \
        double durations[N];                                                                               \
        for (unsigned __i = 0; __i < N; __i++) {                                                           \
            struct timespec ts, te;                                                                        \
            clock_gettime(CLOCK_MONOTONIC, &ts);                                                           \
            for (unsigned int __j = 0; __j < REPEAT; __j++) { todo; }                                      \
            clock_gettime(CLOCK_MONOTONIC, &te);                                                           \
            durations[__i] =                                                                               \
                ((te.tv_sec - ts.tv_sec) * 1e9 + (te.tv_nsec - ts.tv_nsec)) / static_cast<double>(REPEAT); \
        }                                                                                                  \
        long double __t = 0;                                                                               \
        for (unsigned __i = 0; __i < N; __i++) { __t += durations[__i]; }                                  \
        prefix##avg = __t / N;                                                                             \
                                                                                                           \
        for (unsigned __i = 0; __i < N; __i++) {                                                           \
            double diff = durations[__i] - prefix##avg;                                                    \
            __t += diff * diff;                                                                            \
        }                                                                                                  \
        __t /= N;                                                                                          \
        prefix##stddev = sqrt(__t);                                                                        \
    } while (0)

template <class Tp>
inline void DoNotOptimize(Tp const &value)
{
    asm volatile("" : : "r,m"(value) : "memory");
}

template <class Tp>
inline void DoNotOptimize(Tp &value)
{
#if defined(__clang__)
    asm volatile("" : "+r,m"(value) : : "memory");
#else
    asm volatile("" : "+m,r"(value) : : "memory");
#endif
}
