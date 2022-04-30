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

#include <string>
#include <vector>
#include <cctype>
#include <cassert>
#include <sstream>
#include <functional>
#include <locale>
#include <clocale>
#include <wchar.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <regex>
#include <iomanip>

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wswitch-enum"
#elif defined(__clang__)
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif

// https://www.w3schools.com/css/default.asp
namespace tabulate
{
template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}
#define XCONCAT(a, b) __CONCAT(a, b)
#define RESERVED      XCONCAT(resevred_, __COUNTER__)

enum Align { none = 0, left = 1, hcenter = 2, right = 4, center = 18, top = 8, vcenter = 16, bottom = 32 };

enum class Color { black, red, green, yellow, blue, magenta, cyan, white, RESERVED, none };

enum class Which { top, bottom, left, right, top_left, top_right, bottom_left, bottom_right };

enum class Style {
    // 0 - 9
    none,      // default, VT100
    bold,      // bold, VT100
    faint,     // decreased intensity
    italic,    // italicized
    underline, // underlined
    blink,     // blink, VT100
    RESERVED,  // placeholder for underlying_type
    inverse,   // inverse, VT100
    invisible, // hidden
    crossed,   // crossed-out chracters

    // 21 - 29
    doubly_underline, // doubly underlined
    normal,           // neither bold nor faint
    not_italic,       // not italicized
    not_underline,    // not underlined
    steady,           // not blinking
    RESERVED,         // placeholder for underlying_type
    positive,         // not inverse
    visible,          // not hidden
    not_crossed,      // not crossed-out
};

struct TrueColor {
    TrueColor() : hex(DEFAULT), color(Color::none) {}
    TrueColor(int hex) : hex(hex), color(Color::none) {}
    TrueColor(Color color) : color(color)
    {
        switch (color) {
            case Color::black:
                hex = 0x808080;
                break;
            case Color::red:
                hex = 0xFF0000;
                break;
            case Color::green:
                hex = 0x008000;
                break;
            case Color::yellow:
                hex = 0xFFFF00;
                break;
            case Color::blue:
                hex = 0x0000FF;
                break;
            case Color::magenta:
                hex = 0xFF00FF;
                break;
            case Color::cyan:
                hex = 0x00FFFF;
                break;
            case Color::white:
                hex = 0xFFFFFF;
                break;
            case Color::none:
            default:
                hex = DEFAULT;
                break;
        }
    }

    bool none() const
    {
        return hex == DEFAULT;
    }

#define BYTEn(v, n) (((v) >> ((n)*8)) & 0xFF)
    std::tuple<unsigned char, unsigned char, unsigned char> RGB() const
    {
        return std::make_tuple(BYTEn(hex, 2), BYTEn(hex, 1), BYTEn(hex, 0));
    }

    static TrueColor merge(const TrueColor &a, const TrueColor &b)
    {
        int rr = (BYTEn(a.hex, 2) + BYTEn(b.hex, 2) + 1) / 2;
        int gg = (BYTEn(a.hex, 1) + BYTEn(b.hex, 1) + 1) / 2;
        int bb = (BYTEn(a.hex, 0) + BYTEn(b.hex, 0) + 1) / 2;

        return TrueColor((rr << 16) | (gg << 8) | bb);
    }

    static double similarity(const TrueColor &a, const TrueColor &b)
    {
        // d = sqrt((r2-r1)^2 + (g2-g1)^2 + (b2-b1)^2)

        int dr = BYTEn(a.hex, 2) - BYTEn(b.hex, 2);
        int dg = BYTEn(a.hex, 1) - BYTEn(b.hex, 1);
        int db = BYTEn(a.hex, 0) - BYTEn(b.hex, 0);

        const double r = sqrt(static_cast<double>(255 * 255 * 3));
        double d = sqrt(static_cast<double>(dr * dr + dg * dg + db * db));

        return d / r;
    }
#undef BYTEn

    static Color most_similar(const TrueColor &a)
    {
        struct color_distance {
            Color color;
            double distance;
            color_distance(const TrueColor &color, Color base) : color(base)
            {
                distance = similarity(color, base);
            }

            bool operator<(const color_distance &other) const
            {
                return (distance < other.distance);
            }
        };
        // clang-format off
        std::vector<color_distance> distances = {
            color_distance(a, Color::black),
            color_distance(a, Color::red),
            color_distance(a, Color::green),
            color_distance(a, Color::yellow),
            color_distance(a, Color::blue),
            color_distance(a, Color::magenta),
            color_distance(a, Color::cyan),
            color_distance(a, Color::white),
            color_distance(a, Color::none),
        };
        // clang-format on
        std::sort(distances.begin(), distances.end());

        return distances[0].color;
    }

  public:
    int hex;
    Color color;

  private:
    static const int DEFAULT = 0xFF000000;
};

class Cell;
using Styles = std::vector<Style>;
using StringFormatter = std::function<std::string(const std::string &, TrueColor, TrueColor, const Styles &)>;
using BorderFormatter =
    std::function<std::string(Which which, const Cell *self, const Cell *left, const Cell *right, const Cell *top,
                              const Cell *bottom, size_t expect_size, StringFormatter stringformater)>;
using CornerFormatter =
    std::function<std::string(Which which, const Cell *self, const Cell *top_left, const Cell *top_right,
                              const Cell *bottom_left, const Cell *bottom_right, StringFormatter stringformater)>;
} // namespace tabulate

namespace tabulate
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

template <>
inline std::string to_string<Color>(const Color &v)
{
    switch (v) {
        case Color::black:
            return "black";
        case Color::red:
            return "red";
        case Color::green:
            return "green";
        case Color::yellow:
            return "yellow";
        case Color::blue:
            return "blue";
        case Color::magenta:
            return "magenta";
        case Color::cyan:
            return "cyan";
        case Color::white:
            return "white";
        case Color::none:
        default:
            return "(none)";
    }
}

template <>
inline std::string to_string<TrueColor>(const TrueColor &v)
{
    std::stringstream ss;
    ss << "#" << std::setfill('0') << std::setw(6) << std::hex << v.hex;

    return std::string(ss.str());
}

template <>
inline std::string to_string<Style>(const Style &v)
{
    switch (v) {
        default:
            return "(none)";
        case Style::bold:
            return "bold";
        case Style::faint:
            return "faint";
        case Style::italic:
            return "italic";
        case Style::underline:
            return "underline";
        case Style::blink:
            return "blink";
        case Style::inverse:
            return "inverse";
        case Style::invisible:
            return "invisible";
        case Style::crossed:
            return "crossed";
        case Style::doubly_underline:
            return "doubly_underline";
    }
}
} // namespace tabulate

