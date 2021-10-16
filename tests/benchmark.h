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
#include <vector>
#include <regex>
#include <algorithm>
#include <iostream>
#include <functional>
#include "tabulate.h"

struct value {
    double avg;
    double stddev;
    std::string name;
    std::vector<std::string> keys;
    value(const std::string &name_, double avg_, double stddev_) : avg(avg_), stddev(stddev_), name(name_)
    {
        std::regex pattern("([\\w\\-]+)");
        auto key_end = std::sregex_iterator();
        auto key_begin = std::sregex_iterator(name_.begin(), name_.end(), pattern);

        for (auto it = key_begin; it != key_end; ++it) { keys.push_back(it->str()); }
    }
};

class BencherCollection {
  public:
    static BencherCollection &GetInstance()
    {
        static BencherCollection instance;
        return instance;
    }

    void insert(const std::string &name, double avg, double stddev)
    {
        collections.emplace_back(name, avg, stddev);
    }

    void display()
    {
        using namespace tabulate;

        Table table("operation with options", "average time(nanoseconds)", "coefficient of variation",
                    "perfermance ratio to ref");
        table[0].format().align(Align::center);

        int i = 0;
        for (auto const &v : collections) {
            double diff = get_ref(v.name).avg / v.avg;

            table.add(v.name, v.avg, v.stddev / v.avg, diff);

            i++;
            if (diff >= 1.2) {
                table[i][3].format().color(Color::green);
                if (diff >= 2.0) {
                    table[i][3].format().styles(Style::bold);
                    if (diff >= 5.0) { table[i][3].format().styles(Style::blink); }
                }
            } else if (diff <= 0.8) {
                table[i][3].format().color(Color::red);
                if (diff <= 0.5) {
                    table[i][3].format().styles(Style::bold);
                    if (diff <= 0.2) { table[i][3].format().styles(Style::blink); }
                }
            }
        }
        table.column(1).format().align(Align::center);
        table.column(2).format().align(Align::center);
        table.column(3).format().align(Align::center);

        // awk '/BEGIN/{ f = 1; next } /END/{ f = 0 } f' data.txt
        std::cout << "TERMINAL BEGIN" << std::endl;
        std::cout << table.xterm() << std::endl;
        std::cout << "TERMINAL END" << std::endl;

        std::cout << "MARKDOWN BEGIN" << std::endl;
        std::cout << table.markdown() << std::endl;
        std::cout << "MARKDOWN END" << std::endl;
    }

    void mark_as_ref(const std::string &s)
    {
        auto it = std::find_if(collections.begin(), collections.end(), [&](const value &another) {
            return s == another.name;
        });
        if (it != collections.end()) {
            references.emplace_back(it->keys[0], it->avg, it->stddev);
        } else {
            std::cout << "not found" << std::endl;
        }
    }

    value get_ref(const std::string &s)
    {
        std::regex pattern("([\\w\\-]+)");
        auto key_begin = std::sregex_iterator(s.begin(), s.end(), pattern);
        if (key_begin != std::sregex_iterator()) {
            auto it = std::find_if(references.begin(), references.end(), [&](const value &another) {
                return key_begin->str() == another.name;
            });
            if (it != references.end()) { return *it; }
        }

        static const value notfound("not found", 0, 0);
        return notfound;
    }

  private:
    std::vector<value> references;
    std::vector<value> collections;

    BencherCollection() {}
    ~BencherCollection()
    {
        display();
    }
};

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

        BencherCollection::GetInstance().insert(title, avg, stddev);
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
