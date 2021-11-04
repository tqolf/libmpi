#pragma once

#include <ctime>
#include <string>
#include <sstream>
#include <iostream>
#include <sys/time.h>

#if __cplusplus >= 202002L
#include "nameof.h" // c++20 required for nameof
#else
#define NAMEOF(...) #__VA_ARGS__
#endif
#include "tabulate.h"

namespace logging
{
template <typename T>
struct has_ostream_operator_impl {
    template <typename V>
    static auto test(V *) -> decltype(std::declval<std::ostream>() << std::declval<V>());
    template <typename>
    static auto test(...) -> std::false_type;

    using type = typename std::is_same<std::ostream &, decltype(test<T>(0))>::type;
};

template <typename T>
struct has_ostream_operator : has_ostream_operator_impl<T>::type {
};

template <typename T, typename std::enable_if<((std::is_integral<T>::value && !std::is_same<T, bool>::value)
                                               || std::is_same<T, float>::value || std::is_same<T, double>::value
                                               || std::is_same<T, long double>::value)
                                                  || has_ostream_operator<T>::value,
                                              T>::type * = nullptr>
inline std::string to_string(const T &v)
{
    std::stringstream ss;
    ss << v;

    return ss.str();
}

template <typename T, typename std::enable_if<std::is_enum<T>::value, T>::type * = nullptr>
inline std::string to_string(const T &v)
{
#if (defined NAMEOF_ENUM) && (defined NAMEOF_ENUM_FLAG)
    using D = std::decay_t<T>;
    using U = std::underlying_type_t<D>;
    const bool valid = static_cast<U>(v) >= static_cast<U>(nameof::detail::min_v<D>)
                       && static_cast<U>(v) <= static_cast<U>(nameof::detail::max_v<D>);

    if (valid) {
        return std::string(NAMEOF_ENUM(v));
    } else {
        return std::string(NAMEOF_ENUM_FLAG(v));
    }
#else
    std::stringstream ss;
    ss << static_cast<typename std::underlying_type<T>::type>(v);

    return ss.str();
#endif
}

inline std::string to_string(const bool v)
{
    return v ? "true" : "false";
}

inline std::string to_string(const char *v)
{
    return std::string(v);
}

inline std::string to_string(const std::string &v)
{
    return v;
}

template <typename Value>
inline std::string to_string(const std::vector<Value> &v)
{
    std::string s = "{", sep = ", ";
    for (auto const &item : v) { s += to_string(item) + sep; }
    if (s.size() >= sep.size()) {
        s.erase(s.size() - sep.size(), sep.size()); // pop last sep
    }
    s += "}";

    return s;
}
} // namespace logging

namespace logging
{
namespace detail
{
struct value {
    template <typename T>
    value(const std::string &name, const T &data) : name(name), data(logging::to_string(data))
    {
    }

    std::string name;
    std::string data;
};
} // namespace detail
} // namespace logging

static inline std::string __log_time(const char *prefix = "\033[2;3m", const char *suffix = "\033[0m ")
{
    std::string s;
    char buffer[48];
    struct tm lctime;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &lctime);
    snprintf(buffer, sizeof(buffer), "%02d/%02d %02d:%02d:%02d.%03d", lctime.tm_mon + 1, lctime.tm_mday, lctime.tm_hour,
             lctime.tm_min, lctime.tm_sec, (int)(((tv.tv_usec + 500) / 1000) % 1000));
    return std::string(prefix) + std::string(buffer) + std::string(suffix);
}

// lazy log
#define __nargs_0(...)                                                    __nargs_1(__VA_ARGS__, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0)
#define __nargs_1(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, ...) q

/* Force 4 * 4 = 16 preprocessor passes on the arguments. */
#define __eval_0(...) __eval_1(__eval_1(__eval_1(__eval_1(__VA_ARGS__))))
#define __eval_1(...) __eval_2(__eval_2(__eval_2(__eval_2(__VA_ARGS__))))
#define __eval_2(...) __VA_ARGS__

#define __log_lazy_args(vec, ...)        __eval_0(__log_lazy_args_1(vec, __VA_ARGS__))
#define __log_lazy_args_1(vec, ...)      __log_lazy_args_2(vec, __nargs_0(__VA_ARGS__), __VA_ARGS__)
#define __log_lazy_args_2(vec, num, ...) __log_lazy_args_3(vec, num, __VA_ARGS__)
#define __log_lazy_args_3(vec, num, ...) __log_lazy_arg_##num(vec, __VA_ARGS__)

