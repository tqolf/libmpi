#include <string>
#include <vector>
#include <cctype>
#include <cassert>
#include <sstream>
#include <functional>
#include <locale>
#include <clocale>
#include <wchar.h>
#include <iostream>

// https://www.w3schools.com/css/default.asp
namespace tabulate
{
enum class Align { none, left, right, center, top, bottom };

enum class Color { none, grey, red, green, yellow, blue, magenta, cyan, white };

enum class Style { none, bold, dark, italic, underline, blink, reverse, concealed, crossed };

using Applier = std::function<std::string(const std::string &, Color, Color, const std::vector<Style> &)>;
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
        default:
            return "";
        case Color::grey:
            return "grey";
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
    }
}

template <>
inline std::string to_string<Style>(const Style &v)
{
    switch (v) {
        default:
            return "reset";
        case Style::bold:
            return "bold";
        case Style::dark:
            return "dark";
        case Style::italic:
            return "italic";
        case Style::underline:
            return "underline";
        case Style::blink:
            return "blink";
        case Style::reverse:
            return "reverse";
        case Style::concealed:
            return "concealed";
        case Style::crossed:
            return "crossed";
    }
}
} // namespace tabulate

namespace tabulate
{
inline size_t display_length(const std::string &text, const std::string &locale, bool wchar_enabled)
{
    if (!wchar_enabled) return text.length();

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

// trim white spaces from right end of an input string
static std::string rstrip(const std::string &s)
{
    std::string trimed = s;
    trimed.erase(std::find_if(trimed.rbegin(), trimed.rend(),
                              [](int ch) {
                                  return !std::isspace(ch);
                              })
                     .base(),
                 trimed.end());
    return trimed;
}

// trim white spaces from either end of an input string
static std::string strip(const std::string &s)
{
    return lstrip(rstrip(s));
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
            if (display_length(wrapped, locale, multi_bytes_character)
                    + display_length(word, locale, multi_bytes_character)
                > width) {
                if (display_length(wrapped, locale, multi_bytes_character) > 0) {
                    wrapped_lines.push_back(wrapped);
                    wrapped = "";
                }

                while (display_length(word, locale, multi_bytes_character) > width) {
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

static std::string align_line_by(const std::string &line, size_t width, Align align, const std::string &locale,
                                 bool multi_bytes_character)
{
    size_t linesize = display_length(line, locale, multi_bytes_character);
    if (linesize >= width) { return line; }
    switch (align) {
        default:
        case Align::left: {
            return line + std::string(width - linesize, ' ');
        } break;
        case Align::right: {
            return std::string(width - linesize, ' ') + line;
        } break;
        case Align::center: {
            size_t remains = width - linesize;
            return std::string(remains / 2, ' ') + line + std::string((remains + 1) / 2, ' ');
        } break;
    }
}

static std::string repeate_to_size(const std::string &s, size_t len)
{
    std::string r;
    for (size_t i = 0; i < len;) {
        if (s.size() > len - i) {
            r += s.substr(0, len - i);
        } else {
            r += s;
        }
        i += s.size();
    }
    return r;
}
} // namespace tabulate

namespace tabulate
{
namespace termcolor
{
static const std::string reset = "\033[00m";

std::string get_style(Style style)
{
    static const std::string bold = "\033[1m";
    static const std::string dark = "\033[2m";
    static const std::string italic = "\033[3m";
    static const std::string underline = "\033[4m";
    static const std::string blink = "\033[5m";
    static const std::string reverse = "\033[7m";
    static const std::string concealed = "\033[8m";
    static const std::string crossed = "\033[9m";
    switch (style) {
        default:
            return reset;
        case Style::bold:
            return bold;
        case Style::dark:
            return dark;
        case Style::italic:
            return italic;
        case Style::underline:
            return underline;
        case Style::blink:
            return blink;
        case Style::reverse:
            return reverse;
        case Style::concealed:
            return concealed;
        case Style::crossed:
            return crossed;
    }
}

std::string get_color(Color color)
{
    static const std::string grey = "\033[30m";
    static const std::string red = "\033[31m";
    static const std::string green = "\033[32m";
    static const std::string yellow = "\033[33m";
    static const std::string blue = "\033[34m";
    static const std::string magenta = "\033[35m";
    static const std::string cyan = "\033[36m";
    static const std::string white = "\033[37m";

    switch (color) {
        default:
            return "";
        case Color::grey:
            return grey;
        case Color::red:
            return red;
        case Color::green:
            return green;
        case Color::yellow:
            return yellow;
        case Color::blue:
            return blue;
        case Color::magenta:
            return magenta;
        case Color::cyan:
            return cyan;
        case Color::white:
            return white;
    }
}

std::string get_background_color(Color color)
{
    static const std::string background_grey = "\033[40m";
    static const std::string background_red = "\033[41m";
    static const std::string background_green = "\033[42m";
    static const std::string background_yellow = "\033[43m";
    static const std::string background_blue = "\033[44m";
    static const std::string background_magenta = "\033[45m";
    static const std::string background_cyan = "\033[46m";
    static const std::string background_white = "\033[47m";

    switch (color) {
        default:
            return "";
        case Color::grey:
            return background_grey;
        case Color::red:
            return background_red;
        case Color::green:
            return background_green;
        case Color::yellow:
            return background_yellow;
        case Color::blue:
            return background_blue;
        case Color::magenta:
            return background_magenta;
        case Color::cyan:
            return background_cyan;
        case Color::white:
            return background_white;
    }
}
} // namespace termcolor
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

        // border-left
        borders.left.visiable = true;
        borders.left.padding = 1;

        borders.left.content = "|";

        borders.left.color = Color::none;
        borders.left.backgroud_color = Color::none;

        // border-right
        borders.right.visiable = true;
        borders.right.padding = 1;

        borders.right.content = "|";

        borders.right.color = Color::none;
        borders.right.backgroud_color = Color::none;

        // border-top
        borders.top.visiable = true;
        borders.top.padding = 0;

        borders.top.content = "-";

        borders.top.color = Color::none;
        borders.top.backgroud_color = Color::none;

        // border-bottom
        borders.bottom.visiable = true;
        borders.bottom.padding = 0;

        borders.bottom.content = "-";

        borders.bottom.color = Color::none;
        borders.bottom.backgroud_color = Color::none;

        // corner-top_left
        corners.top_left.content = "+";
        corners.top_left.color = Color::none;
        corners.top_left.backgroud_color = Color::none;

        // corner-top_right
        corners.top_right.content = "+";
        corners.top_right.color = Color::none;
        corners.top_right.backgroud_color = Color::none;

        // corner-bottom_left
        corners.bottom_left.content = "+";
        corners.bottom_left.color = Color::none;
        corners.bottom_left.backgroud_color = Color::none;

        // corner-bottom_right
        corners.bottom_right.content = "+";
        corners.bottom_right.color = Color::none;
        corners.bottom_right.backgroud_color = Color::none;

        // separators
        separators.column = "|";
        separators.color = Color::none;
        separators.backgroud_color = Color::none;

        // shape
        shape.width = 0;

        // internationlization
        internationlization.locale = "";
        internationlization.multi_bytes_character = true;
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

    Color color() const
    {
        return font.color;
    }

    Format &color(Color value)
    {
        font.color = value;
        return *this;
    }

    Color background_color() const
    {
        return font.background_color;
    }

    Format &background_color(Color value)
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

    Format &styles(std::initializer_list<Style> values)
    {
        font.styles.insert(font.styles.end(), values.begin(), values.end());
        return *this;
    }

    Format &padding(size_t value)
    {
        borders.left.padding = value;
        borders.right.padding = value;
        borders.top.padding = value;
        borders.bottom.padding = value;
        return *this;
    }

    Format &padding_left(size_t value)
    {
        borders.left.padding = value;
        return *this;
    }

    Format &padding_right(size_t value)
    {
        borders.right.padding = value;
        return *this;
    }

    Format &padding_top(size_t value)
    {
        borders.top.padding = value;
        return *this;
    }

    Format &padding_bottom(size_t value)
    {
        borders.bottom.padding = value;
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

    Format &border_color(Color value)
    {
        borders.left.color = value;
        borders.right.color = value;
        borders.top.color = value;
        borders.bottom.color = value;
        return *this;
    }

    Format &border_background_color(Color value)
    {
        borders.left.backgroud_color = value;
        borders.right.backgroud_color = value;
        borders.top.backgroud_color = value;
        borders.bottom.backgroud_color = value;
        return *this;
    }

    Format &border_left(const std::string &value)
    {
        borders.left.content = value;
        return *this;
    }

    Format &border_left_color(Color value)
    {
        borders.left.color = value;
        return *this;
    }

    Format &border_left_background_color(Color value)
    {
        borders.left.backgroud_color = value;
        return *this;
    }

    Format &border_right(const std::string &value)
    {
        borders.right.content = value;
        return *this;
    }

    Format &border_right_color(Color value)
    {
        borders.right.color = value;
        return *this;
    }

    Format &border_right_background_color(Color value)
    {
        borders.right.backgroud_color = value;
        return *this;
    }

    Format &border_top(const std::string &value)
    {
        borders.top.content = value;
        return *this;
    }

    Format &border_top_color(Color value)
    {
        borders.top.color = value;
        return *this;
    }

    Format &border_top_background_color(Color value)
    {
        borders.top.backgroud_color = value;
        return *this;
    }

    Format &border_bottom(const std::string &value)
    {
        borders.bottom.content = value;
        return *this;
    }

    Format &border_bottom_color(Color value)
    {
        borders.bottom.color = value;
        return *this;
    }

    Format &border_bottom_background_color(Color value)
    {
        borders.bottom.backgroud_color = value;
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

    Format &corner_color(Color value)
    {
        corners.top_left.color = value;
        corners.top_right.color = value;
        corners.bottom_left.color = value;
        corners.bottom_right.color = value;
        return *this;
    }

    Format &corner_background_color(Color value)
    {
        corners.top_left.backgroud_color = value;
        corners.top_right.backgroud_color = value;
        corners.bottom_left.backgroud_color = value;
        corners.bottom_right.backgroud_color = value;
        return *this;
    }

    Format &corner_top_left(const std::string &value)
    {
        corners.top_left.content = value;
        return *this;
    }

    Format &corner_top_left_color(Color value)
    {
        corners.top_left.color = value;
        return *this;
    }

    Format &corner_top_left_background_color(Color value)
    {
        corners.top_left.backgroud_color = value;
        return *this;
    }

    Format &corner_top_right(const std::string &value)
    {
        corners.top_right.content = value;
        return *this;
    }

    Format &corner_top_right_color(Color value)
    {
        corners.top_right.color = value;
        return *this;
    }

    Format &corner_top_right_background_color(Color value)
    {
        corners.top_right.backgroud_color = value;
        return *this;
    }

    Format &corner_bottom_left(const std::string &value)
    {
        corners.bottom_left.content = value;
        return *this;
    }

    Format &corner_bottom_left_color(Color value)
    {
        corners.bottom_left.color = value;
        return *this;
    }

    Format &corner_bottom_left_background_color(Color value)
    {
        corners.bottom_left.backgroud_color = value;
        return *this;
    }

    Format &corner_bottom_right(const std::string &value)
    {
        corners.bottom_right.content = value;
        return *this;
    }

    Format &corner_bottom_right_color(Color value)
    {
        corners.bottom_right.color = value;
        return *this;
    }

    Format &corner_bottom_right_background_color(Color value)
    {
        corners.bottom_right.backgroud_color = value;
        return *this;
    }

    Format &column_separator(const std::string &value)
    {
        separators.column = value;
        return *this;
    }

    Format &column_separator_color(Color value)
    {
        separators.color = value;
        return *this;
    }

    Format &column_separator_background_color(Color value)
    {
        separators.backgroud_color = value;
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

    // Element padding and Border
    struct {
        struct Border {
            bool visiable;
            size_t padding;

            Color color;
            std::string content;
            Color backgroud_color;
        } left, right, top, bottom;
    } borders;

    // Element Corner
    struct {
        struct Corner {
            Color color;
            std::string content;
            Color backgroud_color;
        } top_left, top_right, bottom_left, bottom_right;
    } corners;

    // Element separator
    struct {
        std::string column;
        Color color, backgroud_color;
    } separators;

    struct {
        size_t width; // width limitation
    } shape;

    struct {
        std::string locale;
        bool multi_bytes_character;
    } internationlization;

    struct {
        Align align;
        Color color;
        Color background_color;
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

    size_t size()
    {
        return display_length(m_content, m_format.locale(), m_format.multi_bytes_character());
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
                    max_width =
                        std::max(max_width, display_length(line, m_format.locale(), m_format.multi_bytes_character()));
                }

                return max_width;
            }
        }
    }

    Align align() const
    {
        return m_format.align();
    }

    Color color() const
    {
        return m_format.color();
    }

    Color background_color() const
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

class CellIterator {
  public:
    explicit CellIterator(std::vector<std::shared_ptr<Cell>>::iterator ptr) : ptr(ptr) {}

    CellIterator operator++()
    {
        ++ptr;
        return *this;
    }
    bool operator!=(const CellIterator &other) const
    {
        return ptr != other.ptr;
    }
    Cell &operator*()
    {
        return **ptr;
    }

  private:
    std::vector<std::shared_ptr<Cell>>::iterator ptr;
};

class CellConstIterator {
  public:
    explicit CellConstIterator(std::vector<std::shared_ptr<Cell>>::const_iterator ptr) : ptr(ptr) {}

    CellConstIterator operator++()
    {
        ++ptr;
        return *this;
    }
    bool operator!=(const CellConstIterator &other) const
    {
        return ptr != other.ptr;
    }
    const Cell &operator*()
    {
        return **ptr;
    }

  private:
    std::vector<std::shared_ptr<Cell>>::const_iterator ptr;
};

class BatchFormat {
  public:
    BatchFormat(std::vector<std::shared_ptr<Cell>> &cells) : cells(cells) {}

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

    inline BatchFormat &color(Color value)
    {
        for (auto &cell : cells) { cell->format().color(value); }
        return *this;
    }

    inline BatchFormat &background_color(Color value)
    {
        for (auto &cell : cells) { cell->format().background_color(value); }
        return *this;
    }

    inline BatchFormat &styles(Style value)
    {
        for (auto &cell : cells) { cell->format().styles(value); }
        return *this;
    }

    inline BatchFormat &styles(std::initializer_list<Style> values)
    {
        for (auto &cell : cells) { cell->format().styles(values); }
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

    inline BatchFormat &border_color(Color value)
    {
        for (auto &cell : cells) { cell->format().border_color(value); }
        return *this;
    }

    inline BatchFormat &border_background_color(Color value)
    {
        for (auto &cell : cells) { cell->format().border_background_color(value); }
        return *this;
    }

    inline BatchFormat &border_left(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().border_left(value); }
        return *this;
    }

    inline BatchFormat &border_left_color(Color value)
    {
        for (auto &cell : cells) { cell->format().border_left_color(value); }
        return *this;
    }

    inline BatchFormat &border_left_background_color(Color value)
    {
        for (auto &cell : cells) { cell->format().border_left_background_color(value); }
        return *this;
    }

    inline BatchFormat &border_right(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().border_right(value); }
        return *this;
    }

    inline BatchFormat &border_right_color(Color value)
    {
        for (auto &cell : cells) { cell->format().border_right_color(value); }
        return *this;
    }

    inline BatchFormat &border_right_background_color(Color value)
    {
        for (auto &cell : cells) { cell->format().border_right_background_color(value); }
        return *this;
    }

    inline BatchFormat &border_top(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().border_top(value); }
        return *this;
    }

    inline BatchFormat &border_top_color(Color value)
    {
        for (auto &cell : cells) { cell->format().border_top_color(value); }
        return *this;
    }

    inline BatchFormat &border_top_background_color(Color value)
    {
        for (auto &cell : cells) { cell->format().border_top_background_color(value); }
        return *this;
    }

    inline BatchFormat &border_bottom(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().border_bottom(value); }
        return *this;
    }

    inline BatchFormat &border_bottom_color(Color value)
    {
        for (auto &cell : cells) { cell->format().border_bottom_color(value); }
        return *this;
    }

    inline BatchFormat &border_bottom_background_color(Color value)
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

    inline BatchFormat &corner_color(Color value)
    {
        for (auto &cell : cells) { cell->format().corner_color(value); }
        return *this;
    }

    inline BatchFormat &corner_background_color(Color value)
    {
        for (auto &cell : cells) { cell->format().corner_background_color(value); }
        return *this;
    }

    inline BatchFormat &corner_top_left(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().corner_top_left(value); }
        return *this;
    }

    inline BatchFormat &corner_top_left_color(Color value)
    {
        for (auto &cell : cells) { cell->format().corner_top_left_color(value); }
        return *this;
    }

    inline BatchFormat &corner_top_left_background_color(Color value)
    {
        for (auto &cell : cells) { cell->format().corner_top_left_background_color(value); }
        return *this;
    }

    inline BatchFormat &corner_top_right(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().corner_top_right(value); }
        return *this;
    }

    inline BatchFormat &corner_top_right_color(Color value)
    {
        for (auto &cell : cells) { cell->format().corner_top_right_color(value); }
        return *this;
    }

    inline BatchFormat &corner_top_right_background_color(Color value)
    {
        for (auto &cell : cells) { cell->format().corner_top_right_background_color(value); }
        return *this;
    }

    inline BatchFormat &corner_bottom_left(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().corner_bottom_left(value); }
        return *this;
    }