namespace tabulate
{
static const std::string NEWLINE = "\n";

static size_t compute_width(const std::string &text, const std::string &locale, bool wchar_enabled)
{
    // delete ansi escape sequences
    std::regex e("\x1b(?:[@-Z\\-_]|\\[[0-?]*[ -/]*[@-~])");
    std::string str = std::regex_replace(text, e, "");

    if (!wchar_enabled) {
        return str.length();
    }

    if (str.size() == 0) {
        return 0;
    }

    // XXX: Markus Kuhn's open-source wcswidth.c
#if defined(__unix__) || defined(__unix) || defined(__APPLE__)
    {
        // The behavior of wcswidth() depends on the LC_CTYPE category of the current locale.
        // Set the current locale based on cell properties before computing width
        auto old_locale = std::locale::global(std::locale(locale));

        // Convert from narrow std::string to wide string
        wchar_t stackbuff[128], *wstr = stackbuff;
        if (str.size() > 128) {
            wstr = new wchar_t[str.size()];
        }

        std::mbstowcs(wstr, str.c_str(), str.size());

        // Compute display width of wide string
        int len = wcswidth(wstr, str.size());

        if (wstr != stackbuff) {
            delete[] wstr;
        }

        // Restore old locale
        std::locale::global(old_locale);

        if (len >= 0) {
            return len;
        }
    }
#endif

    {
        return (str.length() - std::count_if(str.begin(), str.end(), [](char c) -> bool {
                    return (c & 0xC0) == 0x80;
                }));
    }
}
} // namespace tabulate

namespace tabulate
{
// trim white spaces from the left end of an input string
static std::string lstrip(const std::string &s)
{
    std::string trimed = s;
    trimed.erase(trimed.begin(), std::find_if(trimed.begin(), trimed.end(), [](int ch) {
                     return !std::isspace(ch);
                 }));
    return trimed;
}

static std::vector<std::string> explode_string(const std::string &input, const std::vector<std::string> &separators)
{
    auto first_of = [](const std::string &input, size_t start, const std::vector<std::string> &separators) -> size_t {
        std::vector<size_t> indices;
        for (auto &c : separators) {
            auto index = input.find(c, start);
            if (index != std::string::npos) {
                indices.push_back(index);
            }
        }
        if (indices.size() > 0) {
            return *std::min_element(indices.begin(), indices.end());
        } else {
            return std::string::npos;
        }
    };

    std::vector<std::string> segments;

    size_t start = 0;
    while (true) {
        auto index = first_of(input, start, separators);

        if (index == std::string::npos) {
            segments.push_back(input.substr(start));
            return segments;
        }

        std::string word = input.substr(start, index - start);
        char next_character = input.substr(index, 1)[0];
        // Unlike whitespace, dashes and the like should stick to the word occurring before it.
        if (isspace(next_character)) {
            segments.push_back(word);
            segments.push_back(std::string(1, next_character));
        } else {
            segments.push_back(word + next_character);
        }
        start = index + 1;
    }
}

static std::vector<std::string> wrap_to_lines(const std::string &str, size_t width, const std::string &locale,
                                              bool multi_bytes_character)
{
    std::vector<std::string> lines;
    {
        std::string line;
        std::stringstream ss(str);
        while (std::getline(ss, line, '\n')) {
            lines.push_back(line);
        }
    }

    std::vector<std::string> wrapped_lines;
    for (auto const &line : lines) {
        std::string wrapped;
        std::vector<std::string> words = explode_string(line, {" ", "-", "\t"});

        for (auto &word : words) {
            if (compute_width(wrapped, locale, multi_bytes_character)
                    + compute_width(word, locale, multi_bytes_character)
                > width) {
                if (compute_width(wrapped, locale, multi_bytes_character) > 0) {
                    wrapped_lines.push_back(wrapped);
                    wrapped = "";
                }

                while (compute_width(word, locale, multi_bytes_character) > width) {
                    wrapped = word.substr(0, width - 1) + "-";
                    wrapped_lines.push_back(wrapped);
                    wrapped = "";
                    word = word.substr(width - 1);
                }

                word = lstrip(word);
            }

            wrapped += word;
        }
        wrapped_lines.push_back(wrapped);
    }

    return wrapped_lines;
}

static std::string expand_to_size(const std::string &s, size_t len, bool multi_bytes_character = true)
{
    std::string r;
    if (s == "") {
        return std::string(' ', len);
    }
    size_t swidth = compute_width(s, "", multi_bytes_character);
    for (size_t i = 0; i < len;) {
        if (swidth > len - i) {
            r += s.substr(0, len - i);
        } else {
            r += s;
        }
        i += swidth;
    }
    return r;
}
} // namespace tabulate

namespace tabulate
{
struct Border {
    bool visiable;

    size_t padding;
    TrueColor color;
    std::string content;
    TrueColor background_color;
};

struct Corner {
    bool visiable;
    TrueColor color;
    std::string content;
    TrueColor background_color;
};

class Format {
  public:
    Format()
    {
        cell.width = 0;
        cell.height = 0;
        cell.align = Align::left;
        cell.color = Color::none;
        cell.background_color = Color::none;

        // border-left
        borders.left.visiable = true;
        borders.left.padding = 1;
        borders.left.content = "|";
        borders.left.color = Color::none;
        borders.left.background_color = Color::none;

        // border-right
        borders.right.visiable = true;
        borders.right.padding = 1;
        borders.right.content = "|";
        borders.right.color = Color::none;
        borders.right.background_color = Color::none;

        // border-top
        borders.top.visiable = true;
        borders.top.padding = 0;
        borders.top.content = "-";
        borders.top.color = Color::none;
        borders.top.background_color = Color::none;

        // border-bottom
        borders.bottom.visiable = true;
        borders.bottom.padding = 0;
        borders.bottom.content = "-";
        borders.bottom.color = Color::none;
        borders.bottom.background_color = Color::none;

        // corner-top_left
        corners.top_left.visiable = true;
        corners.top_left.content = "+";
        corners.top_left.color = Color::none;
        corners.top_left.background_color = Color::none;

        // corner-top_right
        corners.top_right.visiable = true;
        corners.top_right.content = "+";
        corners.top_right.color = Color::none;
        corners.top_right.background_color = Color::none;

        // corner-bottom_left
        corners.bottom_left.visiable = true;
        corners.bottom_left.content = "+";
        corners.bottom_left.color = Color::none;
        corners.bottom_left.background_color = Color::none;

        // corner-bottom_right
        corners.bottom_right.visiable = true;
        corners.bottom_right.content = "+";
        corners.bottom_right.color = Color::none;
        corners.bottom_right.background_color = Color::none;

        // internationlization
        internationlization.locale = "";
        internationlization.multi_bytes_character = false;
    }

    size_t width() const
    {
        return cell.width;
    }

    size_t height() const
    {
        return cell.height;
    }

    Format &width(size_t value)
    {
        cell.width = value;
        return *this;
    }

    Align align() const
    {
        return cell.align;
    }

    Format &align(Align value)
    {
        cell.align = value;
        return *this;
    }

    TrueColor color() const
    {
        return cell.color;
    }

    Format &color(TrueColor value)
    {
        cell.color = value;
        return *this;
    }

    TrueColor background_color() const
    {
        return cell.background_color;
    }

    Format &background_color(TrueColor value)
    {
        cell.background_color = value;
        return *this;
    }

    const Styles &styles() const
    {
        return cell.styles;
    }

    Format &styles(Style value)
    {
        cell.styles.push_back(value);
        return *this;
    }

    template <typename... Args>
    Format &styles(Style style, Args... args)
    {
        cell.styles.push_back(style);
        return styles(args...);
    }

    /* borders */
    Format &border(const std::string &value)
    {
        borders.left.content = value;
        borders.right.content = value;
        borders.top.content = value;
        borders.bottom.content = value;
        return *this;
    }

    Format &border_padding(size_t value)
    {
        borders.left.padding = value;
        borders.right.padding = value;
        borders.top.padding = value;
        borders.bottom.padding = value;
        return *this;
    }

