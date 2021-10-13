#include <string>
#include <vector>
#include <cctype>
#include <cassert>
#include <sstream>
#include <iostream>
#include <optional>

namespace tabulate
{
// https://www.w3schools.com/css/default.asp

enum class Align { none, left, right, center, top, bottom };

enum class Color { none, grey, red, green, yellow, blue, magenta, cyan, white };

enum class Style { bold, dark, italic, underline, blink, reverse, concealed, crossed };
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

static std::vector<std::string> split_words(const std::string &str, const std::string &delimiters)
{
    std::vector<std::string> words;

    size_t start = 0, end;
    while ((end = str.find_first_of(delimiters, start)) != std::string::npos) {
        if (end > start) {
            if (std::isspace(str[end])) {
                words.push_back(str.substr(start, end - start));
                words.push_back(std::string(1, str[end]));
            } else {
                words.push_back(str.substr(start, end - start + 1));
            }
        }
        start = end + 1;
    }

    if (start < str.length()) { words.push_back(str.substr(start)); }

    return words;
}

static std::vector<std::string> wrap_to_lines(const std::string &str, size_t width)
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
        std::vector<std::string> words = split_words(line, " -\t");

        for (auto &word : words) {
            if (wrapped.size() + word.size() > width) {
                if (wrapped.size() > 0) {
                    wrapped_lines.push_back(wrapped);
                    wrapped = "";
                }

                while (word.size() > width) {
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

static std::string align_line_by(const std::string &line, size_t width, Align align)
{
    switch (align) {
        default:
        case Align::left: {
            return line + std::string(width - line.size(), ' ');
        } break;
        case Align::right: {
            return std::string(width - line.size(), ' ') + line;
        } break;
        case Align::center: {
            size_t remains = width - line.size();
            return std::string(remains / 2, ' ') + line + std::string((remains + 1) / 2, ' ');
        } break;
    }
}

static std::string repeate(const std::string &s, size_t len)
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
class Border {
  public:
    Border() {}

    bool visiable;
    size_t padding;

    std::string content;

    Color color;
    Color backgroud_color;
};

// Outline

class Corner {
  public:
    std::string content;

    Color color;
    Color backgroud_color;
};

class FormatOptions {
  public:
    FormatOptions()
    {
        // border-left
        border.left.visiable = true;
        border.left.padding = 1;

        border.left.content = "|";

        border.left.color = Color::none;
        border.left.backgroud_color = Color::none;

        // border-right
        border.right.visiable = true;
        border.right.padding = 1;

        border.right.content = "|";

        border.right.color = Color::none;
        border.right.backgroud_color = Color::none;

        // border-top
        border.top.visiable = true;
        border.top.padding = 0;

        border.top.content = "-";

        border.top.color = Color::none;
        border.top.backgroud_color = Color::none;

        // border-bottom
        border.bottom.visiable = true;
        border.bottom.padding = 0;

        border.bottom.content = "-";

        border.bottom.color = Color::none;
        border.bottom.backgroud_color = Color::none;

        // corner-top_left
        corner.top_left.content = "+";
        corner.top_left.color = Color::none;
        corner.top_left.backgroud_color = Color::none;

        // corner-top_right
        corner.top_right.content = "+";
        corner.top_right.color = Color::none;
        corner.top_right.backgroud_color = Color::none;

        // corner-bottom_left
        corner.bottom_left.content = "+";
        corner.bottom_left.color = Color::none;
        corner.bottom_left.backgroud_color = Color::none;

        // corner-bottom_right
        corner.bottom_right.content = "+";
        corner.bottom_right.color = Color::none;
        corner.bottom_right.backgroud_color = Color::none;

        // column_separator
        column_separator.separator = "|";
        column_separator.color = Color::none;
        column_separator.backgroud_color = Color::none;
    }

    // Element padding and Border
    struct {
        Border left, right, top, bottom;
    } border;

    struct {
        Corner top_left, top_right, bottom_left, bottom_right;
    } corner;

    // Element column separator
    struct {
        std::string separator;
        Color color, backgroud_color;
    } column_separator;
};

class Format {
  public:
    auto &width() const
    {
        return width_;
    }

    Format &width(size_t value)
    {
        width_ = value;
        return *this;
    }

    auto &align() const
    {
        return align_;
    }

    Format &align(Align value)
    {
        align_ = value;
        return *this;
    }

    auto &color() const
    {
        return color_;
    }

    Format &color(Color value)
    {
        color_ = value;
        return *this;
    }

    auto &background_color() const
    {
        return background_color_;
    }

    Format &background_color(Color value)
    {
        background_color_ = value;
        return *this;
    }

    auto &font_styles() const
    {
        return font_styles_;
    }

    Format &font_styles(Style value)
    {
        if (font_styles_.has_value()) { font_styles_.value().push_back(value); }
        return *this;
    }

  private:
    std::optional<size_t> width_;

    std::optional<Align> align_;
    std::optional<Color> color_;
    std::optional<Color> background_color_;
    std::optional<std::vector<Style>> font_styles_;
};

class Cell {
  public:
    Cell(const std::string &content) : content(content) {}

    const std::string &get() const
    {
        return content;
    }

    size_t width() const
    {
        if (format.width().has_value()) {
            return format.width().value();
        } else {
            return content.size();
        }
    }

    Align align() const
    {
        if (format.align().has_value()) {
            return format.align().value();
        } else {
            return Align::none;
        }
    }

  public:
    Format format;

  private:
    std::string content;
};

class BatchFormat : public Format {
  public:
    BatchFormat(std::vector<Cell *> &cells) : cells_(cells) {}
    BatchFormat &width(size_t value)
    {
        for (auto &cell : cells_) { cell->format.width(value); }
        return *this;
    }

    BatchFormat &align(Align value)
    {
        for (auto &cell : cells_) { cell->format.align(value); }
        return *this;
    }

    BatchFormat &color(Color value)
    {
        for (auto &cell : cells_) { cell->format.color(value); }
        return *this;
    }

    BatchFormat &background_color(Color value)
    {
        for (auto &cell : cells_) { cell->format.background_color(value); }
        return *this;
    }

    BatchFormat &font_styles(Style value)
    {
        for (auto &cell : cells_) { cell->format.font_styles(value); }
        return *this;
    }

  private:
    std::vector<Cell *> &cells_;
};

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

class Cells {
  public:
    Cells() {}

    template <typename... Args>
    Cells(Args... args)
    {
        add(args...);
    }

    template <typename T, template <class> class Container = std::vector>
    explicit Cells(const Container<T> &args)
    {
        for (auto const &arg : args) { add(arg); }
    }

    template <typename T>
    void add(const T v)
    {
        cells.push_back(Cell(to_string(v)));
    }

    template <typename T, typename... Args>
    void add(T v, Args... args)
    {
        add(v);
        add(args...);
    }

    Cell &operator[](size_t index)
    {
        return cells[index];
    }

    size_t size() const
    {
        return cells.size();
    }

    // corner padding words padding  corner   padding words padding  corner
    std::string border_top(const FormatOptions &options) const
    {
        std::string seperator;
        seperator += options.corner.top_left.content;
        for (auto const &cell : cells) {
            seperator += repeate(options.border.top.content,
                                 options.border.left.padding + cell.width() + options.border.right.padding);
            seperator += options.corner.top_right.content;
        }

        return seperator;
    }

    std::string border_bottom(const FormatOptions &options) const
    {
        std::string seperator;
        seperator += options.corner.bottom_left.content;
        for (auto const &cell : cells) {
            seperator += repeate(options.border.bottom.content,
                                 options.border.left.padding + cell.width() + options.border.right.padding);
            seperator += options.corner.bottom_right.content;
        }

        return seperator;
    }

    std::vector<std::string> dump(const FormatOptions &options) const
    {
        Align align = Align::center;

        size_t max_height = 0;
        std::vector<std::vector<std::string>> dumps;
        for (auto const &cell : cells) {
            std::vector<std::string> formatted;
            for (auto const &wrapped : tabulate::wrap_to_lines(cell.get(), cell.width())) {
                formatted.push_back(align_line_by(wrapped, cell.width(), align));
            }
            dumps.push_back(formatted);
            max_height = std::max(formatted.size(), max_height);
        }

        std::vector<std::string> lines;

        // border padding words padding sepeartor padding words padding sepeartor border
        for (size_t i = 0; i < max_height; i++) {
            std::string line;

            line += options.border.left.content;
            for (size_t j = 0; j < cells.size(); j++) {
                line += std::string(options.border.left.padding, ' ');
                if (dumps[j].size() <= i) {
                    line += std::string(cells[j].width(), ' ');
                } else {
                    line += dumps[j][i];
                }
                line += std::string(options.border.right.padding, ' ');
                if (j == cells.size() - 1) {
                    line += options.border.right.content;
                } else {
                    line += options.column_separator.separator;
                }
            }

            lines.push_back(line);
        }

        return lines;
    }

    /* iterator */
    using iterator = std::vector<Cell>::iterator;
    using const_iterator = std::vector<Cell>::const_iterator;

    iterator begin()
    {
        return cells.begin();
    }

    iterator end()
    {
        return cells.end();
    }

    const_iterator begin() const
    {
        return cells.cbegin();
    }

    const_iterator end() const
    {
        return cells.cend();
    }

  private:
    std::vector<Cell> cells;
};

class Column {
  public:
    Column() : format_(cells) {}

    void add(Cell &cell)
    {
        cells.push_back(&cell);
    }

    BatchFormat &format()
    {
        return format_;
    }

    /* iterator */
    using iterator = std::vector<Cell *>::iterator;
    using const_iterator = std::vector<Cell *>::const_iterator;

    iterator begin()
    {
        return cells.begin();
    }

    iterator end()
    {
        return cells.end();
    }

    const_iterator begin() const
    {
        return cells.cbegin();
    }

    const_iterator end() const
    {
        return cells.cend();
    }

  private:
    BatchFormat format_;
    std::vector<Cell *> cells;
};

class Table {
  public:
    Table() {}

    template <typename... Args>
    Table(Args... args)
    {
        add(args...);
    }

    template <typename... Args>
    Cells &add(Args... args)
    {
        /**
         * emplace_back return reference since c++17
         */
        rows.emplace_back(args...);
        update_width();
        return rows[rows.size() - 1];
    }

    Cells &operator[](size_t index)
    {
        return rows[index];
    }

    size_t size()
    {
        return rows.size();
    }

    /* iterator */
    using iterator = std::vector<Cells>::iterator;
    using const_iterator = std::vector<Cells>::const_iterator;

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

        for (auto &row : rows) {
            auto &cell = row[index];
            column.add(cell);
        }

        return column;
    }

    size_t column_size()
    {
        size_t max_size = 0;
        for (auto const &row : rows) { max_size = std::max(max_size, row.size()); }
        return max_size;
    }

  private:
    std::vector<Cells> rows;

    size_t max_width(const Column &column)
    {
        size_t max_width = 0;
        for (auto const &cell : column) { max_width = std::max(max_width, cell->get().size()); }

        return max_width;
    }

    void update_width()
    {
        for (size_t i = 0; i < column_size(); i++) {
            Column col = column(i);

            auto width = max_width(col);
            col.format().width(width);
        }
    }
};

static std::string ExportToMarkdown(Table &table)
{
    std::string exported;
    const std::string newline = "\n";

    FormatOptions options;
    for (auto const &row : table) {
        for (auto const &cell : row) {
            // hide border-top
            // hide border-bottom
            // border-left: "|"
            // border-right: "|"
            // column_separator: "|"
            // corner: "|"
        }
    }

    // add header
    exported += table[0].border_top(options) + newline;
    for (auto const &line : table[0].dump(options)) { exported += line + newline; }
    exported += table[0].border_bottom(options) + newline;

    // add alignentment row
    {
        Cells alignment_row;
        for (auto const &cell : table[0]) {
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
        for (auto const &line : alignment_row.dump(options)) { exported += line + newline; }
    }

    for (size_t i = 1; i < table.size(); i++) {
        auto const &row = table[i];
        for (auto const &line : row.dump(options)) { exported += line + newline; }
        exported += row.border_bottom(options) + newline;
    }

    return exported;
}

static std::string ExportToPlainText(Table &table)
{
    std::string exported;
    const std::string newline = "\n";

    FormatOptions options;
    for (auto const &row : table) {
        for (auto const &cell : row) {
            // hide border-top
            // hide border-bottom
            // border-left: "|"
            // border-right: "|"
            // column_separator: "|"
            // corner: "|"
        }
    }

    // add header
    exported += table[0].border_top(options) + newline;
    for (auto const &line : table[0].dump(options)) { exported += line + newline; }
    exported += table[0].border_bottom(options) + newline;

    for (size_t i = 1; i < table.size(); i++) {
        auto const &row = table[i];
        for (auto const &line : row.dump(options)) { exported += line + newline; }
        exported += row.border_bottom(options) + newline;
    }

    return exported;
}


} // namespace tabulate

#include <iostream>
#include <unistd.h>
#include <ctime>
#include <cstdlib>

static std::string randstr(const int len)
{
    std::string tmp_s;
    static const char alphanum[] = "0123456789"
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz"
                                   " -\n";

    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) { tmp_s += alphanum[std::rand() % (sizeof(alphanum) - 1)]; }

    return tmp_s;
}

int main()
{
    using namespace tabulate;
    {
        tabulate::Table movies("S/N", "Movie Name", "Director", "Estimated Budget", "Release Date");

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
        for (size_t i = 0; i < 5; ++i) {
            // header
            // movies[0][i].format().color(Color::yellow).font_style(Style::bold);
        }

        std::cout << "Console Table:\n" << ExportToPlainText(movies) << std::endl;
        std::cout << "Markdown Table:\n" << ExportToMarkdown(movies) << std::endl;
    }

    return 0;
}