    inline BatchFormat &corner_bottom_left_color(Color value)
    {
        for (auto &cell : cells) { cell->format().corner_bottom_left_color(value); }
        return *this;
    }

    inline BatchFormat &corner_bottom_left_background_color(Color value)
    {
        for (auto &cell : cells) { cell->format().corner_bottom_left_background_color(value); }
        return *this;
    }

    inline BatchFormat &corner_bottom_right(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().corner_bottom_right(value); }
        return *this;
    }

    inline BatchFormat &corner_bottom_right_color(Color value)
    {
        for (auto &cell : cells) { cell->format().corner_bottom_right_color(value); }
        return *this;
    }

    inline BatchFormat &corner_bottom_right_background_color(Color value)
    {
        for (auto &cell : cells) { cell->format().corner_bottom_right_background_color(value); }
        return *this;
    }

    inline BatchFormat &column_separator(const std::string &value)
    {
        for (auto &cell : cells) { cell->format().column_separator(value); }
        return *this;
    }

    inline BatchFormat &column_separator_color(Color value)
    {
        for (auto &cell : cells) { cell->format().column_separator_color(value); }
        return *this;
    }

    inline BatchFormat &column_separator_background_color(Color value)
    {
        for (auto &cell : cells) { cell->format().column_separator_background_color(value); }
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
            seperator += apply(corner.content, corner.color, corner.backgroud_color, {});
        }
        for (auto const &cell : cells) {
            auto &borders = cell->format().borders;
            auto &corner = cell->format().corners.top_right;

            size_t size = borders.left.padding + cell->width() + borders.right.padding;
            seperator += repeate_to_size(borders.top.content, size);
            seperator += apply(corner.content, corner.color, corner.backgroud_color, {});
        }

        return seperator;
    }

    std::string border_bottom(Applier apply) const
    {
        std::string seperator;

        if (cells.size() > 0) {
            auto &corner = cells[0]->format().corners.bottom_left;
            seperator += apply(corner.content, corner.color, corner.backgroud_color, {});
        }
        for (auto const &cell : cells) {
            auto &borders = cell->format().borders;
            auto &corner = cell->format().corners.bottom_right;

            size_t size = borders.left.padding + cell->width() + borders.right.padding;
            seperator += repeate_to_size(borders.bottom.content, size);
            seperator += apply(corner.content, corner.color, corner.backgroud_color, {});
        }

        return seperator;
    }

    std::vector<std::string> dump(Applier apply) const
    {
        size_t max_height = 0;
        std::vector<std::vector<std::string>> dumplines;
        for (auto const &cell : cells) {
            std::vector<std::string> formatted;
            if (cell->format().width() == 0) {
                formatted.push_back(align_line_by(cell->get(), cell->width(), cell->align(), cell->format().locale(),
                                                  cell->format().multi_bytes_character()));
            } else {
                for (auto const &wrapped : tabulate::wrap_to_lines(cell->get(), cell->width(), cell->format().locale(),
                                                                   cell->format().multi_bytes_character())) {
                    auto aligned = align_line_by(wrapped, cell->width(), cell->align(), cell->format().locale(),
                                                 cell->format().multi_bytes_character());
                    formatted.push_back(aligned);
                }
            }
            dumplines.push_back(formatted);
            max_height = std::max(formatted.size(), max_height);
        }

        std::vector<std::string> lines;

        // border padding words padding sepeartor padding words padding sepeartor border
        for (size_t i = 0; i < max_height; i++) {
            std::string line;

            if (cells.size() > 0) {
                auto const &border = cells[0]->format().borders.left;
                line += apply(border.content, border.color, border.backgroud_color, {});
            }
            for (size_t j = 0; j < cells.size(); j++) {
                Cell &cell = *cells[j];
                auto &bl = cell.format().borders.left;
                auto &br = cell.format().borders.right;
                auto &sp = cell.format().separators;
                auto foreground_color = cell.color();
                auto background_color = cell.background_color();
                auto styles = cell.styles();

                line += apply(std::string(bl.padding, ' '), bl.color, bl.backgroud_color, {});
                if (dumplines[j].size() <= i) {
                    line += apply(std::string(cell.width(), ' '), foreground_color, background_color, styles);
                } else {
                    line += apply(dumplines[j][i], foreground_color, background_color, styles);
                }

                line += apply(std::string(br.padding, ' '), foreground_color, background_color, {});
                if (j == cells.size() - 1) {
                    line += apply(br.content, br.color, br.backgroud_color, {});
                } else {
                    line += apply(sp.column, sp.color, sp.backgroud_color, {});
                }
            }

            lines.push_back(line);
        }

        return lines;
    }

    /* iterator */
    using iterator = CellIterator;
    using const_iterator = CellConstIterator;

    iterator begin()
    {
        return CellIterator(cells.begin());
    }

    iterator end()
    {
        return CellIterator(cells.end());
    }

    const_iterator begin() const
    {
        return CellConstIterator(cells.cbegin());
    }

    const_iterator end() const
    {
        return CellConstIterator(cells.cend());
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
    using iterator = CellIterator;
    using const_iterator = CellConstIterator;

    iterator begin()
    {
        return CellIterator(cells.begin());
    }

    iterator end()
    {
        return CellIterator(cells.end());
    }

    const_iterator begin() const
    {
        return CellConstIterator(cells.cbegin());
    }

    const_iterator end() const
    {
        return CellConstIterator(cells.cend());
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

    template <typename T>
    Row &add(T arg)
    {
        /**
         * emplace_back return reference since c++17
         */
        rows.emplace_back(arg);
        __on_add_auto_update();
        return rows[rows.size() - 1];
    }

    template <typename... Args>
    Row &add(Args... args)
    {
        /**
         * emplace_back return reference since c++17
         */
        rows.emplace_back(args...);
        __on_add_auto_update();
        return rows[rows.size() - 1];
    }

    Row &operator[](size_t index)
    {
        return rows[index];
    }

    size_t size()
    {
        return rows.size();
    }

    /* iterator */
    using iterator = std::vector<Row>::iterator;
    using const_iterator = std::vector<Row>::const_iterator;

    iterator begin()
    {
        return rows.begin();
    }

    iterator end()
    {
        return rows.end();
    }

    const_iterator begin() const
    {
        return rows.cbegin();
    }

    const_iterator end() const
    {
        return rows.cend();
    }

    Column column(size_t index)
    {
        Column column;
        for (auto &row : rows) { column.add(row.cell(index)); }

        return column;
    }

    size_t column_size()
    {
        size_t max_size = 0;
        for (auto const &row : rows) { max_size = std::max(max_size, row.size()); }
        return max_size;
    }

    std::string plaintext() const
    {
        std::string exported;
        const std::string newline = "\n";

        auto apply = [](const std::string &str, Color foreground_color, Color background_color,
                        const std::vector<Style> &styles) -> std::string {
            std::string applied = termcolor::reset;

            applied += termcolor::get_color(foreground_color);
            applied += termcolor::get_background_color(background_color);

            for (auto const &style : styles) { applied += termcolor::get_style(style); }

            applied += str + termcolor::reset;

            return applied;
        };

        // add header
        if (rows.size() > 0) {
            const auto &header = rows[0];
            const Format &format = header[0].format();
            if (format.borders.top.visiable) { exported += header.border_top(apply) + newline; }
            for (auto const &line : header.dump(apply)) { exported += line + newline; }
            if (format.borders.bottom.visiable) { exported += header.border_bottom(apply) + newline; }
        }

        for (size_t i = 1; i < rows.size(); i++) {
            auto const &row = rows[i];
            for (auto const &line : row.dump(apply)) { exported += line + newline; }

            auto const &border = row[0].format().borders.bottom;
            if (border.visiable) { exported += row.border_bottom(apply) + newline; }
        }

        return exported;
    }

    std::string markdown()
    {
        std::string exported;
        const std::string newline = "\n";

        // m_format.borders.left.content = "|";
        // m_format.borders.right.content = "|";

        // m_format.separators.column = "|";

        auto apply = [](const std::string &str, Color, Color, const std::vector<Style> &) -> std::string {
            return str;
        };

        // add header
        for (auto const &line : rows[0].dump(apply)) { exported += line + newline; }

        // add alignentment row
        {
            Row alignment_row;
            for (auto const &cell : rows[0]) {
                switch (cell.align()) {
                    case Align::left:
                        alignment_row.add(":----");
                        break;
                    case Align::right:
                        alignment_row.add("----:");
                        break;
                    case Align::center:
                        alignment_row.add(":---:");
                        break;
                    default:
                        alignment_row.add("-----");
                        break;
                }
            }
            for (auto const &line : alignment_row.dump(apply)) { exported += line + newline; }
        }

        for (size_t i = 1; i < rows.size(); i++) {
            auto const &row = rows[i];
            for (auto const &line : row.dump(apply)) { exported += line + newline; }
        }

        return exported;
    }

  private:
    BatchFormat formats;
    std::vector<Row> rows;
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
        Row &last_row = rows[rows.size() - 1];
        for (size_t i = 0; i < last_row.size(); i++) { cells.push_back(last_row.cell(i)); }
    }
};

