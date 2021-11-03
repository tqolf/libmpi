#pragma once

#include <string>
#include <sstream>
#include <iostream>

#include "nameof.h"
#include "tabulate.h"

namespace logging
{
template <typename T>
inline std::string to_string(const T &v)
{
    std::stringstream ss;
    ss << v;

    return ss.str();
}

template <>
inline std::string to_string<bool>(const bool &v)
{
    return v ? "true" : "false";
}

template <>
inline std::string to_string<const char *>(const char *const &v)
{
    return std::string(v);
}

template <>
inline std::string to_string<std::string>(const std::string &v)
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
namespace details
{
struct value {
    template <typename T>
    value(const std::string &name, const T &data) : name(name), data(logging::to_string(data))
    {
    }

    std::string name;
    std::string data;
};
} // namespace details

inline std::ostream &print(std::ostream &os, const details::value &value, const std::string sep = " = ")
{
    return os << value.name << sep << value.data;
}
} // namespace logging

#define __nargs_0(...)                                                    __nargs_1(__VA_ARGS__, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0)
#define __nargs_1(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, ...) q

/* Force 4 * 4 = 16 preprocessor passes on the arguments. */
#define __eval_0(...) __eval_1(__eval_1(__eval_1(__eval_1(__VA_ARGS__))))
#define __eval_1(...) __eval_2(__eval_2(__eval_2(__eval_2(__VA_ARGS__))))
#define __eval_2(...) __VA_ARGS__

#define __log_args(vec, ...)        __eval_0(__log_args_1(vec, __VA_ARGS__))
#define __log_args_1(vec, ...)      __log_args_2(vec, __nargs_0(__VA_ARGS__), __VA_ARGS__)
#define __log_args_2(vec, num, ...) __log_args_3(vec, num, __VA_ARGS__)
#define __log_args_3(vec, num, ...) __log_arg_##num(vec, __VA_ARGS__)

#define __log_arg_1(vec, arg)      __log_arg(vec, arg)
#define __log_arg_2(vec, arg, ...) __log_arg(vec, arg), __loop(vec, __VA_ARGS__)
#define __loop(vec, ...)           __loop_helper __empty_helper()()(vec, __VA_ARGS__)
#define __loop_helper()            __log_args_1
#define __empty_helper()

#define __log_arg(vec, arg) vec.emplace_back(std::string(NAMEOF(arg)), logging::to_string(arg))

#define log_always(os, ...)                                                                   \
    do {                                                                                      \
        std::vector<logging::details::value> vec;                                             \
        __log_args(vec, __VA_ARGS__);                                                         \
        os << std::string(__FILE__) << ":" << __LINE__ << ": ";                               \
        bool indent = false;                                                                  \
        for (auto i = 0; i < vec.size(); i++) {                                               \
            auto &s = vec[i];                                                                 \
            bool ml = std::count(s.data.begin(), s.data.end(), '\n') != 0;                    \
            if (ml) {                                                                         \
                indent = true;                                                                \
                os << std::endl << "\t" << s.name << ":" << std::endl;                        \
                {                                                                             \
                    std::string line;                                                         \
                    std::stringstream ss(s.data);                                             \
                    while (std::getline(ss, line, '\n')) { os << "\t" << line << std::endl; } \
                }                                                                             \
            } else {                                                                          \
                if (indent) { os << "\t"; }                                                   \
                os << s.name << " = " << s.data;                                              \
            }                                                                                 \
            if ((i != vec.size() - 1) && !ml) { os << ", "; }                                 \
        }                                                                                     \
        os << std::endl;                                                                      \
    } while (0)


namespace logging
{
enum class level { fatal = 0, error = 1, warning = 2, info = 3, debug = 4, verbose = 5 };

class logger {
  public:
    logger(std::ostream &os, level level = level::info) : level(level), os(os) {}

    level level;
    std::ostream &os;
};
} // namespace logging

#define logf(logger, ...) \
    if (logger.level <= logging::level::fatal) { log_always(logger.os, #__VA_ARGS__); }

#define loge(logger, ...) \
    if (logger.level <= logging::level::error) { log_always(logger.os, #__VA_ARGS__); }

#define logw(logger, ...) \
    if (logger.level <= logging::level::warning) { log_always(logger.os, #__VA_ARGS__); }

#define logi(logger, ...) \
    if (logger.level <= logging::level::info) { log_always(logger.os, #__VA_ARGS__); }

#define logd(logger, ...) \
    if (logger.level <= logging::level::debug) { log_always(logger.os, #__VA_ARGS__); }

#define logv(logger, ...) \
    if (logger.level <= logging::level::verbose) { log_always(logger.os, #__VA_ARGS__); }