    Format &border_color(TrueColor value)
    {
        borders.left.color = value;
        borders.right.color = value;
        borders.top.color = value;
        borders.bottom.color = value;
        return *this;
    }

    Format &border_background_color(TrueColor value)
    {
        borders.left.background_color = value;
        borders.right.background_color = value;
        borders.top.background_color = value;
        borders.bottom.background_color = value;
        return *this;
    }

    Format &border_left(const std::string &value)
    {
        borders.left.content = value;
        return *this;
    }

    Format &border_left_color(TrueColor value)
    {
        borders.left.color = value;
        return *this;
    }

    Format &border_left_background_color(TrueColor value)
    {
        borders.left.background_color = value;
        return *this;
    }

    Format &border_left_padding(size_t value)
    {
        borders.left.padding = value;
        return *this;
    }

    Format &border_right(const std::string &value)
    {
        borders.right.content = value;
        return *this;
    }

    Format &border_right_color(TrueColor value)
    {
        borders.right.color = value;
        return *this;
    }

    Format &border_right_background_color(TrueColor value)
    {
        borders.right.background_color = value;
        return *this;
    }

    Format &border_right_padding(size_t value)
    {
        borders.right.padding = value;
        return *this;
    }

    Format &border_top(const std::string &value)
    {
        borders.top.content = value;
        return *this;
    }

    Format &border_top_color(TrueColor value)
    {
        borders.top.color = value;
        return *this;
    }

    Format &border_top_background_color(TrueColor value)
    {
        borders.top.background_color = value;
        return *this;
    }

    Format &border_top_padding(size_t value)
    {
        borders.top.padding = value;
        return *this;
    }

    Format &border_bottom(const std::string &value)
    {
        borders.bottom.content = value;
        return *this;
    }

    Format &border_bottom_color(TrueColor value)
    {
        borders.bottom.color = value;
        return *this;
    }

    Format &border_bottom_background_color(TrueColor value)
    {
        borders.bottom.background_color = value;
        return *this;
    }

    Format &border_bottom_padding(size_t value)
    {
        borders.bottom.padding = value;
        return *this;
    }

    Format &show_border()
    {
        borders.left.visiable = true;
        borders.right.visiable = true;
        borders.top.visiable = true;
        borders.bottom.visiable = true;
        return *this;
    }

    Format &hide_border()
    {
        borders.left.visiable = false;
        borders.right.visiable = false;
        borders.top.visiable = false;
        borders.bottom.visiable = false;
        return *this;
    }

    Format &show_border_top()
    {
        borders.top.visiable = true;
        return *this;
    }

    Format &hide_border_top()
    {
        borders.top.visiable = false;
        return *this;
    }

    Format &show_border_bottom()
    {
        borders.bottom.visiable = true;
        return *this;
    }

    Format &hide_border_bottom()
    {
        borders.bottom.visiable = false;
        return *this;
    }

    Format &show_border_left()
    {
        borders.left.visiable = true;
        return *this;
    }

    Format &hide_border_left()
    {
        borders.left.visiable = false;
        return *this;
    }

    Format &show_border_right()
    {
        borders.right.visiable = true;
        return *this;
    }

    Format &hide_border_right()
    {
        borders.right.visiable = false;
        return *this;
    }

    /* corners */
    Format &corner(const std::string &value)
    {
        corners.top_left.content = value;
        corners.top_right.content = value;
        corners.bottom_left.content = value;
        corners.bottom_right.content = value;
        return *this;
    }

    Format &corner_color(TrueColor value)
    {
        corners.top_left.color = value;
        corners.top_right.color = value;
        corners.bottom_left.color = value;
        corners.bottom_right.color = value;
        return *this;
    }

    Format &corner_background_color(TrueColor value)
    {
        corners.top_left.background_color = value;
        corners.top_right.background_color = value;
        corners.bottom_left.background_color = value;
        corners.bottom_right.background_color = value;
        return *this;
    }

    Format &corner_top_left(const std::string &value)
    {
        corners.top_left.content = value;
        return *this;
    }

    Format &corner_top_left_color(TrueColor value)
    {
        corners.top_left.color = value;
        return *this;
    }

    Format &corner_top_left_background_color(TrueColor value)
    {
        corners.top_left.background_color = value;
        return *this;
    }

    Format &corner_top_right(const std::string &value)
    {
        corners.top_right.content = value;
        return *this;
    }

    Format &corner_top_right_color(TrueColor value)
    {
        corners.top_right.color = value;
        return *this;
    }

    Format &corner_top_right_background_color(TrueColor value)
    {
        corners.top_right.background_color = value;
        return *this;
    }

    Format &corner_bottom_left(const std::string &value)
    {
        corners.bottom_left.content = value;
        return *this;
    }

    Format &corner_bottom_left_color(TrueColor value)
    {
        corners.bottom_left.color = value;
        return *this;
    }

    Format &corner_bottom_left_background_color(TrueColor value)
    {
        corners.bottom_left.background_color = value;
        return *this;
    }

    Format &corner_bottom_right(const std::string &value)
    {
        corners.bottom_right.content = value;
        return *this;
    }

    Format &corner_bottom_right_color(TrueColor value)
    {
        corners.bottom_right.color = value;
        return *this;
    }

    Format &corner_bottom_right_background_color(TrueColor value)
    {
        corners.bottom_right.background_color = value;
        return *this;
    }

    /* internationlization */
    const std::string &locale() const
    {
        return internationlization.locale;
    }

    Format &locale(const std::string &value)
    {
        internationlization.locale = value;
        return *this;
    }

    bool multi_bytes_character() const
    {
        return internationlization.multi_bytes_character;
    }

    Format &multi_bytes_character(bool value)
    {
        internationlization.multi_bytes_character = value;
        return *this;
    }

  public:
    struct {
        Align align;
        Styles styles;
        TrueColor color, background_color;

        size_t width, height;
    } cell;

    // Element Border
    struct {
        Border left, right, top, bottom;
    } borders;

    // Element Corner
    struct {
        Corner top_left, top_right, bottom_left, bottom_right;
    } corners;

    struct {
        std::string content;
        TrueColor color, background_color;
    } column_separator;

    struct {
        std::string locale;
        bool multi_bytes_character;
    } internationlization;
};

class Cell {
  public:
    Cell(const std::string &content) : m_content(content) {}

    const std::string &get() const
    {
        return m_content;
    }

    void set(const std::string &content)
    {
        m_content = content;
    }

    template <typename T>
    void set(const T value)
    {
        m_content = to_string(value);
    }

    size_t size()
    {
        return compute_width(m_content, m_format.locale(), m_format.multi_bytes_character());
    }

    Format &format()
    {
        return m_format;
    }

    const Format &format() const
    {
        return m_format;
    }

    size_t width() const
    {
        if (m_format.width() != 0) {
            return m_format.width();
        } else {
            if (m_content.empty()) {
                return 0;
            } else {
                std::string line;
                std::stringstream ss(m_content.c_str());

                size_t max_width = 0;
                while (std::getline(ss, line, '\n')) {
                    max_width = std::max(max_width, compute_width(line, m_format.locale(), true));
                }

                return max_width;
            }
        }
    }

    Align align() const
    {
        return m_format.align();
    }

    TrueColor color() const
    {
        return m_format.color();
    }

    TrueColor background_color() const
    {
        return m_format.background_color();
    }

    Styles styles() const
    {
        return m_format.styles();
    }