template <>
inline std::string to_string<Table>(const Table &v)
{
    return v.plaintext();
}
} // namespace tabulate

#include <iostream>

int main()
{
    using namespace tabulate;

#if 0
    // 0. Quick Start
    {
        Table universal_constants;

        universal_constants.add("Quantity", "Value");
        universal_constants.add("Characteristic impedance of vacuum", "376.730 313 461... Ω");
        universal_constants.add("Electric constant (permittivity of free space)", "8.854 187 817... × 10⁻¹²F·m⁻¹");
        universal_constants.add("Magnetic constant (permeability of free space)",
                                "4π × 10⁻⁷ N·A⁻² = 1.2566 370 614... × 10⁻⁶ N·A⁻²");
        universal_constants.add("Gravitational constant (Newtonian constant of gravitation)",
                                "6.6742(10) × 10⁻¹¹m³·kg⁻¹·s⁻²");
        universal_constants.add("Planck's constant", "6.626 0693(11) × 10⁻³⁴ J·s");
        universal_constants.add("Dirac's constant", "1.054 571 68(18) × 10⁻³⁴ J·s");
        universal_constants.add("Speed of light in vacuum", "299 792 458 m·s⁻¹");

        universal_constants.column(0).format().width(20);

        std::cout << "Console Table:\n" << universal_constants.plaintext() << std::endl;
        // std::cout << "Markdown Table:\n" << universal_constants.markdown() << std::endl;
    }

    // 1.1 Word Wrapping
    {
        Table table;

        table.add("This paragraph contains a veryveryveryveryveryverylong word. The long word will "
                  "break and word wrap to the next line.",
                  "This paragraph \nhas embedded '\\n' \ncharacters and\n will break\n exactly "
                  "where\n you want it\n to\n break.");

        table[0][0].format().width(20);
        table[0][1].format().width(50);

        std::cout << "Console Table:\n" << table.plaintext() << std::endl;
        // std::cout << "Markdown Table:\n" << table.markdown() << std::endl;
    }

    // Font Alignment
    {
        tabulate::Table movies;

        movies.add("S/N", "Movie Name", "Director", "Estimated Budget", "Release Date");
        movies.add("tt1979376", "Toy Story 4", "Josh Cooley", 200000000, "21 June 2019");
        movies.add("tt3263904", "Sully", "Clint Eastwood", 60000000, "9 September 2016");
        movies.add("tt1535109", "Captain Phillips", "Paul Greengrass", 55000000, " 11 October 2013");

        // center align 'Director' column
        movies.column(2).format().align(Align::center);

        // right align 'Estimated Budget' column
        movies.column(3).format().align(Align::right);

        // right align 'Release Date' column
        movies.column(4).format().align(Align::right);

        // Color header cells
        for (size_t i = 0; i < movies.column_size(); ++i) {
            movies[0][i].format().color(Color::yellow).styles(Style::bold);
        }

        std::cout << "Console Table:\n" << movies.plaintext() << std::endl;
        // std::cout << "Markdown Table:\n" << movies.markdown() << std::endl;
    }

    // Font Styles
    {
        Table styled_table;
        styled_table.add("Bold", "Italic", "Bold & Italic", "Blinking");
        styled_table.add("Underline", "Crossed", "Dark", "Bold, Italic & Underlined");

        styled_table[0][0].format().styles({Style::bold});

        styled_table[0][1].format().styles({Style::italic});

        styled_table[0][2].format().styles({Style::bold, Style::italic});

        styled_table[0][3].format().styles({Style::blink});

        styled_table[1][0].format().styles({Style::underline});

        styled_table[1][1].format().styles({Style::crossed});

        styled_table[1][2].format().styles({Style::dark});

        styled_table[1][3].format().styles({Style::bold, Style::italic, Style::underline});

        std::cout << "Console Table:\n" << styled_table.plaintext() << std::endl;
        // std::cout << "Markdown Table:\n" << styled_table.markdown() << std::endl;
    }
#endif

    // Cell Colors
    {
        Table colors;

        colors.add("Font Color is Red", "Font Color is Blue", "Font Color is Green");
        colors.add("Everything is Red", "Everything is Blue", "Everything is Green");
        colors.add("Font Background is Red", "Font Background is Blue", "Font Background is Green");

        colors[0][0].format().color(Color::red).styles({Style::bold});
        colors[0][1].format().color(Color::blue).styles({Style::bold});
        colors[0][2].format().color(Color::green).styles({Style::bold});

        colors[1][0]
            .format()
            .border_left_color(Color::red)
            .border_left_background_color(Color::red)
            .background_color(Color::red)
            .color(Color::red);

        colors[1][1]
            .format()
            .border_left_color(Color::blue)
            .border_left_background_color(Color::blue)
            .background_color(Color::blue)
            .color(Color::blue);

        colors[1][2]
            .format()
            .border_left_color(Color::green)
            .border_left_background_color(Color::green)
            .background_color(Color::green)
            .color(Color::green)
            .border_right_color(Color::green)
            .border_right_background_color(Color::green);

        colors[2][0].format().background_color(Color::red).styles({Style::bold});
        colors[2][1].format().background_color(Color::blue).styles({Style::bold});
        colors[2][2].format().background_color(Color::green).styles({Style::bold});

        std::cout << "Console Table:\n" << colors.plaintext() << std::endl;
        // std::cout << "Markdown Table:\n" << colors.markdown() << std::endl;
    }

    // Borders and Corners
    {
        Table table;

        table.add("ᛏᚺᛁᛊ ᛁᛊ ᚨ ᛊᛏᛟᚱy ᛟᚠᚨ ᛒᛖᚨᚱ ᚨᚾᛞ\n"
                  "ᚨ ᚹᛟᛚᚠ, ᚹᚺᛟ ᚹᚨᚾᛞᛖᚱᛖᛞ ᛏᚺᛖ\n"
                  "ᚱᛖᚨᛚᛗᛊ ᚾᛁᚾᛖ ᛏᛟ ᚠᚢᛚᚠᛁᛚᛚ ᚨ ᛈᚱᛟᛗᛁᛊᛖ\n"
                  "ᛏᛟ ᛟᚾᛖ ᛒᛖᚠᛟᚱᛖ; ᛏᚺᛖy ᚹᚨᛚᚲ ᛏᚺᛖ\n"
                  "ᛏᚹᛁᛚᛁᚷᚺᛏ ᛈᚨᛏᚺ, ᛞᛖᛊᛏᛁᚾᛖᛞ ᛏᛟ\n"
                  "ᛞᛁᛊcᛟᚹᛖᚱ ᛏᚺᛖ ᛏᚱᚢᛏᚺ\nᛏᚺᚨᛏ ᛁᛊ ᛏᛟ cᛟᛗᛖ.");

        table[0][0]
            .format()
            .multi_bytes_character(true)
            // Font styling
            .styles({Style::bold, Style::dark})
            .align(Align::center)
            .color(Color::red)
            .background_color(Color::yellow)
            // Corners
            .corner_top_left("ᛰ")
            .corner_top_right("ᛯ")
            .corner_bottom_left("ᛮ")
            .corner_bottom_right("ᛸ")
            .corner_top_left_color(Color::cyan)
            .corner_top_right_color(Color::yellow)
            .corner_bottom_left_color(Color::green)
            .corner_bottom_right_color(Color::red)
            // Borders
            .border_top("ᛜ")
            .border_bottom("ᛜ")
            .border_left("ᚿ")
            .border_right("ᛆ")
            .border_left_color(Color::yellow)
            .border_right_color(Color::green)
            .border_top_color(Color::cyan)
            .border_bottom_color(Color::red);

        std::cout << "Console Table:\n" << table.plaintext() << std::endl;
        // std::cout << "Markdown Table:\n" << table.markdown() << std::endl;
    }

    // Range-based Iteration
    {
        Table table;

        table.add("Company", "Contact", "Country");
        table.add("Alfreds Futterkiste", "Maria Anders", "Germany");
        table.add("Centro comercial Moctezuma", "Francisco Chang", "Mexico");
        table.add("Ernst Handel", "Roland Mendel", "Austria");
        table.add("Island Trading", "Helen Bennett", "UK");
        table.add("Laughing Bacchus Winecellars", "Yoshi Tannamuri", "Canada");
        table.add("Magazzini Alimentari Riuniti", "Giovanni Rovelli", "Italy");

        // Set width of cells in each column
        table.column(0).format().width(40);
        table.column(1).format().width(30);
        table.column(2).format().width(30);

        // Iterate over cells in the first row
        for (auto &cell : table[0]) { cell.format().styles(Style::underline).align(Align::center); }

        // Iterator over cells in the first column
        for (auto &cell : table.column(0)) {
            if (cell.get() != "Company") { cell.format().align(Align::right); }
        }

        // Iterate over rows in the table
        size_t index = 0;
        for (auto &row : table) {
            // row.format().styles(Style::bold);

            // Set blue background color for alternate rows
            if (index > 0 && index % 2 == 0) {
                for (auto &cell : row) { cell.format().background_color(Color::blue); }
            }
            index += 1;
        }

        std::cout << "Console Table:\n" << table.plaintext() << std::endl;
        // std::cout << "Markdown Table:\n" << table.markdown() << std::endl;
    }

#if 0
    // Nested Tables
    {
        Table class_diagram;

        // Animal class
        {
            Table animal;
            animal.add("Animal");
            animal[0].format().align(Align::center);

            // Animal properties nested table
            {
                Table animal_properties;
                animal_properties.add("+age: Int");
                animal_properties.add("+gender: String");
                animal_properties.format().width(20);
                animal_properties[1].format().hide_border_top();

                std::cout << "Animal Properties:\n" << animal_properties.plaintext() << std::endl;

                animal.add(animal_properties);
            }

            // Animal methods nested table
            {
                Table animal_methods;
                animal_methods.add("+isMammal()");
                animal_methods.add("+mate()");
                animal_methods.format().width(20);
                animal_methods[1].format().hide_border_top();

                animal.add(animal_methods);
            }
            animal[2].format().hide_border_top();

            std::cout << "Animal:\n" << animal.plaintext() << std::endl;

            class_diagram.add(animal);
        }

        // Add rows in the class diagram for the up-facing arrow
        // THanks to center alignment, these will align just fine
        class_diagram.add("▲");
        class_diagram[1][0].format().hide_border_top().multi_bytes_character(true); // ▲ is multi-byte

        class_diagram.add("|");
        class_diagram.add("|");

        // Duck class
        {
            Table duck;
            duck.add("Duck");

            // Duck proeperties nested table
            {
                Table duck_properties;
                duck_properties.add("+beakColor: String = \"yellow\"");
                duck_properties.format().width(40);

                duck.add(duck_properties);
            }

            // Duck methods nested table
            {
                Table duck_methods;
                duck_methods.add("+swim()");
                duck_methods.add("+quack()");
                duck_methods.format().width(40);
                duck_methods[1].format().hide_border_top();

                duck.add(duck_methods);
            }

            // duck[0].format().align(Align::center);
            // duck[2].format().hide_border_top();

            class_diagram.add(duck);
        }

        // class_diagram[2].format().hide_border_top();
        // class_diagram[3].format().hide_border_top();
        // class_diagram[4].format().hide_border_top();

        // Global styling
        class_diagram.format().styles({Style::bold}).align(Align::center).width(60);

        std::cout << "Console Table:\n" << class_diagram.plaintext() << std::endl;
    }
#endif

    // UTF-8 Support
    {
        Table table;

        table.add("English", "I love you");
        table.add("French", "Je t’aime");
        table.add("Spanish", "Te amo");
        table.add("German", "Ich liebe Dich");
        table.add("Mandarin Chinese", "我爱你");
        table.add("Japanese", "愛してる");
        table.add("Korean", "사랑해 (Saranghae)");
        table.add("Greek", "Σ΄αγαπώ (Se agapo)");
        table.add("Italian", "Ti amo");
        table.add("Russian", "Я тебя люблю (Ya tebya liubliu)");
        table.add("Hebrew", "אני אוהב אותך (Ani ohev otakh)");

        // Column 1 is using mult-byte characters
        table.column(1).format().multi_bytes_character(true);
        table.format().corner("♥").styles({Style::bold}).corner_color(Color::magenta).border_color(Color::magenta);

        std::cout << "Console Table:\n" << table.plaintext() << std::endl;
    }

    return 0;
}
