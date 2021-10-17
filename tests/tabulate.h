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
#define PLACEHOLDER   XCONCAT(resevred_, __COUNTER__)

enum class Align { none, left, right, center, top, bottom };

enum class Color { black, red, green, yellow, blue, magenta, cyan, white, PLACEHOLDER, none };

enum class Style {
    // 0 - 9
    none,        // default, VT100
    bold,        // bold, VT100
    faint,       // decreased intensity
    italic,      // italicized
    underline,   // underlined
    blink,       // blink, VT100
    PLACEHOLDER, // placeholder for underlying_type
    inverse,     // inverse, VT100
    invisible,   // hidden
    crossed,     // crossed-out chracters

    // 21 - 29
    doubly_underline, // doubly underlined
    normal,           // neither bold nor faint
    not_italic,       // not italicized
    not_underline,    // not underlined
    steady,           // not blinking
    PLACEHOLDER,      // placeholder for underlying_type
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

using Applier = std::function<std::string(const std::string &, TrueColor, TrueColor, const std::vector<Style> &)>;
} // namespace tabulate

namespace tabulate
{
template <typename T>
inline std::string to_string(const T &v)
{
    /**
     * float/double/long double
     * [unsigned] int/long/long long
     *
     * since c++11
     */
    return std::to_string(v);
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
    ss << "#" << std::hex << v.hex;

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
inline size_t calculate_width(const std::string &text, const std::string &locale, bool wchar_enabled)
{
    if (!wchar_enabled) { return text.length(); }

    if (text.size() == 0) { return 0; }

#if defined(__unix__) || defined(__unix) || defined(__APPLE__)
    {
        // The behavior of wcswidth() depends on the LC_CTYPE category of the current locale.
        // Set the current locale based on cell properties before computing width
        auto old_locale = std::locale::global(std::locale(locale));

        // Convert from narrow std::string to wide string
        wchar_t *wide_string = new wchar_t[text.size()];
        std::mbstowcs(wide_string, text.c_str(), text.size());

        // Compute display width of wide string
        int len = wcswidth(wide_string, text.size());
        delete[] wide_string;

        // Restore old locale
        std::locale::global(old_locale);

        if (len >= 0) { return len; }
    }
#endif

    {
        return (text.length() - std::count_if(text.begin(), text.end(), [](char c) -> bool {
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
            if (index != std::string::npos) { indices.push_back(index); }
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
        while (std::getline(ss, line, '\n')) { lines.push_back(line); }
    }

    std::vector<std::string> wrapped_lines;
    for (auto const &line : lines) {
        std::string wrapped;
        std::vector<std::string> words = explode_string(line, {" ", "-", "\t"});

        for (auto &word : words) {
            if (calculate_width(wrapped, locale, multi_bytes_character)
                    + calculate_width(word, locale, multi_bytes_character)
                > width) {
                if (calculate_width(wrapped, locale, multi_bytes_character) > 0) {
                    wrapped_lines.push_back(wrapped);
                    wrapped = "";
                }

                while (calculate_width(word, locale, multi_bytes_character) > width) {
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

static std::string repeate_to_size(const std::string &s, size_t len)
{
    std::string r;
    size_t swidth = calculate_width(s, "", true);
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
// https://invisible-island.net/xterm/ctlseqs/ctlseqs.html
namespace xterm
{
static const std::string term = std::string(getenv("TERM"));
static const std::vector<std::string> terms_supported_truecolor = {"iterm", "linux", "xterm-truecolor"};
static const bool supported_truecolor =
    std::find(terms_supported_truecolor.begin(), terms_supported_truecolor.end(), term)
    != terms_supported_truecolor.end();

static std::string apply(const std::string &str, TrueColor foreground_color, TrueColor background_color,
                         const std::vector<Style> &styles)
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
            // @PLACEHOLDER
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

    if (have) { applied += "\033[00m"; }

    return applied;
}
} // namespace xterm
} // namespace tabulate

namespace tabulate
{
namespace markdown
{
static std::string apply(const std::string &str, TrueColor foreground_color, TrueColor background_color,
                         const std::vector<Style> &styles)
{
    std::string applied;

    bool have = !foreground_color.none() || !background_color.none() || styles.size() != 0;

    bool bold = false;
    bool crossed = false;
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
                    bold = true;
                    break;
                case Style::crossed:
                    crossed = true;
                    break;
                case Style::italic:
                    applied += "font-style:italic;";
                    break;
                case Style::underline:
                    applied += "text-decoration:underline;";
                    break;
                default:
                    // unsupported, do nothing
                    break;
            }
        }
        applied += "\">";
    }

    if (bold) { applied += "**"; }
    if (crossed) { applied += "~~"; }
    applied += str;
    if (crossed) { applied += "~~"; }
    if (bold) { applied += "**"; }

    if (have) { applied += "</span>"; }

    return applied;
}
} // namespace markdown
} // namespace tabulate

namespace tabulate
{
class Format {
  public:
    Format()
    {
        font.align = Align::left;
        font.color = Color::none;
        font.background_color = Color::none;

        // paddings
        paddings.left.size = 1;
        paddings.right.size = 1;
        paddings.top.size = 0;
        paddings.bottom.size = 0;

        // border-left
        borders.left.visiable = true;
        borders.left.content = "|";
        borders.left.color = Color::none;
        borders.left.background_color = Color::none;

        // border-right
        borders.right.visiable = true;
        borders.right.content = "|";
        borders.right.color = Color::none;
        borders.right.background_color = Color::none;

        // border-top
        borders.top.visiable = true;
        borders.top.content = "-";
        borders.top.color = Color::none;
        borders.top.background_color = Color::none;

        // border-bottom
        borders.bottom.visiable = true;
        borders.bottom.content = "-";
        borders.bottom.color = Color::none;
        borders.bottom.background_color = Color::none;

        // corner-top_left
        corners.top_left.content = "+";
        corners.top_left.color = Color::none;
        corners.top_left.background_color = Color::none;

        // corner-top_right
        corners.top_right.content = "+";
        corners.top_right.color = Color::none;
        corners.top_right.background_color = Color::none;

        // corner-bottom_left
        corners.bottom_left.content = "+";
        corners.bottom_left.color = Color::none;
        corners.bottom_left.background_color = Color::none;

        // corner-bottom_right
        corners.bottom_right.content = "+";
        corners.bottom_right.color = Color::none;
        corners.bottom_right.background_color = Color::none;

        // shape
        shape.width = 0;

        // internationlization
        internationlization.locale = "";
        internationlization.multi_bytes_character = false;
    }

    size_t width() const
    {
        return shape.width;
    }

    Format &width(size_t value)
    {
        shape.width = value;
        return *this;
    }

    Align align() const
    {
        return font.align;
    }

    Format &align(Align value)
    {
        font.align = value;
        return *this;
    }

    TrueColor color() const
    {
        return font.color;
    }

    Format &color(TrueColor value)
    {
        font.color = value;
        return *this;
    }

    TrueColor background_color() const
    {
        return font.background_color;
    }

    Format &background_color(TrueColor value)
    {
        font.background_color = value;
        return *this;
    }

    const std::vector<Style> &styles() const
    {
        return font.styles;
    }

    Format &styles(Style value)
    {
        font.styles.push_back(value);
        return *this;
    }

    template <typename... Args>
    Format &styles(Style style, Args... args)
    {
        font.styles.push_back(style);
        return styles(args...);
    }

    Format &padding(size_t value)
    {
        paddings.left.size = value;
        paddings.right.size = value;
        paddings.top.size = value;
        paddings.bottom.size = value;
        return *this;
    }

    Format &padding_left(size_t value)
    {
        paddings.left.size = value;
        return *this;
    }

    Format &padding_right(size_t value)
    {
        paddings.right.size = value;
        return *this;
    }

    Format &padding_top(size_t value)
    {
        paddings.top.size = value;
        return *this;
    }

    Format &padding_bottom(size_t value)
    {
        paddings.bottom.size = value;
        return *this;
    }

    Format &border(const std::string &value)
    {
        borders.left.content = value;
        borders.right.content = value;
        borders.top.content = value;
        borders.bottom.content = value;
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

  private:
    friend class Cell;
    friend class Row;
    friend class Column;
    friend class Table;

    struct {
        struct Padding {
            size_t size;
        } left, right, top, bottom;
    } paddings;

    // Element padding and Border
    struct {
        struct Border {
            bool visiable;

            TrueColor color;
            std::string content;
            TrueColor background_color;
        } left, right, top, bottom;
    } borders;

    // Element Corner
    struct {
        struct Corner {
            TrueColor color;
            std::string content;
            TrueColor background_color;
        } top_left, top_right, bottom_left, bottom_right;
    } corners;

    struct {
        size_t width; // width limitation
    } shape;

    struct {
        std::string locale;
        bool multi_bytes_character;
    } internationlization;

    struct {
        Align align;
        TrueColor color;
        TrueColor background_color;
        std::vector<Style> styles;
    } font;
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
        return calculate_width(m_content, m_format.locale(), m_format.multi_bytes_character());
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
                    max_width = std::max(max_width, calculate_width(line, m_format.locale(), true));
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

    std::vector<Style> styles() const
    {
        return m_format.styles();
    }

  private:
    Format m_format;
    std::string m_content;
};

template <typename T>
class PtrIterator {
  public:
    explicit PtrIterator(typename std::vector<std::shared_ptr<T>>::iterator ptr) : ptr(ptr) {}

    PtrIterator operator++()
    {
        ++ptr;
        return *this;
    }
    bool operator!=(const PtrIterator &other) const
    {
        return ptr != other.ptr;
    }
    T &operator*()
    {
        return **ptr;
    }

  private:
    typename std::vector<std::shared_ptr<T>>::iterator ptr;
};

template <typename T>
class PtrConstIterator {
  public:
    explicit PtrConstIterator(typename std::vector<std::shared_ptr<T>>::const_iterator ptr) : ptr(ptr) {}

    PtrConstIterator operator++()
    {
        ++ptr;
        return *this;
    }
    bool operator!=(const PtrConstIterator &other) const
    {
        return ptr != other.ptr;
    }
    const T &operator*()
    {
        return **ptr;
    }

  private:
    typename std::vector<std::shared_ptr<T>>::const_iterator ptr;
};

class BatchFormat {
  public:
    BatchFormat(std::vector<std::shared_ptr<Cell>> &cells) : cells(cells) {}

    inline size_t size()
    {
        return cells.size();
    }

    inline BatchFormat &width(size_t value)
    {
        for (auto &cell : cells) { cell->format().width(value); }
        return *this;
    }

    inline BatchFormat &align(Align value)
    {
        for (auto &cell : cells) { cell->format().align(value); }
        return *this;
    }

    inline BatchFormat &color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().color(value); }
        return *this;
    }

    inline BatchFormat &background_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().background_color(value); }
        return *this;
    }

    inline BatchFormat &styles(Style value)
    {
        for (auto &cell : cells) { cell->format().styles(value); }
        return *this;
    }

    template <typename... Args>
    inline BatchFormat &styles(Args... values)
    {
        for (auto &cell : cells) { cell->format().styles(values...); }
        return *this;
    }

    inline BatchFormat &padding(size_t value)
    {
        for (auto &cell : cells) { cell->format().padding(value); }
        return *this;
    }

    inline BatchFormat &padding_left(size_t value)
    {
        for (auto &cell : cells) { cell->format().padding_left(value); }
        return *this;
    }

    inline BatchFormat &padding_right(size_t value)
    {
        for (auto &cell : cells) { cell->format().padding_right(value); }
        return *this;
    }

    inline BatchFormat &padding_top(size_t value)
    {
        for (auto &cell : cells) { cell->format().padding_top(value); }
        return *this;
    }

    inline BatchFormat &padding_bottom(size_t value)
    {
        for (auto &cell : cells) { cell->format().padding_bottom(value); }
        return *this;
    }

    inline BatchFormat &border(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().border(value); }
        return *this;
    }

    inline BatchFormat &border_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().border_color(value); }
        return *this;
    }

    inline BatchFormat &border_background_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().border_background_color(value); }
        return *this;
    }

    inline BatchFormat &border_left(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().border_left(value); }
        return *this;
    }

    inline BatchFormat &border_left_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().border_left_color(value); }
        return *this;
    }

    inline BatchFormat &border_left_background_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().border_left_background_color(value); }
        return *this;
    }

    inline BatchFormat &border_right(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().border_right(value); }
        return *this;
    }

    inline BatchFormat &border_right_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().border_right_color(value); }
        return *this;
    }

    inline BatchFormat &border_right_background_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().border_right_background_color(value); }
        return *this;
    }

    inline BatchFormat &border_top(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().border_top(value); }
        return *this;
    }

    inline BatchFormat &border_top_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().border_top_color(value); }
        return *this;
    }

    inline BatchFormat &border_top_background_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().border_top_background_color(value); }
        return *this;
    }

    inline BatchFormat &border_bottom(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().border_bottom(value); }
        return *this;
    }

    inline BatchFormat &border_bottom_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().border_bottom_color(value); }
        return *this;
    }

    inline BatchFormat &border_bottom_background_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().border_bottom_background_color(value); }
        return *this;
    }

    inline BatchFormat &show_border()
    {
        for (auto &cell : cells) { cell->format().show_border(); }
        return *this;
    }

    inline BatchFormat &hide_border()
    {
        for (auto &cell : cells) { cell->format().hide_border(); }
        return *this;
    }

    inline BatchFormat &show_border_top()
    {
        for (auto &cell : cells) { cell->format().show_border_top(); }
        return *this;
    }

    inline BatchFormat &hide_border_top()
    {
        for (auto &cell : cells) { cell->format().hide_border_top(); }
        return *this;
    }

    inline BatchFormat &show_border_bottom()
    {
        for (auto &cell : cells) { cell->format().show_border_bottom(); }
        return *this;
    }

    inline BatchFormat &hide_border_bottom()
    {
        for (auto &cell : cells) { cell->format().hide_border_bottom(); }
        return *this;
    }

    inline BatchFormat &show_border_left()
    {
        for (auto &cell : cells) { cell->format().show_border_left(); }
        return *this;
    }

    inline BatchFormat &hide_border_left()
    {
        for (auto &cell : cells) { cell->format().hide_border_left(); }
        return *this;
    }

    inline BatchFormat &show_border_right()
    {
        for (auto &cell : cells) { cell->format().show_border_right(); }
        return *this;
    }

    inline BatchFormat &hide_border_right()
    {
        for (auto &cell : cells) { cell->format().hide_border_right(); }
        return *this;
    }

    inline BatchFormat &corner(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().corner(value); }
        return *this;
    }

    inline BatchFormat &corner_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().corner_color(value); }
        return *this;
    }

    inline BatchFormat &corner_background_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().corner_background_color(value); }
        return *this;
    }

    inline BatchFormat &corner_top_left(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().corner_top_left(value); }
        return *this;
    }

    inline BatchFormat &corner_top_left_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().corner_top_left_color(value); }
        return *this;
    }

    inline BatchFormat &corner_top_left_background_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().corner_top_left_background_color(value); }
        return *this;
    }

    inline BatchFormat &corner_top_right(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().corner_top_right(value); }
        return *this;
    }

    inline BatchFormat &corner_top_right_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().corner_top_right_color(value); }
        return *this;
    }

    inline BatchFormat &corner_top_right_background_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().corner_top_right_background_color(value); }
        return *this;
    }

    inline BatchFormat &corner_bottom_left(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().corner_bottom_left(value); }
        return *this;
    }

    inline BatchFormat &corner_bottom_left_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().corner_bottom_left_color(value); }
        return *this;
    }

    inline BatchFormat &corner_bottom_left_background_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().corner_bottom_left_background_color(value); }
        return *this;
    }

    inline BatchFormat &corner_bottom_right(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().corner_bottom_right(value); }
        return *this;
    }

    inline BatchFormat &corner_bottom_right_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().corner_bottom_right_color(value); }
        return *this;
    }

    inline BatchFormat &corner_bottom_right_background_color(TrueColor value)
    {
        for (auto &cell : cells) { cell->format().corner_bottom_right_background_color(value); }
        return *this;
    }

    inline BatchFormat &locale(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().locale(value); }
        return *this;
    }

    inline BatchFormat &multi_bytes_character(bool value)
    {
        for (auto &cell : cells) { cell->format().multi_bytes_character(value); }
        return *this;
    }

  private:
    std::vector<std::shared_ptr<Cell>> &cells;
};

