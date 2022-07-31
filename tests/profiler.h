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
#include <unistd.h>

#include <string>
#include <regex>
#include <thread>
#include <algorithm>
#include <iostream>
#include <functional>
#include <map>
#include <vector>
#include <type_traits>
#include <unordered_map>
#include "tabulate.h"

namespace profiler
{
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

#undef BENCHER_MULTITHREAD
#define BENCHER_MULTITHREAD(prefix, todo, N, REPEAT, NP)                                                           \
    double prefix##avg, prefix##stddev;                                                                            \
    do {                                                                                                           \
        double durations[N];                                                                                       \
        for (unsigned __i = 0; __i < N; __i++) {                                                                   \
            struct timespec ts, te;                                                                                \
            std::vector<std::thread> workers;                                                                      \
            clock_gettime(CLOCK_MONOTONIC, &ts);                                                                   \
            for (size_t __k = 0; __k < NP; __k++) {                                                                \
                workers.emplace_back([&]() {                                                                       \
                    for (unsigned int __j = 0; __j < REPEAT; __j++) {                                              \
                        todo;                                                                                      \
                    }                                                                                              \
                });                                                                                                \
            }                                                                                                      \
            for (auto &worker : workers) {                                                                         \
                worker.join();                                                                                     \
            }                                                                                                      \
            clock_gettime(CLOCK_MONOTONIC, &te);                                                                   \
            durations[__i] = static_cast<double>(te.tv_sec - ts.tv_sec) * (1e9 / static_cast<double>(REPEAT * NP)) \
                             + static_cast<double>(te.tv_nsec - ts.tv_nsec) / static_cast<double>(REPEAT * NP);    \
        }                                                                                                          \
        long double __t = 0;                                                                                       \
        for (unsigned __i = 0; __i < N; __i++) {                                                                   \
            __t += static_cast<long double>(durations[__i]);                                                       \
        }                                                                                                          \
        prefix##avg = static_cast<double>(__t / N);                                                                \
                                                                                                                   \
        for (unsigned __i = 0; __i < N; __i++) {                                                                   \
            double diff = durations[__i] - prefix##avg;                                                            \
            __t += static_cast<long double>(diff * diff);                                                          \
        }                                                                                                          \
        __t /= N;                                                                                                  \
        prefix##stddev = static_cast<double>(sqrt(__t));                                                           \
    } while (0)

enum {
    FORMAT_RAW = 0x01,
    FORMAT_REMARK = 0x02,
    FORMAT_TABLE_XTERM = 0x10,
    FORMAT_TABLE_MARKDOWN = 0x20,
    FORMAT_TABLE_LATEX = 0x40,
};

class ProfilerSet {
  public:
    static ProfilerSet &Instance()
    {
        static ProfilerSet instance;
        return instance;
    }

    void Insert(const std::string &name, double avg, double stddev)
    {
        collections.emplace_back(name, avg, stddev);
        if (formats & FORMAT_RAW) {
            std::cout << name << ": avg = " << avg << ", stddev = " << stddev << std::endl;
        }
    }

    void MarkAsReference(const std::string &s)
    {
        auto it = std::find_if(collections.begin(), collections.end(), [&](const ProfileData &another) {
            return s == another.name;
        });
        if (it != collections.end()) {
            references.emplace_back(it->name, it->avg, it->stddev);
            collections.erase(it);
        } else {
            std::cerr << "Not Found: " << s << std::endl;
        }
    }

    void SetTitle(std::string title)
    {
        this->title = std::move(title);
    }

    void SetFormats(unsigned int formats)
    {
        this->formats = formats;
    }

  private:
    struct ProfileData {
        double avg;
        double stddev;
        std::string name;
        std::string package;
        std::string interface;
        std::vector<std::string> keys;
        ProfileData(const std::string &name, double avg, double stddev) : avg(avg), stddev(stddev), name(name)
        {
            std::regex pattern("([\\w\\-:]+)");
            auto key_end = std::sregex_iterator();
            auto key_begin = std::sregex_iterator(name.begin(), name.end(), pattern);

            for (auto it = key_begin; it != key_end; ++it) {
                if (it == key_begin && it->str().find("::") != std::string::npos) {
                    auto pos = it->str().rfind("::");
                    package = it->str().substr(0, pos);
                    interface = it->str().substr(pos + 2);
                    keys.push_back(interface);
                    keys.push_back(package);
                } else {
                    keys.push_back(it->str());
                }
            }
        }
    };
    std::string title;
    unsigned int formats;
    std::vector<ProfileData> references;
    std::vector<ProfileData> collections;

