/**
 * Copyright 2022 Kiran Nowak(kiran.nowak@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <time.h>
#include <math.h>

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

#undef BENCHER
#define BENCHER(prefix, todo, N, REPEAT)                                                                      \
    double prefix##avg, prefix##stddev;                                                                       \
    do {                                                                                                      \
        double durations[N];                                                                                  \
        for (unsigned __i = 0; __i < N; __i++) {                                                              \
            struct timespec ts, te;                                                                           \
            clock_gettime(CLOCK_MONOTONIC, &ts);                                                              \
            for (unsigned int __j = 0; __j < REPEAT; __j++) {                                                 \
                todo;                                                                                         \
            }                                                                                                 \
            clock_gettime(CLOCK_MONOTONIC, &te);                                                              \
            durations[__i] = static_cast<double>(te.tv_sec - ts.tv_sec) * (1e9 / static_cast<double>(REPEAT)) \
                             + static_cast<double>(te.tv_nsec - ts.tv_nsec) / static_cast<double>(REPEAT);    \
        }                                                                                                     \
        long double __t = 0;                                                                                  \
        for (unsigned __i = 0; __i < N; __i++) {                                                              \
            __t += static_cast<long double>(durations[__i]);                                                  \
        }                                                                                                     \
        prefix##avg = static_cast<double>(__t / N);                                                           \
                                                                                                              \
        for (unsigned __i = 0; __i < N; __i++) {                                                              \
            double diff = durations[__i] - prefix##avg;                                                       \
            __t += static_cast<long double>(diff * diff);                                                     \
        }                                                                                                     \
        __t /= N;                                                                                             \
        prefix##stddev = static_cast<double>(sqrt(__t));                                                      \
    } while (0)

#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include <iostream>
#include <functional>
#include "tabulate.h"

class ProfilerCache {
  public:
    static ProfilerCache &Instance()
    {
        static ProfilerCache instance;
        return instance;
    }

    void insert(const std::string &name, double avg, double stddev)
    {
        collections.emplace_back(name, avg, stddev);
    }

    void mark_as_ref(const std::string &s)
    {
        auto it = std::find_if(collections.begin(), collections.end(), [&](const value &another) {
            return s == another.name;
        });
        if (it != collections.end()) {
            references.emplace_back(it->name, it->avg, it->stddev);
            collections.erase(it);
        } else {
            std::cout << "not found" << std::endl;
        }
    }

  private:
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

            for (auto it = key_begin; it != key_end; ++it) {
                keys.push_back(it->str());
            }
        }
    };
    std::vector<value> references;
    std::vector<value> collections;

    void summary()
    {
        using namespace tabulate;
        Table table;

        if (references.size() == 0) {
            table.add("description", "average time(nanoseconds)", "coefficient of variation");
        } else {
            table.add("description", "average time", "average time(reference)", "coefficient of variation",
                      "perfermance ratio");
        }

        int i = 0;
        for (auto const &v : collections) {
            if (references.size() != 0) {
                auto ref = std::find_if(references.begin(), references.end(), [&](const value &another) -> bool {
                    return v.keys[0] == another.keys[0];
                });
                if (ref != references.end()) {
                    double diff = ref->avg / v.avg;

                    std::string name = v.keys[0] + "(";
                    if (v.keys.size() > 2) {
                        name += "(";
                    }
                    for (size_t _i = 1; _i < v.keys.size(); _i++) {
                        if (_i == v.keys.size() - 1) {
                            name += v.keys[_i];
                        } else {
                            name += v.keys[_i] + ", ";
                        }
                    }
                    if (v.keys.size() > 2) {
                        name += ")";
                    }
                    name += " vs ";
                    if (ref->keys.size() > 2) {
                        name += "(";
                    }
                    for (size_t _i = 1; _i < ref->keys.size(); _i++) {
                        if (_i == ref->keys.size() - 1) {
                            name += ref->keys[_i];
                        } else {
                            name += ref->keys[_i] + ", ";
                        }
                    }
                    if (ref->keys.size() > 2) {
                        name += ")";
                    }
                    name += ")";
                    table.add(name, v.avg, ref->avg, v.stddev / v.avg, diff);

                    i++;
                    if (diff >= 1.2) {
                        table[i][4].format().color(Color::green);
                        if (diff >= 2.0) {
                            table[i][4].format().styles(Style::bold);
                            if (diff >= 5.0) {
                                table[i][4].format().styles(Style::blink);
                            }
                        }
                    } else if (diff <= 0.8) {
                        table[i][4].format().color(Color::red);
                        if (diff <= 0.5) {
                            table[i][4].format().styles(Style::bold);
                            if (diff <= 0.2) {
                                table[i][4].format().styles(Style::blink);
                            }
                        }
                    }
                } else {
                    auto &row = table.add(v.name, v.avg, "No Reference", v.stddev / v.avg, "N/A");
                    row[2].format().styles(Style::italic);
                }
            } else {
                table.add(v.name, v.avg, v.stddev / v.avg);
            }
        }
        table.format().align(Align::center);

        // awk '/BEGIN/{ f = 1; next } /END/{ f = 0 } f' data.txt
        std::cout << "TERMINAL BEGIN" << std::endl;
        std::cout << table.xterm() << std::endl;
        std::cout << "TERMINAL END" << std::endl;

        std::cout << "MARKDOWN BEGIN" << std::endl;
        std::cout << table.markdown() << std::endl;
        std::cout << "MARKDOWN END" << std::endl;
    }

    ProfilerCache() {}
    ~ProfilerCache()
    {
        summary();
    }
};

class Profiler {
  public:
    Profiler(const std::string &name, std::function<bool(void)> task, size_t repeat = 0, const size_t N = 5)
        : ok(true), title(name)
    {
        if (repeat == 0) { // auto checking
            struct timespec ts, te;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            if (!task()) {
                ok = false;
                return;
            }
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
        ProfilerCache::Instance().insert(title, avg, stddev);
    }

    ~Profiler()
    {
        if (ok) {
            std::cout << title << ": avg = " << avg << ", stddev = " << stddev << std::endl;
        } else {
            std::cout << title << ": failed" << std::endl;
        }
    }

  private:
    bool ok;
    double avg;
    double stddev;
    std::string title;
};