class Row {
  public:
    Row() : m_format(cells) {}

    template <typename... Args>
    Row(Args... args) : m_format(cells)
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
            for (size_t i = 0; i < size; i++) { add(""); }
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

    size_t size() const
    {
        return cells.size();
    }

    // corner padding words padding  corner   padding words padding  corner
    std::string border_top(Applier apply) const
    {
        std::string seperator;

        if (cells.size() > 0) {
            auto &corner = cells[0]->format().corners.top_left;
            seperator += apply(corner.content, corner.color, corner.background_color, {});
        }
        for (auto const &cell : cells) {
            auto &borders = cell->format().borders;
            auto &corner = cell->format().corners.top_right;
            auto &paddings = cell->format().paddings;

            size_t size = paddings.left.size + cell->width() + paddings.right.size;
            seperator +=
                apply(repeate_to_size(borders.top.content, size), borders.top.color, borders.top.background_color, {});
            seperator += apply(corner.content, corner.color, corner.background_color, {});
        }

        // padding top
        if (cells[0]->format().paddings.top.size > 0) {
            std::string padline;
            {
                {
                    auto &corner = cells[0]->format().corners.top_left;
                    padline += apply(corner.content, corner.color, corner.background_color, {});
                }

                for (auto const &cell : cells) {
                    auto &paddings = cell->format().paddings;
                    auto &corner = cell->format().corners.top_right;
                    size_t size = paddings.left.size + cell->width() + paddings.right.size;
                    padline += apply(std::string(size, ' '), Color::none, cell->background_color(), {});
                    padline += apply(corner.content, corner.color, corner.background_color, {});
                }
            }
            // std::cout << "padline: \"" << padline << "\"" << std::endl;
            for (size_t i = 0; i < cells[0]->format().paddings.top.size; i++) { seperator += "\n" + padline; }
        }

        return seperator;
    }

