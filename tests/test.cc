#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <iostream>

#include "logger.h"
#include "tabulate.h"
#include "benchmark.h"

template <typename T>
T reverse(T n);

unsigned char reverse(unsigned char n)
{
#ifdef USE_SMALL_LOOKUP_TABLE
    // clang-format off
    static const unsigned char lookup[16] = {
        0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
        0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf,
    };
    // clang-format on

    // Detailed breakdown of the math
    //  + lookup reverse of bottom nibble
    //  |       + grab bottom nibble
    //  |       |        + move bottom result into top nibble
    //  |       |        |     + combine the bottom and top results
    //  |       |        |     | + lookup reverse of top nibble
    //  |       |        |     | |       + grab top nibble
    //  V       V        V     V V       V
    // (lookup[n&0b1111] << 4) | lookup[n>>4]

    // Reverse the top and bottom nibble then swap them.
    return (lookup[n & 0b1111] << 4) | lookup[n >> 4];
#else
    // clang-format off
    static const unsigned char reversed[] = {
        0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
        0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
        0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
        0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
        0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
        0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
        0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
        0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
        0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
        0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
        0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
        0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
        0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
        0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
        0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
        0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
        0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
        0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
        0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
        0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
        0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
        0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
        0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
        0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
        0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
        0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
        0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
        0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
        0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
        0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
        0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
        0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
    };
    // clang-format on

    return reversed[n];
#endif
}

unsigned int reverse(unsigned int n)
{
    unsigned int m = n;
    unsigned char *p = (unsigned char *)(&m);

    if (sizeof(unsigned int) == 4) {
        return (reverse(p[0]) << 24) | (reverse(p[1]) << 16) | (reverse(p[2]) << 8) | reverse(p[3]);
    } else {
        for (unsigned i = 0; i < sizeof(unsigned int) / 2; i++) {
            unsigned char h = p[i], l = p[sizeof(unsigned int) - 1 - i];
            p[i] = reverse(l);
            p[sizeof(unsigned int) - 1 - i] = reverse(h);
        }

        return m;
    }
}

unsigned int reverse_ref(unsigned int num)
{
    unsigned int count = sizeof(num) * 8 - 1;
    unsigned int reverse_num = num;

    num >>= 1;
    while (num) {
        reverse_num <<= 1;
        reverse_num |= num & 1;
        num >>= 1;
        count--;
    }
    reverse_num <<= count;

    return reverse_num;
}


static int clz(unsigned int x)
{
    static_assert(sizeof(unsigned int) == 4, "unsigned int must be 32 bits");

    // clang-format off
    static const char debruijn32[32] = {
         0, 31,  9, 30,  3,  8, 13, 29,
         2,  5,  7, 21, 12, 24, 28, 19,
         1, 10,  4, 14,  6, 22, 25, 20,
        11, 15, 23, 26, 16, 27, 17, 18,
    };
    // clang-format on
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;

    return debruijn32[x * 0x076be629 >> 27];
}

int merge(unsigned int &merged, unsigned int hi, unsigned int lo)
{
    auto hbits = clz(hi);
    auto lbits = clz(lo);
    merged = lo | reverse(hi);

    return static_cast<int>(lbits + hbits) - static_cast<int>(sizeof(unsigned int) * 8);
}

struct data {
    std::string ma;
    std::string mb;
    std::string mc;
    data(const std::string &a, const std::string &b, const std::string &c) : ma(a), mb(b), mc(c) {}
};

namespace logging
{
template <>
inline std::string to_string<data>(const std::vector<data> &v)
{
    using namespace tabulate;
    Table table("Company", "Contact", "Country");
    table[0].format().align(Align::center);
    for (auto const &item : v) { table.add(item.ma, item.mb, item.mc); }

    // Iterate over rows in the table
    size_t index = 0;
    for (auto &row : table) {
        row.format().styles(Style::bold);

        // Set blue background color for alternate rows
        if (index > 0 && index % 2 == 0) {
            for (auto &cell : row) { cell.format().background_color(Color::blue); }
        }
        index += 1;
    }

    return table.xterm();
}
} // namespace logging

int main()
{
    {
        int a = 1;
        float b = 2.0;
        std::string c = "three";
        bool d = true;
        std::vector<int> e{1, 3, 5, 7, 9};
        std::vector<data> f{
            data("Alfreds Futterkiste", "Maria Anders", "Germany"),
            data("Centro comercial Moctezuma", "Francisco Chang", "Mexico"),
            data("Ernst Handel", "Roland Mendel", "Austria"),
            data("Island Trading", "Helen Bennett", "UK"),
            data("Laughing Bacchus Winecellars", "Yoshi Tannamuri", "Canada"),
            data("Magazzini Alimentari Riuniti", "Giovanni Rovelli", "Italy"),
        };

        enum flags {
            FLAG1 = 0x1,
            FLAG2 = 0x2,
            FLAG3 = 0x4,
        } g = FLAG2,
          h = static_cast<flags>(FLAG1 | FLAG3);

        llogi(a, b, c, d, e, f, f[0].mc, g, h);
    }

    {
        struct {
            unsigned int hi, lo;
        } datas[] = {
            {.hi = 0x01, .lo = 0x1000},
            {.hi = 0x09, .lo = 0x1000},
            {.hi = 0x10, .lo = 0x1000},
            {.hi = 0xF1, .lo = 0x1000},
        };

        std::cout << std::endl;
        for (auto const &data : datas) {
            int rbits;
            unsigned int merged;

            printf("merge(0x%02X, 0x%04X): ", data.hi, data.lo);
            if ((rbits = merge(merged, data.hi, data.lo)) >= 0) {
                printf("0x%08X, remain-bits = %2d\n", merged, rbits);
            } else {
                printf("failed.\n");
            }
        }
    }

    {
        BENCHER(reverse_, DoNotOptimize(reverse(__j)), 20, 20000000);
        BENCHER(reverse_ref_, DoNotOptimize(reverse_ref(__j)), 20, 20000000);

        std::cout << std::endl;
        std::cout << "reverse: avg = " << reverse_avg << ", stddev = " << reverse_stddev << std::endl;
        std::cout << "reverse(ref): avg = " << reverse_ref_avg << ", stddev = " << reverse_ref_stddev << std::endl;
        std::cout << "perf-diff: " << reverse_ref_avg / reverse_avg << std::endl;
    }
}
