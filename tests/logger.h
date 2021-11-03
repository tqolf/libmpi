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

#define __log_head(os, sep)  os << std::string(__FILE__) << ":" << __LINE__ << sep
#define __log_tail(os)       os << std::endl
#define __log_text(os, text) os << text
#define __log_args(os, sep, ...)

#define __nargs_0(...)                                                    __nargs_1(__VA_ARGS__, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0)
#define __nargs_1(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, ...) q

/* Force 4 * 4 = 16 preprocessor passes on the arguments. */
#define __eval_0(...) __eval_1(__eval_1(__eval_1(__eval_1(__VA_ARGS__))))
#define __eval_1(...) __eval_2(__eval_2(__eval_2(__eval_2(__VA_ARGS__))))
#define __eval_2(...) __VA_ARGS__

/* Eval what follows as many times as necessary. */
#define __print_0(os, ...)      __eval_0(__print_1(os, __VA_ARGS__))
#define __print_1(os, P, ...)   __print_2(os, __nargs_0(__VA_ARGS__), P, __VA_ARGS__)
#define __print_2(os, num, ...) __print_3(os, num, __VA_ARGS__)
#define __print_3(os, num, ...) __arg_##num(os, __VA_ARGS__)

#define __arg_1(os, P, arg)      P##1(os, arg)
#define __arg_2(os, P, arg, ...) P##2(os, arg), __loop(os, P, __VA_ARGS__)
#define __loop(os, ...)          __loop_helper __empty_helper()()(os, __VA_ARGS__)
#define __loop_helper()          __print_1
#define __empty_helper()

#define __custom_log(os, P, ...)       (__print_0(os, P, __VA_ARGS__), os << std::endl)
#define __custom_ilog(os, P, sep, ...) (__log_head(os, sep), __custom_log(os, P, __VA_ARGS__))

#define __log_P_1(os, arg) logging::print(os, logging::details::value(std::string(NAMEOF(arg)), arg))
#define __log_P_2(os, arg) __log_P_1(os, arg) << std::endl

#define __ilog_P_1(os, arg) \
    logging::print(os, logging::details::value(std::string("\t") + std::string(NAMEOF(arg)), arg))
#define __ilog_P_2(os, arg) __ilog_P_1(os, arg) << std::endl

#define log_always(os, ...)   __log_0(os, __nargs_0(__VA_ARGS__), __VA_ARGS__)
#define __log_0(os, num, ...) __log_1(os, num, __VA_ARGS__)
#define __log_1(os, num, ...) __log_arg_##num(os, __VA_ARGS__)
#define __log_arg_1(os, ...)  __custom_ilog(os, __log_P_, " ", __VA_ARGS__)
#define __log_arg_2(os, ...)  __custom_ilog(os, __ilog_P_, "\n", __VA_ARGS__)


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