    std::string border_bottom(Applier apply) const
    {
        std::string seperator;

        // padding top
        if (cells[0]->format().paddings.top.size > 0) {
            std::string padline;
            {
                {
                    auto &corner = cells[0]->format().corners.bottom_left;
                    padline += apply(corner.content, corner.color, corner.background_color, {});
                }

                for (auto const &cell : cells) {
                    auto &paddings = cell->format().paddings;
                    auto &corner = cell->format().corners.bottom_right;
                    size_t size = paddings.left.size + cell->width() + paddings.right.size;
                    padline += apply(std::string(size, ' '), Color::none, cell->background_color(), {});
                    padline += apply(corner.content, corner.color, corner.background_color, {});
                }
            }
            for (size_t i = 0; i < cells[0]->format().paddings.top.size; i++) { seperator += padline + "\n"; }
        }

        if (cells.size() > 0) {
            auto &corner = cells[0]->format().corners.bottom_left;
            seperator += apply(corner.content, corner.color, corner.background_color, {});
        }
        for (auto const &cell : cells) {
            auto &borders = cell->format().borders;
            auto &paddings = cell->format().paddings;
            auto &corner = cell->format().corners.bottom_right;

            size_t size = paddings.left.size + cell->width() + paddings.right.size;
            seperator += apply(repeate_to_size(borders.bottom.content, size), borders.bottom.color,
                               borders.bottom.background_color, {});
            seperator += apply(corner.content, corner.color, corner.background_color, {});
        }

        return seperator;
    }