  private:
    Format m_format;
    std::string m_content;
};

template <template <typename, typename> class Container, typename Value>
class PtrIterator {
    using iterator = typename Container<std::shared_ptr<Value>, std::allocator<std::shared_ptr<Value>>>::iterator;

  public:
    explicit PtrIterator(iterator ptr) : ptr(ptr) {}

    PtrIterator operator++()
    {
        ++ptr;
        return *this;
    }
    bool operator!=(const PtrIterator &other) const
    {
        return ptr != other.ptr;
    }
    Value &operator*()
    {
        return **ptr;
    }

  private:
    iterator ptr;
};

template <template <typename, typename> class Container, typename Value>
class PtrConstIterator {
  public:
    using iterator = typename Container<std::shared_ptr<Value>, std::allocator<std::shared_ptr<Value>>>::const_iterator;
    explicit PtrConstIterator(iterator ptr) : ptr(ptr) {}

    PtrConstIterator operator++()
    {
        ++ptr;
        return *this;
    }
    bool operator!=(const PtrConstIterator &other) const
    {
        return ptr != other.ptr;
    }
    const Value &operator*()
    {
        return **ptr;
    }

  private:
    iterator ptr;
};

class BatchFormat {
  public:
    BatchFormat(std::vector<std::shared_ptr<Cell>> cells) : cells(cells) {}

    inline size_t size()
    {
        return cells.size();
    }

    inline BatchFormat &width(size_t value)
    {
        for (auto &cell : cells) {
            cell->format().width(value);
        }
        return *this;
    }

    inline BatchFormat &align(Align value)
    {
        for (auto &cell : cells) {
            cell->format().align(value);
        }
        return *this;
    }

    inline BatchFormat &color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().color(value);
        }
        return *this;
    }

    inline BatchFormat &background_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().background_color(value);
        }
        return *this;
    }

    inline BatchFormat &styles(Style value)
    {
        for (auto &cell : cells) {
            cell->format().styles(value);
        }
        return *this;
    }

    template <typename... Args>
    inline BatchFormat &styles(Args... values)
    {
        for (auto &cell : cells) {
            cell->format().styles(values...);
        }
        return *this;
    }

    inline BatchFormat &border_padding(size_t value)
    {
        for (auto &cell : cells) {
            cell->format().border_padding(value);
        }
        return *this;
    }

    inline BatchFormat &border_left_padding(size_t value)
    {
        for (auto &cell : cells) {
            cell->format().border_left_padding(value);
        }
        return *this;
    }

    inline BatchFormat &border_right_padding(size_t value)
    {
        for (auto &cell : cells) {
            cell->format().border_right_padding(value);
        }
        return *this;
    }

    inline BatchFormat &border_top_padding(size_t value)
    {
        for (auto &cell : cells) {
            cell->format().border_top_padding(value);
        }
        return *this;
    }

    inline BatchFormat &border_bottom_padding(size_t value)
    {
        for (auto &cell : cells) {
            cell->format().border_bottom_padding(value);
        }
        return *this;
    }

    inline BatchFormat &border(const std::string &value)
    {
        for (auto &cell : cells) {
            cell->format().border(value);
        }
        return *this;
    }

    inline BatchFormat &border_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().border_color(value);
        }
        return *this;
    }

    inline BatchFormat &border_background_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().border_background_color(value);
        }
        return *this;
    }

    inline BatchFormat &border_left(const std::string &value)
    {
        for (auto &cell : cells) {
            cell->format().border_left(value);
        }
        return *this;
    }

    inline BatchFormat &border_left_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().border_left_color(value);
        }
        return *this;
    }

    inline BatchFormat &border_left_background_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().border_left_background_color(value);
        }
        return *this;
    }

    inline BatchFormat &border_right(const std::string &value)
    {
        for (auto &cell : cells) {
            cell->format().border_right(value);
        }
        return *this;
    }

    inline BatchFormat &border_right_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().border_right_color(value);
        }
        return *this;
    }

    inline BatchFormat &border_right_background_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().border_right_background_color(value);
        }
        return *this;
    }

    inline BatchFormat &border_top(const std::string &value)
    {
        for (auto &cell : cells) {
            cell->format().border_top(value);
        }
        return *this;
    }

    inline BatchFormat &border_top_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().border_top_color(value);
        }
        return *this;
    }

    inline BatchFormat &border_top_background_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().border_top_background_color(value);
        }
        return *this;
    }

    inline BatchFormat &border_bottom(const std::string &value)
    {
        for (auto &cell : cells) {
            cell->format().border_bottom(value);
        }
        return *this;
    }

    inline BatchFormat &border_bottom_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().border_bottom_color(value);
        }
        return *this;
    }

    inline BatchFormat &border_bottom_background_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().border_bottom_background_color(value);
        }
        return *this;
    }

    inline BatchFormat &show_border()
    {
        for (auto &cell : cells) {
            cell->format().show_border();
        }
        return *this;
    }

    inline BatchFormat &hide_border()
    {
        for (auto &cell : cells) {
            cell->format().hide_border();
        }
        return *this;
    }

    inline BatchFormat &show_border_top()
    {
        for (auto &cell : cells) {
            cell->format().show_border_top();
        }
        return *this;
    }

    inline BatchFormat &hide_border_top()
    {
        for (auto &cell : cells) {
            cell->format().hide_border_top();
        }
        return *this;
    }

    inline BatchFormat &show_border_bottom()
    {
        for (auto &cell : cells) {
            cell->format().show_border_bottom();
        }
        return *this;
    }

    inline BatchFormat &hide_border_bottom()
    {
        for (auto &cell : cells) {
            cell->format().hide_border_bottom();
        }
        return *this;
    }

    inline BatchFormat &show_border_left()
    {
        for (auto &cell : cells) {
            cell->format().show_border_left();
        }
        return *this;
    }

    inline BatchFormat &hide_border_left()
    {
        for (auto &cell : cells) {
            cell->format().hide_border_left();
        }
        return *this;
    }

    inline BatchFormat &show_border_right()
    {
        for (auto &cell : cells) {
            cell->format().show_border_right();
        }
        return *this;
    }

    inline BatchFormat &hide_border_right()
    {
        for (auto &cell : cells) {
            cell->format().hide_border_right();
        }
        return *this;
    }

    inline BatchFormat &corner(const std::string &value)
    {
        for (auto &cell : cells) {
            cell->format().corner(value);
        }
        return *this;
    }

    inline BatchFormat &corner_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().corner_color(value);
        }
        return *this;
    }

    inline BatchFormat &corner_background_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().corner_background_color(value);
        }
        return *this;
    }

    inline BatchFormat &corner_top_left(const std::string &value)
    {
        for (auto &cell : cells) {
            cell->format().corner_top_left(value);
        }
        return *this;
    }

    inline BatchFormat &corner_top_left_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().corner_top_left_color(value);
        }
        return *this;
    }

    inline BatchFormat &corner_top_left_background_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().corner_top_left_background_color(value);
        }
        return *this;
    }

    inline BatchFormat &corner_top_right(const std::string &value)
    {
        for (auto &cell : cells) {
            cell->format().corner_top_right(value);
        }
        return *this;
    }

    inline BatchFormat &corner_top_right_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().corner_top_right_color(value);
        }
        return *this;
    }

    inline BatchFormat &corner_top_right_background_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().corner_top_right_background_color(value);
        }
        return *this;
    }

    inline BatchFormat &corner_bottom_left(const std::string &value)
    {
        for (auto &cell : cells) {
            cell->format().corner_bottom_left(value);
        }
        return *this;
    }

    inline BatchFormat &corner_bottom_left_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().corner_bottom_left_color(value);
        }
        return *this;
    }

    inline BatchFormat &corner_bottom_left_background_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().corner_bottom_left_background_color(value);
        }
        return *this;
    }

    inline BatchFormat &corner_bottom_right(const std::string &value)
    {
        for (auto &cell : cells) {
            cell->format().corner_bottom_right(value);
        }
        return *this;
    }

    inline BatchFormat &corner_bottom_right_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().corner_bottom_right_color(value);
        }
        return *this;
    }

    inline BatchFormat &corner_bottom_right_background_color(TrueColor value)
    {
        for (auto &cell : cells) {
            cell->format().corner_bottom_right_background_color(value);
        }
        return *this;
    }

    inline BatchFormat &locale(const std::string &value)
    {
        for (auto &cell : cells) {
            cell->format().locale(value);
        }
        return *this;
    }

    inline BatchFormat &multi_bytes_character(bool value)
    {
        for (auto &cell : cells) {
            cell->format().multi_bytes_character(value);
        }
        return *this;
    }

  private:
    std::vector<std::shared_ptr<Cell>> cells;
};