#define __log_lazy_arg(vec, arg)        vec.emplace_back(std::string(NAMEOF(arg)), logging::to_string(arg))
#define __log_lazy_arg_1(vec, arg)      __log_lazy_arg(vec, arg)
#define __log_lazy_arg_2(vec, arg, ...) __log_lazy_arg(vec, arg), __loop(vec, __VA_ARGS__)
#define __loop(vec, ...)                __loop_helper __empty_helper()()(vec, __VA_ARGS__)
#define __loop_helper()                 __log_lazy_args_1
#define __empty_helper()

#define log_lazy_always(os, ...)                                                              \
    do {                                                                                      \
        std::vector<logging::detail::value> values;                                           \
        __log_lazy_args(values, __VA_ARGS__);                                                 \
        bool indent = false;                                                                  \
        for (auto i = 0; i < values.size(); i++) {                                            \
            auto &v = values[i];                                                              \
            bool ml = std::count(v.data.begin(), v.data.end(), '\n') != 0;                    \
            if (ml) {                                                                         \
                indent = true;                                                                \
                os << std::endl << "\t" << v.name << ":" << std::endl;                        \
                {                                                                             \
                    std::string line;                                                         \
                    std::stringstream ss(v.data);                                             \
                    while (std::getline(ss, line, '\n')) { os << "\t" << line << std::endl; } \
                }                                                                             \
            } else {                                                                          \
                if (indent) {                                                                 \
                    os << "\t";                                                               \
                    indent = false;                                                           \
                }                                                                             \
                os << v.name << " = " << v.data;                                              \
            }                                                                                 \
            if ((i != values.size() - 1) && !ml) { os << ", "; }                              \
        }                                                                                     \
        os << std::endl;                                                                      \
    } while (0)


namespace logging
{
enum class level { fatal = 0, error = 1, warning = 2, info = 3, debug = 4, verbose = 5 };

class logger {
  public:
    logger(std::ostream &os, level level = level::info) : level(level), os(os) {}

    static logger &instance()
    {
        static logger logger(std::cout);
        return logger;
    }

    level level;
    std::ostream &os;
};
} // namespace logging

#define llogf(...)                                                                                                    \
    if (logging::logger::instance().level <= logging::level::fatal) {                                                 \
        logging::logger::instance().os << "<0> " << __log_time() << std::string(__FILE__) << ":" << __LINE__ << ": "; \
        log_lazy_always(logging::logger::instance().os, ##__VA_ARGS__);                                               \
    }

#define lloge(...)                                                                                                    \
    if (logging::logger::instance().level <= logging::level::error) {                                                 \
        logging::logger::instance().os << "<1> " << __log_time() << std::string(__FILE__) << ":" << __LINE__ << ": "; \
        log_lazy_always(logging::logger::instance().os, ##__VA_ARGS__);                                               \
    }

#define llogw(...)                                                                                                    \
    if (logging::logger::instance().level <= logging::level::warning) {                                               \
        logging::logger::instance().os << "<2> " << __log_time() << std::string(__FILE__) << ":" << __LINE__ << ": "; \
        log_lazy_always(logging::logger::instance().os, ##__VA_ARGS__);                                               \
    }

#define llogi(...)                                                                                                    \
    if (logging::logger::instance().level <= logging::level::info) {                                                  \
        logging::logger::instance().os << "<3> " << __log_time() << std::string(__FILE__) << ":" << __LINE__ << ": "; \
        log_lazy_always(logging::logger::instance().os, ##__VA_ARGS__);                                               \
    }

#define llogd(...)                                                                                                    \
    if (logging::logger::instance().level <= logging::level::debug) {                                                 \
        logging::logger::instance().os << "<4> " << __log_time() << std::string(__FILE__) << ":" << __LINE__ << ": "; \
        log_lazy_always(logging::logger::instance().os, ##__VA_ARGS__);                                               \
    }

#define llogv(...)                                                                                                    \
    if (logging::logger::instance().level <= logging::level::verbose) {                                               \
        logging::logger::instance().os << "<5> " << __log_time() << std::string(__FILE__) << ":" << __LINE__ << ": "; \
        log_lazy_always(logging::logger::instance().os, ##__VA_ARGS__);                                               \
    }