    std::vector<std::string> dump(Applier apply) const
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
                    if (&style != &cell->styles().back()) { std::cout << " "; }
                }
                std::cout << std::endl;
            }
#endif
            std::vector<std::string> wrapped;
            if (cell->width() == 0) {
                wrapped.push_back(cell->get());
            } else {
                wrapped = wrap_to_lines(cell->get(), cell->width(), cell->format().locale(),
                                        cell->format().multi_bytes_character());
            }
            dumplines.push_back(wrapped);
            max_height = std::max(wrapped.size(), max_height);
        }

        std::vector<std::string> lines;
        // border padding words padding sepeartor padding words padding sepeartor border
        for (size_t i = 0; i < max_height; i++) {
            std::string line;

            if (cells.size() > 0) {
                auto const &bl = cells[0]->format().borders.left;
                if (bl.visiable) { line += apply(bl.content, bl.color, bl.background_color, {}); }
            }
            for (size_t j = 0; j < cells.size(); j++) {
                Cell &cell = *cells[j];
                auto &pl = cell.format().paddings.left;
                auto &pr = cell.format().paddings.right;
                auto &br = cell.format().borders.right;
                auto foreground_color = cell.color();
                auto background_color = cell.background_color();
                auto styles = cell.styles();

                line += apply(std::string(pl.size, ' '), Color::none, background_color, {});
                if (dumplines[j].size() <= i) {
                    line += apply(std::string(cell.width(), ' '), Color::none, background_color, styles);
                } else {
                    auto align_line_by = [&](const std::string &str, size_t width, Align align,
                                             const std::string &locale, bool multi_bytes_character) -> std::string {
                        size_t linesize = calculate_width(str, locale, multi_bytes_character);
                        if (linesize >= width) { return apply(str, foreground_color, background_color, styles); }
                        switch (align) {
                            default:
                            case Align::left: {
                                return apply(str, foreground_color, background_color, styles)
                                       + apply(std::string(width - linesize, ' '), Color::none, background_color, {});
                            } break;
                            case Align::right: {
                                return apply(std::string(width - linesize, ' '), Color::none, background_color, {})
                                       + apply(str, foreground_color, background_color, styles);
                            } break;
                            case Align::center: {
                                size_t remains = width - linesize;
                                return apply(std::string(remains / 2, ' '), Color::none, background_color, {})
                                       + apply(str, foreground_color, background_color, styles)
                                       + apply(std::string((remains + 1) / 2, ' '), Color::none, background_color, {});
                            } break;
                        }
                    };

                    line += align_line_by(dumplines[j][i], cell.width(), cell.align(), cell.format().locale(),
                                          cell.format().multi_bytes_character());
                }

                line += apply(std::string(pr.size, ' '), Color::none, background_color, {});
                if (br.visiable) { line += apply(br.content, br.color, br.background_color, {}); }
            }

            lines.push_back(line);
        }

        return lines;
    }

    /* iterator */
    using iterator = PtrIterator<Cell>;
    using const_iterator = PtrConstIterator<Cell>;

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

    BatchFormat &format()
    {
        return m_format;
    }

    const BatchFormat &format() const
    {
        return m_format;
    }

  private:
    BatchFormat m_format;
    std::vector<std::shared_ptr<Cell>> cells;
};