class Row {
  public:
    Row() {}

    template <typename... Args>
    Row(Args... args)
    {
        add(args...);
    }

    template <typename T>
    void add(const T v)
    {
        cells.push_back(std::shared_ptr<Cell>(new Cell(to_string(v))));
    }

    template <typename T, typename... Args>
    void add(T v, Args... args)
    {
        add(v);
        add(args...);
    }

    Cell &operator[](size_t index)
    {
        if (index >= cells.size()) {
            size_t size = index - cells.size() + 1;
            for (size_t i = 0; i < size; i++) {
                add("");
            }
        }
        return *cells[index];
    }

    const Cell &operator[](size_t index) const
    {
        return *cells[index];
    }

    std::shared_ptr<Cell> &cell(size_t index)
    {
        return cells[index];
    }

    BatchFormat format(size_t from, size_t to)
    {
        std::vector<std::shared_ptr<Cell>> selected_cells;
        for (size_t i = std::min(from, to); i <= std::max(from, to); i++) {
            selected_cells.push_back(cells[i]);
        }
        return BatchFormat(selected_cells);
    }

    BatchFormat format(std::initializer_list<std::tuple<size_t, size_t>> ranges)
    {
        std::vector<std::shared_ptr<Cell>> selected_cells;
        for (auto range : ranges) {
            for (size_t i = std::min(std::get<0>(range), std::get<1>(range));
                 i < std::max(std::get<0>(range), std::get<1>(range)); i++) {
                selected_cells.push_back(cells[i]);
            }
        }
        return BatchFormat(selected_cells);
    }

    size_t size() const
    {
        return cells.size();
    }

    std::vector<std::string> dump(StringFormatter stringformatter, BorderFormatter borderformatter,
                                  CornerFormatter cornerformatter, bool showtop, bool showbottom) const
    {
        size_t max_height = 0;
        std::vector<std::vector<std::string>> dumplines;
        for (auto const &cell : cells) {
#ifdef __DEBUG__
            std::cout << "cell: " << cell->get() << std::endl;
            std::cout << "\tcolor: " << to_string(cell->color()) << std::endl;
            std::cout << "\tbackground_color: " << to_string(cell->background_color()) << std::endl;
            if (!cell->styles().empty()) {
                std::cout << "\tstyles: ";
                for (auto &style : cell->styles()) {
                    std::cout << to_string(style);
                    if (&style != &cell->styles().back()) {
                        std::cout << " ";
                    }
                }
                std::cout << std::endl;
            }
#endif
            std::vector<std::string> wrapped;
            if (cell->width() == 0) {
                wrapped.push_back(cell->get());
            } else {
                wrapped = wrap_to_lines(cell->get(), cell->width(), cell->format().locale(),
                                        cell->format().internationlization.multi_bytes_character);
#ifdef __DEBUG__
                std::cout << "wrap to " << wrapped.size() << " lines" << std::endl;
#endif
            }
            max_height = std::max(wrapped.size(), max_height);
            dumplines.push_back(wrapped);
        }

        std::vector<std::string> lines;
        if (showtop && cells.size() > 0 && cells[0]->format().borders.top.visiable) {
            std::string line;
            line +=
                cornerformatter(Which::top_left, cells[0].get(), nullptr, nullptr, nullptr, nullptr, stringformatter);
            for (size_t i = 0; i < cells.size(); i++) {
                auto const cell = cells[i].get();
                auto const left = i > 0 ? cells[i - 1].get() : nullptr;
                auto const right = i < cells.size() ? cells[i + 1].get() : nullptr;

                auto &borders = cell->format().borders;
                size_t size = borders.left.padding + cell->width() + borders.right.padding;
                line += borderformatter(Which::top, cell, left, right, nullptr, nullptr, size, stringformatter);
                line += cornerformatter(Which::top_right, cell, nullptr, nullptr, nullptr, nullptr, stringformatter);
            }
            lines.push_back(line);
        }

        // padding lines on top
        if (cells[0]->format().borders.top.padding > 0) {
            std::string padline;
            for (size_t i = 0; i < cells.size(); i++) {
                auto const cell = cells[i].get();
                auto &borders = cell->format().borders;
                auto const left = i > 0 ? cells[i - 1].get() : nullptr;
                auto const right = i < cells.size() ? cells[i + 1].get() : nullptr;
                size_t size = borders.left.padding + cell->width() + borders.right.padding;

                padline += borderformatter(Which::left, cell, left, right, nullptr, nullptr, 1, stringformatter);
                padline += stringformatter(std::string(size, ' '), Color::none, cell->background_color(), {});
            }
            padline += borderformatter(Which::right, cells.back().get(), nullptr, nullptr, nullptr, nullptr, 1,
                                       stringformatter);

            for (size_t i = 0; i < cells[0]->format().borders.top.padding; i++) {
                lines.push_back(padline);
            }
        }

        // border padding words padding sepeartor padding words padding sepeartor border
        for (size_t i = 0; i < max_height; i++) {
            std::string line;

            if (cells.size() > 0) {
                auto const &bl = cells[0]->format().borders.left;
                if (bl.visiable) {
                    line += stringformatter(bl.content, bl.color, bl.background_color, {});
                }
            }
            for (size_t j = 0; j < cells.size(); j++) {
                Cell &cell = *cells[j];
                auto &pl = cell.format().borders.left;
                auto &pr = cell.format().borders.right;
                auto &br = cell.format().borders.right;
                auto foreground_color = cell.color();
                auto background_color = cell.background_color();
                auto styles = cell.styles();
                size_t cell_offset = 0, empty_lines = max_height - dumplines[j].size();
                auto alignment = cell.format().align();
                if (alignment & Align::bottom) {
                    cell_offset = empty_lines;
                } else if (alignment & Align::top) {
                    cell_offset = 0;
                } else { // DEFAULT: align center in vertical
                    cell_offset = empty_lines / 2;
                }

                line += stringformatter(std::string(pl.padding, ' '), Color::none, background_color, {});
                if (i < cell_offset || i >= dumplines[j].size() + cell_offset) {
                    line += stringformatter(std::string(cell.width(), ' '), Color::none, background_color, styles);
                } else {
                    auto align_line_by = [&](const std::string &str, size_t width, Align align,
                                             const std::string &locale, bool multi_bytes_character) -> std::string {
                        size_t linesize = compute_width(str, locale, multi_bytes_character);
                        if (linesize >= width) {
                            return stringformatter(str, foreground_color, background_color, styles);
                        }
                        if (align & Align::hcenter) {
                            size_t remains = width - linesize;
                            return stringformatter(std::string(remains / 2, ' '), Color::none, background_color, {})
                                   + stringformatter(str, foreground_color, background_color, styles)
                                   + stringformatter(std::string((remains + 1) / 2, ' '), Color::none, background_color,
                                                     {});
                        } else if (align & Align::right) {
                            return stringformatter(std::string(width - linesize, ' '), Color::none, background_color,
                                                   {})
                                   + stringformatter(str, foreground_color, background_color, styles);
                        } else { // DEFAULT: align left in horizontal
                            return stringformatter(str, foreground_color, background_color, styles)
                                   + stringformatter(std::string(width - linesize, ' '), Color::none, background_color,
                                                     {});
                        }
                    };

                    line += align_line_by(dumplines[j][i - cell_offset], cell.width(), alignment,
                                          cell.format().locale(), cell.format().multi_bytes_character());
                }

                line += stringformatter(std::string(pr.padding, ' '), Color::none, background_color, {});
                if (br.visiable) {
                    line += stringformatter(br.content, br.color, br.background_color, {});
                }
            }

            lines.push_back(line);
        }

        // padding lines on bottom
        if (cells.back()->format().borders.bottom.padding > 0) {
            std::string padline;
            for (size_t i = 0; i < cells.size(); i++) {
                auto const cell = cells[i].get();
                auto &borders = cell->format().borders;
                auto const left = i > 0 ? cells[i - 1].get() : nullptr;
                auto const right = i < cells.size() ? cells[i + 1].get() : nullptr;
                size_t size = borders.left.padding + cell->width() + borders.right.padding;

                padline += borderformatter(Which::left, cell, left, right, nullptr, nullptr, 1, stringformatter);
                padline += stringformatter(std::string(size, ' '), Color::none, cell->background_color(), {});
            }
            padline += borderformatter(Which::right, cells.back().get(), nullptr, nullptr, nullptr, nullptr, 1,
                                       stringformatter);

            for (size_t i = 0; i < cells.back()->format().borders.bottom.padding; i++) {
                lines.push_back(padline);
            }
        }

        if (showbottom && cells.size() > 0 && cells.back()->format().borders.bottom.visiable) {
            std::string line;
            line += cornerformatter(Which::bottom_left, cells[0].get(), nullptr, nullptr, nullptr, nullptr,
                                    stringformatter);
            for (size_t i = 0; i < cells.size(); i++) {
                auto const cell = cells[i].get();
                auto const left = i > 0 ? cells[i - 1].get() : nullptr;
                auto const right = i < cells.size() ? cells[i + 1].get() : nullptr;

                auto &borders = cell->format().borders;
                size_t size = borders.left.padding + cell->width() + borders.right.padding;
                line += borderformatter(Which::bottom, cell, left, right, nullptr, nullptr, size, stringformatter);
                line += cornerformatter(Which::bottom_right, cell, nullptr, nullptr, nullptr, nullptr, stringformatter);
            }
            lines.push_back(line);
        }

        return lines;
    }

