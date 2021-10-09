#undef BENCHER

#include <time.h>
#include <math.h>

#define BENCHER(prefix, todo, N, REPEAT)                                                            \
    double prefix##avg, prefix##stddev;                                                             \
    do {                                                                                            \
        double durations[N];                                                                        \
        for (unsigned __i = 0; __i < N; __i++) {                                                    \
            struct timespec ts, te;                                                                 \
            clock_gettime(CLOCK_MONOTONIC, &ts);                                                    \
            for (unsigned int __j = 0; __j < REPEAT; __j++) { todo; }                               \
            clock_gettime(CLOCK_MONOTONIC, &te);                                                    \
            durations[__i] = (te.tv_sec - ts.tv_sec) * (1e9 / static_cast<double>(REPEAT))          \
                             + (te.tv_nsec - ts.tv_nsec) / static_cast<double>(REPEAT);             \
        }                                                                                           \
        long double __t = 0;                                                                        \
        for (unsigned __i = 0; __i < N; __i++) { __t += static_cast<long double>(durations[__i]); } \
        prefix##avg = static_cast<double>(__t / N);                                                 \
                                                                                                    \
        for (unsigned __i = 0; __i < N; __i++) {                                                    \
            double diff = durations[__i] - prefix##avg;                                             \
            __t += static_cast<long double>(diff * diff);                                           \
        }                                                                                           \
        __t /= N;                                                                                   \
        prefix##stddev = static_cast<double>(sqrt(__t));                                            \
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

#include <string>
#include <iostream>
#include <functional>

class Bencher {
  public:
    Bencher(const std::string &name, std::function<bool(void)> task, size_t repeat = 0, const size_t N = 5)
        : title(name)
    {
        if (repeat == 0) { // auto checking
            struct timespec ts, te;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            if (!task()) { return; }
            clock_gettime(CLOCK_MONOTONIC, &te);

            time_t delta = (te.tv_sec - ts.tv_sec) * 1'000'000'000 + (te.tv_nsec - ts.tv_nsec);
            if (delta <= 1000) {
                repeat = 1000;
            } else {
                // Aim for about 100ms between time checks.
                repeat = 100'000'000 / delta;
                if (repeat > 1000) {
                    repeat = 1000;
                } else if (repeat < 1) {
                    repeat = 1;
                }
            }
        }

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvla-extension"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvla-extension"
#endif
        BENCHER(_, task(), N, repeat);
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
        avg = _avg;
        stddev = _stddev;
    }

    ~Bencher()
    {
        std::cout << title << ": avg = " << avg << ", stddev = " << stddev << std::endl;
    }

  private:
    double avg;
    double stddev;
    std::string title;
};