class Column {
  public:
    Column() : formats(cells) {}

    void add(std::shared_ptr<Cell> cell)
    {
        cells.push_back(cell);
    }

    BatchFormat &format()
    {
        return formats;
    }

    const BatchFormat &format() const
    {
        return formats;
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

    /* iterator */
    using iterator = PtrIterator<Cell>;
    using const_iterator = PtrConstIterator<Cell>;

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
    BatchFormat formats;
    std::vector<std::shared_ptr<Cell>> cells;
};

class Table {
  public:
    Table() : formats(cells) {}

    template <typename... Args>
    Table(Args... args) : formats(cells)
    {
        add(args...);
    }

    BatchFormat &format()
    {
        return formats;
    }

    Row &add()
    {
        auto row = std::make_shared<Row>();
        rows.push_back(row);
        __on_add_auto_update();

        return *row;
    }

    template <typename T>
    Row &add(T arg)
    {
        auto row = std::make_shared<Row>();
        row->add(arg);
        rows.push_back(row);
        __on_add_auto_update();

        return *row;
    }

    template <typename... Args>
    Row &add(Args... args)
    {
        auto row = std::make_shared<Row>();
        row->add(args...);
        rows.push_back(row);
        __on_add_auto_update();

        return *row;
    }

    Row &operator[](size_t index)
    {
        if (index >= rows.size()) {
            size_t size = index + 1 - rows.size();
            for (size_t i = 0; i < size; i++) { add(); }
        }
        return *rows[index];
    }