    /* iterator */
    using iterator = PtrIterator<std::vector, Cell>;
    using const_iterator = PtrConstIterator<std::vector, Cell>;

    iterator begin()
    {
        return iterator(cells.begin());
    }

    iterator end()
    {
        return iterator(cells.end());
    }

    const_iterator begin() const
    {
        return const_iterator(cells.cbegin());
    }

    const_iterator end() const
    {
        return const_iterator(cells.cend());
    }

    BatchFormat format()
    {
        return BatchFormat(cells);
    }

  private:
    std::vector<std::shared_ptr<Cell>> cells;
};

class Column {
  public:
    Column() {}

    void add(std::shared_ptr<Cell> cell)
    {
        cells.push_back(cell);
    }

    BatchFormat format()
    {
        return BatchFormat(cells);
    }

    size_t size()
    {
        return cells.size();
    }

    Cell &operator[](size_t index)
    {
        return *cells[index];
    }

    const Cell &operator[](size_t index) const
    {
        return *cells[index];
    }

    BatchFormat format(size_t from, size_t to)
    {
        std::vector<std::shared_ptr<Cell>> selected_cells;
        for (size_t i = std::min(from, to); i <= std::max(from, to); i++) {
            selected_cells.push_back(cells[i]);
        }
        return BatchFormat(selected_cells);
    }

    BatchFormat format(std::initializer_list<std::tuple<size_t, size_t>> ranges)
    {
        std::vector<std::shared_ptr<Cell>> selected_cells;
        for (auto range : ranges) {
            for (size_t i = std::min(std::get<0>(range), std::get<1>(range));
                 i < std::max(std::get<0>(range), std::get<1>(range)); i++) {
                selected_cells.push_back(cells[i]);
            }
        }
        return BatchFormat(selected_cells);
    }

    /* iterator */
    using iterator = PtrIterator<std::vector, Cell>;
    using const_iterator = PtrConstIterator<std::vector, Cell>;

    iterator begin()
    {
        return iterator(cells.begin());
    }

    iterator end()
    {
        return iterator(cells.end());
    }

    const_iterator begin() const
    {
        return const_iterator(cells.cbegin());
    }

    const_iterator end() const
    {
        return const_iterator(cells.cend());
    }

  private:
    std::vector<std::shared_ptr<Cell>> cells;
};