    void make_summary_table(tabulate::Table &table)
    {
        if (references.size() == 0) {
            table.add("brief", "average time\n(nanoseconds)", "instability\n(coefficient of variation)");
        } else {
            table.add("brief", "average time\n(nanoseconds)", "instability\n(coefficient of variation)", "rating");
        }

        int i = 1;
        for (auto const &v : collections) {
            if (references.size() != 0) {
                auto ref = std::find_if(references.begin(), references.end(), [&](const ProfileData &another) -> bool {
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
                    auto diff_decorator = [&](double diff) {
                        if (formats & FORMAT_REMARK) {
                            struct {
                                double threshold;
                                std::string positve;
                                std::string nagtive;
                            } remarks[] = {
                                {10.0, "Tu es mon meilleur frère...", "Quelle est la valeur de votre existence..."},
                                {5.0, "C'est super, dessine-toi une tarte", "Putain de merde, que Dieu vous bénisse"},
                                {2.0, "Tu peux faire mieux, continue", "je suis déçu pour toi"},
                            };
                            for (auto const &remark : remarks) {
                                if (diff >= remark.threshold) {
                                    return tabulate::to_string(diff) + "\n(" + remark.positve + ")";
                                }
                                if (diff <= 1 / remark.threshold) {
                                    return "1/" + tabulate::to_string(1.0 / diff) + "\n(" + remark.nagtive + ")";
                                }
                            }
                        }

                        return diff >= 1.0 ? tabulate::to_string(diff) : ("1/" + tabulate::to_string(1.0 / diff));
                    };
                    table.add(name, tabulate::to_string(v.avg) + "\n" + tabulate::to_string(ref->avg) + "*",
                              v.stddev / v.avg, diff_decorator(diff));

                    if (diff >= 1.2) {
                        table[i][3].format().color(tabulate::Color::green);
                        if (diff >= 2.0) {
                            table[i][3].format().styles(tabulate::Style::bold);
                            if (diff >= 5.0) {
                                table[i][3].format().styles(tabulate::Style::blink);
                            }
                        }
                    } else if (diff <= 0.8) {
                        table[i][3].format().color(tabulate::Color::red);
                        if (diff <= 0.5) {
                            table[i][3].format().styles(tabulate::Style::bold);
                            if (diff <= 0.2) {
                                table[i][3].format().styles(tabulate::Style::blink);
                            }
                        }
                    }
                } else {
                    auto &row = table.add(v.name, v.avg, v.stddev / v.avg, "N/A");
                    row[3].format().styles(tabulate::Style::italic);
                }
            } else {
                table.add(v.name, v.avg, v.stddev / v.avg);
            }
            i++;
        }
        if (formats & FORMAT_REMARK) {
            table.format().multi_bytes_character(true);
        }
        table.format().align(tabulate::Align::center);
        table.column(0).format().align(tabulate::Align::left);
    }

    template <typename T, std::enable_if_t<std::is_integral_v<T>> * = nullptr>
    static unsigned count_set_bits(T n)
    {
        unsigned int count = 0;
        while (n) {
            count += n & 0x1;
            n >>= 1;
        }
        return count;
    }

    ProfilerSet() : formats(FORMAT_RAW | FORMAT_TABLE_XTERM | FORMAT_REMARK) {}
    ~ProfilerSet()
    {
        tabulate::Table table;
        table.set_title(title);
        make_summary_table(table);

        /**
         * output all supported format, and you can catch one or more via
         *
         * awk '/BEGIN/{ f = 1; next } /END/{ f = 0 } f' all-formats.txt
         *
         */
        bool tag = count_set_bits(formats & ~(FORMAT_RAW | FORMAT_REMARK)) > 1;
        if (formats & FORMAT_TABLE_XTERM) {
            if (tag) {
                std::cout << "-----BEGIN XTERM TABLE-----" << std::endl;
            }
            std::cout << table.xterm() << std::endl;
            if (tag) {
                std::cout << "-----END XTERM TABLE-----" << std::endl;
            }
        }

        if (formats & FORMAT_TABLE_MARKDOWN) {
            if (tag) {
                std::cout << "-----BEGIN MARKDOWN TABLE-----" << std::endl;
            }
            std::cout << table.markdown() << std::endl;
            if (tag) {
                std::cout << "-----END MARKDOWN TABLE-----" << std::endl;
            }
        }

        if (formats & FORMAT_TABLE_LATEX) {
            if (tag) {
                std::cout << "-----BEGIN LATEX TABLE-----" << std::endl;
            }
            std::cout << table.latex() << std::endl;
            if (tag) {
                std::cout << "-----END LATEX TABLE-----" << std::endl;
            }
        }
    }
};

int CheckAndTune(std::function<bool(void)> task, size_t &repeat)
{
    if (repeat == 0) { // auto checking
        struct timespec ts, te;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        if (!task()) {
            return -EINVAL;
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

    return 0;
}

inline void Add(const std::string &name, std::function<bool(void)> task, size_t repeat = 0, const size_t N = 5)
{
    if (profiler::CheckAndTune(task, repeat) != 0) {
        std::cerr << name << ": failed" << std::endl;
        return;
    }

    BENCHER(_, task(), N, repeat);
    ProfilerSet::Instance().Insert(name, _avg, _stddev);
}

inline void AddMultiThread(const std::string &name, std::function<bool(void)> task, size_t repeat = 0,
                           const size_t N = 5, size_t concurrency = std::thread::hardware_concurrency())
{
    if (profiler::CheckAndTune(task, repeat) != 0) {
        std::cerr << name << ": failed" << std::endl;
        return;
    }

    BENCHER_MULTITHREAD(_, task(), N, repeat, concurrency);
    ProfilerSet::Instance().Insert(name, _avg, _stddev);
}

inline void SetTitle(const std::string &title)
{
    ProfilerSet::Instance().SetTitle(title);
}

inline void SetFormats(unsigned int formats)
{
    ProfilerSet::Instance().SetFormats(formats);
}

inline void AsReference(const std::string &name)
{
    ProfilerSet::Instance().MarkAsReference(name);
}

template <typename T>
inline void DoNotOptimize(T const &value)
{
    asm volatile("" : : "r,m"(value) : "memory");
}

template <typename T>
inline void DoNotOptimize(T &value)
{
#if defined(__clang__)
    asm volatile("" : "+r,m"(value) : : "memory");
#else
    asm volatile("" : "+m,r"(value) : : "memory");
#endif
}
} // namespace profiler