    size_t size()
    {
        return rows.size();
    }

    /* iterator */
    using iterator = PtrIterator<Row>;
    using const_iterator = PtrConstIterator<Row>;

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
                for (size_t i = 0; i < size; i++) { row->add(""); }
            }
            column.add(row->cell(index));
        }

        return column;
    }

    size_t column_size()
    {
        size_t max_size = 0;
        for (auto const &row : rows) { max_size = std::max(max_size, row->size()); }
        return max_size;
    }

    std::string xterm() const
    {
        std::string exported;
        const std::string newline = "\n";

        // add header
        if (rows.size() > 0) {
            const auto &header = *rows[0];
            const Format &format = header[0].format();
            if (format.borders.top.visiable) { exported += header.border_top(xterm::apply) + newline; }
            for (auto const &line : header.dump(xterm::apply)) { exported += line + newline; }
            if (format.borders.bottom.visiable) { exported += header.border_bottom(xterm::apply) + newline; }
        }

        for (size_t i = 1; i < rows.size(); i++) {
            auto const &row = *rows[i];
            for (auto const &line : row.dump(xterm::apply)) { exported += line + newline; }

            auto const &border = row[0].format().borders.bottom;
            if (border.visiable) { exported += row.border_bottom(xterm::apply) + newline; }
        }

        return exported;
    }

    std::string markdown()
    {
        std::string exported;
        const std::string newline = "\n";

        format().border_left("|");
        format().border_right("|");
        format().border_color(Color::none);

        // add header and alignentment
        for (auto const &line : rows[0]->dump(markdown::apply)) {
            // XXX: first row as header
            exported += line + newline;
        }
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
            exported += alignment + newline;
        }

        for (size_t i = 1; i < rows.size(); i++) {
            auto const &row = *rows[i];
            for (auto const &line : row.dump(markdown::apply)) { exported += line + newline; }
        }

        return exported;
    }

  private:
    BatchFormat formats;
    std::vector<std::shared_ptr<Row>> rows;
    std::vector<std::shared_ptr<Cell>> cells; // for batch format

    void __on_add_auto_update()
    {
        // auto update width
        for (size_t i = 0; i < column_size(); i++) {
            Column col = column(i);

            size_t oldwidth = col.size() == 0 ? 0 : col[0].width();
            size_t newwidth = col[col.size() - 1].width();

            if (newwidth != oldwidth) {
                if (newwidth > oldwidth) {
                    col.format().width(newwidth);
                } else {
                    col[col.size() - 1].format().width(oldwidth);
                }
            }
        }

        // append new cells
        Row &last_row = *rows[rows.size() - 1];
        for (size_t i = 0; i < last_row.size(); i++) { cells.push_back(last_row.cell(i)); }
    }
};

template <>
inline std::string to_string<Row>(const Row &v)
{
    auto lines = v.dump(xterm::apply);
    std::string ret;
    ret += v.border_top(xterm::apply);
    for (auto &line : lines) { ret += line + "\n"; }
    ret += v.border_bottom(xterm::apply);
    return ret;
}

template <>
inline std::string to_string<Table>(const Table &v)
{
    return v.xterm();
}
} // namespace tabulate