namespace tabulate
{
// https://invisible-island.net/xterm/ctlseqs/ctlseqs.html
namespace xterm
{
static bool has_truecolor()
{
    const char *term = getenv("TERM");
    if (term == NULL) {
        term = "";
    };
    static const std::vector<std::string> terms_supported_truecolor = {"iterm", "linux", "xterm-truecolor"};

    return std::find(terms_supported_truecolor.begin(), terms_supported_truecolor.end(), term)
           != terms_supported_truecolor.end();
}
static const bool supported_truecolor = has_truecolor();

static std::string stringformatter(const std::string &str, TrueColor foreground_color, TrueColor background_color,
                                   const Styles &styles)
{
    std::string applied;

    bool have = !foreground_color.none() || !background_color.none() || styles.size() != 0;

    if (have) {
        auto rgb = [](TrueColor color) -> std::string {
            auto v = color.RGB();
            return std::to_string(std::get<0>(v)) + ":" + std::to_string(std::get<1>(v)) + ":"
                   + std::to_string(std::get<2>(v));
        };

        auto style_code = [](Style style) -> unsigned int {
            // @RESERVED
            unsigned int i = to_underlying(style);
            switch (i) {
                case 0 ... 9: // 0 - 9
                    return i;
                case 10 ... 18: // 21 - 29
                    return i + 11;
                default:
                    return 0; // none, reset
            }
        };

        if (!supported_truecolor) {
            applied += std::string("\033[");
            applied += std::to_string(to_underlying(TrueColor::most_similar(foreground_color.color)) + 30) + ";";
            applied += std::to_string(to_underlying(TrueColor::most_similar(background_color.color)) + 40) + ";";

            if (styles.size() > 0) {
                for (auto const &style : styles) {
                    // style: 0 - 9, 21 - 29
                    applied += std::to_string(style_code(style)) + ";";
                }
            }
            applied[applied.size() - 1] = 'm';
        } else {
            if (!foreground_color.none()) {
                // TrueColor: CSI 38 : 2 : r : g : b m
                applied += std::string("\033[38:2:") + rgb(foreground_color) + "m";
            }

            if (!background_color.none()) {
                // CSI 48 : 2 : r : g : b m
                applied += std::string("\033[48:2:") + rgb(background_color) + "m";
            }

            if (styles.size() > 0) {
                applied += "\033[";
                for (auto const &style : styles) {
                    // style: 0 - 9, 21 - 29
                    applied += std::to_string(style_code(style)) + ";";
                }
                applied[applied.size() - 1] = 'm';
            }
        }
    }

    applied += str;

    if (have) {
        applied += "\033[00m";
    }

    return applied;
}

std::string borderformatter(Which which, const Cell *self, const Cell *left, const Cell *right, const Cell *top,
                            const Cell *bottom, size_t expected_size, StringFormatter stringformatter)
{
#define TRY_GET(pattern, which, which_reverse)                                                                   \
    if (self->format().pattern.which.visiable) {                                                                 \
        auto it = self->format().pattern.which;                                                                  \
        return stringformatter(                                                                                  \
            expand_to_size(it.content, expected_size, self->format().internationlization.multi_bytes_character), \
            it.color, it.background_color, {});                                                                  \
    } else if (which && which->format().pattern.which_reverse.visiable) {                                        \
        auto it = which->format().pattern.which_reverse;                                                         \
        return stringformatter(                                                                                  \
            expand_to_size(it.content, expected_size, self->format().internationlization.multi_bytes_character), \
            it.color, it.background_color, {});                                                                  \
    }
    if (which == Which::top) {
        TRY_GET(borders, top, bottom);
    } else if (which == Which::bottom) {
        TRY_GET(borders, bottom, top);
    } else if (which == Which::left) {
        TRY_GET(borders, left, right);
    } else if (which == Which::right) {
        TRY_GET(borders, right, left);
    }
#undef TRY_GET
    return "";
}

std::string cornerformatter(Which which, const Cell *self, const Cell *top_left, const Cell *top_right,
                            const Cell *bottom_left, const Cell *bottom_right, StringFormatter stringformatter)
{
#define TRY_GET(pattern, which, which_reverse)                                 \
    if (self->format().pattern.which.visiable) {                               \
        auto it = self->format().pattern.which;                                \
        return stringformatter(it.content, it.color, it.background_color, {}); \
    } else if (which && which->format().pattern.which_reverse.visiable) {      \
        auto it = which->format().pattern.which_reverse;                       \
        return stringformatter(it.content, it.color, it.background_color, {}); \
    }
    if (which == Which::top_left) {
        TRY_GET(corners, top_left, bottom_right);
    } else if (which == Which::top_right) {
        TRY_GET(corners, top_right, bottom_left);
    } else if (which == Which::bottom_left) {
        TRY_GET(corners, bottom_left, top_right);
    } else if (which == Which::bottom_right) {
        TRY_GET(corners, bottom_right, top_left);
    }
#undef TRY_GET
    return "";
}
} // namespace xterm
} // namespace tabulate

namespace tabulate
{
namespace markdown
{
std::string stringformatter(const std::string &str, TrueColor foreground_color, TrueColor background_color,
                            const Styles &styles)
{
    std::string applied;

    bool have = !foreground_color.none() || !background_color.none() || styles.size() != 0;

    if (have) {
        applied += "<span style=\"";

        if (!foreground_color.none()) {
            // color: <color>
            applied += "color:" + to_string(foreground_color) + ";";
        }

        if (!background_color.none()) {
            // color: <color>
            applied += "background-color:" + to_string(background_color) + ";";
        }

        for (auto const &style : styles) {
            switch (style) {
                case Style::bold:
                    applied += "font-weight:bold;";
                    break;
                case Style::italic:
                    applied += "font-style:italic;";
                    break;
                // text-decoration: none|underline|overline|line-through|blink
                case Style::crossed:
                    applied += "text-decoration:line-through;";
                    break;
                case Style::underline:
                    applied += "text-decoration:underline;";
                    break;
                case Style::blink:
                    applied += "text-decoration:blink;";
                    break;
                default:
                    // unsupported, do nothing
                    break;
            }
        }
        applied += "\">";
    }

    applied += str;

    if (have) {
        applied += "</span>";
    }

    return applied;
}

std::string borderformatter(Which, const Cell *, const Cell *, const Cell *, const Cell *, const Cell *, size_t,
                            StringFormatter)
{
    return "|";
}

std::string cornerformatter(Which, const Cell *, const Cell *, const Cell *, const Cell *, const Cell *,
                            StringFormatter)
{
    return "";
}
} // namespace markdown
} // namespace tabulate

class Table : public std::enable_shared_from_this<Table> {
  public:
    Table() {}

    template <typename... Args>
    Table(Args... args)
    {
        add(args...);
    }

    void set_title(std::string title)
    {
        this->title = std::move(title);
    }

    BatchFormat format()
    {
        return BatchFormat(cells);
    }

    Row &add()
    {
        Row &row = __add_row();
        __on_add_auto_update();

        return row;
    }

    template <typename T>
    Row &add(T arg)
    {
        Row &row = __add_row();
        row.add(arg);
        __on_add_auto_update();

        return row;
    }

    template <typename... Args>
    Row &add(Args... args)
    {
        Row &row = __add_row();
        row.add(args...);
        __on_add_auto_update();

        return row;
    }

    template <typename Value, template <typename, typename> class Container, typename Allocator = std::allocator<Value>>
    Row &add_multiple(const Container<Value, Allocator> &multiple)
    {
        Row &row = __add_row();
        for (auto const &value : multiple) {
            row.add(value);
        }
        __on_add_auto_update();

        return row;
    }

    Row &operator[](size_t index)
    {
        if (index >= rows.size()) {
            size_t size = index + 1 - rows.size();
            for (size_t i = 0; i < size; i++) {
                add();
            }
        }
        return *rows[index];
    }

    size_t size()
    {
        return rows.size();
    }

    /* iterator */
    using iterator = PtrIterator<std::vector, Row>;
    using const_iterator = PtrConstIterator<std::vector, Row>;

    iterator begin()
    {
        return iterator(rows.begin());
    }

    iterator end()
    {
        return iterator(rows.end());
    }

    const_iterator begin() const
    {
        return const_iterator(rows.cbegin());
    }

    const_iterator end() const
    {
        return const_iterator(rows.cend());
    }

    Column column(size_t index)
    {
        Column column;
        for (auto &row : rows) {
            if (row->size() <= index) {
                size_t size = index - row->size() + 1;
                for (size_t i = 0; i < size; i++) {
                    row->add("");
                }
            }
            column.add(row->cell(index));
        }

        return column;
    }

    size_t column_size() const
    {
        size_t max_size = 0;
        for (auto const &row : rows) {
            max_size = std::max(max_size, row->size());
        }
        return max_size;
    }

    size_t width() const
    {
        return cached_width;
    }

    // TODO: merge cells
    int merge(std::tuple<int, int> from, std::tuple<int, int> to)
    {
        auto fx = std::get<0>(from), tx = std::get<0>(to);
        auto fy = std::get<1>(from), ty = std::get<1>(to);
        if (fx != tx && fy != ty) {
            merges.push_back(std::tuple<int, int, int, int>(fx, fy, tx, ty));
        }

        return 0;
    }

    std::string xterm() const
    {
        std::string exported;
        // add title
        if (!title.empty() && rows.size() > 0) {
            exported += std::string((cached_width - title.size()) / 2, ' ') + title + NEWLINE;
        }

        // add header
        if (rows.size() > 0) {
            const auto &header = *rows[0];
            for (auto line : header.dump(tabulate::xterm::stringformatter, tabulate::xterm::borderformatter,
                                         tabulate::xterm::cornerformatter, true, rows.size() == 1)) {
                exported += line + NEWLINE;
            }
        }

        // add table content
        for (size_t i = 1; i < rows.size(); i++) {
            auto const &row = *rows[i];
            for (auto const &line : row.dump(tabulate::xterm::stringformatter, tabulate::xterm::borderformatter,
                                             tabulate::xterm::cornerformatter, true, i == rows.size() - 1)) {
                exported += line + NEWLINE;
            }
        }
        if (exported.size() >= NEWLINE.size()) {
            exported.erase(exported.size() - NEWLINE.size(), NEWLINE.size()); // pop last NEWLINE
        }

        return exported;
    }

    /* page break for xterm */
    std::string xterm(size_t maxlines, bool keep_row_in_one_page = true) const
    {
        std::string exported;
        if (!title.empty() && rows.size() > 0) {
            size_t size = width();
            if (size > title.size()) {
                exported = std::string((size - title.size()) / 2, ' ') + title + NEWLINE;
            } else {
                auto lines = wrap_to_lines(title, size, "", true);
                for (auto line : lines) {
                    exported += line + NEWLINE;
                }
            }
        }

        // add header
        size_t hlines = 0;
        std::string header;
        if (rows.size() > 0) {
            const auto &_header = *rows[0];
            for (auto const &line : _header.dump(tabulate::xterm::stringformatter, tabulate::xterm::borderformatter,
                                                 tabulate::xterm::cornerformatter, true, rows.size() == 1)) {
                hlines++;
                header += line + NEWLINE;
            }
        }
        if (maxlines <= hlines) { // maxlines too small
            return header + "===== <Inappropriate Max Lines for PageBreak> ====";
        }

        exported += header;
        size_t nlines = hlines;
        for (size_t i = 1; i < rows.size(); i++) {
            auto const &row = *rows[i];
            auto lines = row.dump(tabulate::xterm::stringformatter, tabulate::xterm::borderformatter,
                                  tabulate::xterm::cornerformatter, true, i == rows.size() - 1);

            if (keep_row_in_one_page) {
                size_t rowlines = lines.size();
                if (hlines + rowlines > maxlines) {
                    return exported + "===== <Inappropriate Max Lines for PageBreak> ====";
                }
                if (nlines + rowlines > maxlines) {
                    if (exported.size() >= NEWLINE.size()) {
                        exported.erase(exported.size() - NEWLINE.size(), NEWLINE.size()); // pop last NEWLINE
                    }
                    exported += "\x0c" + header;
                    nlines = hlines;
                }
                nlines += rowlines;
                for (auto const &line : lines) {
                    exported += line + NEWLINE;
                }
            } else {
                for (auto line : lines) {
                    if (nlines >= maxlines) {
                        if (exported.size() >= NEWLINE.size()) {
                            exported.erase(exported.size() - NEWLINE.size(), NEWLINE.size()); // pop last NEWLINE
                        }
                        exported += "\x0c" + header;
                        nlines = hlines;
                    }
                    nlines++;
                    exported += line;
                }
            }
        }

        return exported;
    }

    std::string markdown() const
    {
        std::string exported;

        // add header
        for (auto const &line : rows[0]->dump(tabulate::markdown::stringformatter, tabulate::markdown::borderformatter,
                                              tabulate::markdown::cornerformatter, false, false)) {
            exported += line + NEWLINE;
        }

        // add alignentment
        {
            std::string alignment = "|";
            for (auto const &cell : *rows[0]) {
                switch (cell.align()) {
                    case Align::left:
                        alignment += " :--";
                        break;
                    case Align::right:
                        alignment += " --:";
                        break;
                    case Align::center:
                        alignment += " :-:";
                        break;
                    default:
                        alignment += " ---";
                        break;
                }
                alignment += " |";
            }
            exported += alignment + NEWLINE;
        }

        for (size_t i = 1; i < rows.size(); i++) {
            auto const &row = *rows[i];
            for (auto const &line : row.dump(tabulate::markdown::stringformatter, tabulate::markdown::borderformatter,
                                             tabulate::markdown::cornerformatter, false, false)) {
                exported += line + NEWLINE;
            }
        }
        if (exported.size() >= NEWLINE.size()) {
            exported.erase(exported.size() - NEWLINE.size(), NEWLINE.size()); // pop last NEWLINE
        }

        return exported;
    }

  private:
    std::string title;
    std::vector<std::shared_ptr<Row>> rows;
    std::vector<std::shared_ptr<Cell>> cells; // for batch format
    std::vector<std::tuple<int, int, int, int>> merges;

    size_t cached_width;

    Row &__add_row()
    {
        auto row = std::make_shared<Row>();
        rows.push_back(row);
        return *row;
    }

    void __on_add_auto_update()
    {
        // auto update width
        size_t headerwidth = 0;
        for (size_t i = 0; i < column_size(); i++) {
            Column col = column(i);

            size_t oldwidth = col.size() == 0 ? 0 : col[0].width();
            size_t newwidth = col[col.size() - 1].width();

            if (newwidth != oldwidth) {
                if (newwidth > oldwidth) {
                    headerwidth += newwidth;
                    col.format().width(newwidth);
                } else {
                    headerwidth += oldwidth;
                    col[col.size() - 1].format().width(oldwidth);
                }
            } else {
                headerwidth += oldwidth;
            }
        }
        cached_width = headerwidth;

        // append new cells
        Row &last_row = *rows[rows.size() - 1];
        for (size_t i = 0; i < last_row.size(); i++) {
            cells.push_back(last_row.cell(i));
        }
    }

    size_t __width()
    {
        size_t size = 0;
        for (auto const &cell : *rows[0]) {
            auto &format = cell.format();
            if (format.borders.left.visiable) {
                size += compute_width(format.borders.left.content, format.locale(),
                                      format.internationlization.multi_bytes_character);
            }
            size += format.borders.left.padding + cell.width() + format.borders.right.padding;
            if (format.borders.right.visiable) {
                size += compute_width(format.borders.right.content, format.locale(),
                                      format.internationlization.multi_bytes_character);
            }
        }
        return size;
    }
};

template <>
inline std::string to_string<Row>(const Row &v)
{
    std::string ret;
    auto lines = v.dump(tabulate::xterm::stringformatter, tabulate::xterm::borderformatter,
                        tabulate::xterm::cornerformatter, true, true);
    for (auto &line : lines) {
        ret += line + NEWLINE;
    }
    return ret;
}

template <>
inline std::string to_string<Table>(const Table &v)
{
    return v.xterm();
}
} // namespace tabulate
