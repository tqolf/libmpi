/**
 * Copyright 2021 Ethan.cr.yp.to
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
#include <mpi/mpi.h>
#include <mpn/mpn-binary.h>
#include <mpn/mpn-montgomery.h>

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wconversion"
#elif defined(__clang__)
#pragma clang diagnostic ignored "-Wundef"
#pragma clang diagnostic ignored "-Wconversion"
#endif

#include <openssl/bn.h>
#include <openssl/rand.h>
#include <gtest/gtest.h>

#include <string>
#include <iostream>
#include <unordered_map>

template <typename T>
class printer {
  public:
    static std::ostream &print(std::ostream &os, const T t)
    {
        return os << t;
    }
};

template <>
std::ostream &printer<BIGNUM *>::print(std::ostream &os, BIGNUM *const v)
{
    char *str = BN_bn2hex(v);
    if (str == NULL) {
        return os << "(Out of memory)";
    } else {
        os << std::string(str);
        free(str);
        return os;
    }
}

template <>
std::ostream &printer<mpi_t *>::print(std::ostream &os, mpi_t *const v)
{
    char *str = mpi_to_string(v);
    if (str == NULL) {
        return os << "(Out of memory)";
    } else {
        os << std::string(str);
        free(str);
        return os;
    }
}

template <>
std::ostream &printer<std::vector<unsigned char>>::print(std::ostream &os, std::vector<unsigned char> const vec)
{
    for (auto ch : vec) {
        const char HEX_CHARS[] = "0123456789ABCDEF";
        os << HEX_CHARS[ch >> 4] << HEX_CHARS[ch & 0x0F];
    }
    return os;
}

class verifier {
  public:
    verifier(verifier const &) = delete;
    verifier &operator=(verifier const &) = delete;

    static verifier *get(void)
    {
        static verifier instance;
        return &instance;
    }

    void set_verbose(bool verbose)
    {
        m_verbose = verbose;
    }

    bool erase(std::string const &key)
    {
        return m_nodes.erase(key) != 0;
    }

    bool probe(std::string key, const std::vector<unsigned char> &buffer)
    {
        const ::testing::TestInfo *const testinfo = ::testing::UnitTest::GetInstance()->current_test_info();

        if (m_nodes.find(key) != m_nodes.end()) {
            std::vector<unsigned char> &r = m_nodes[key];
            if (r.size() == buffer.size() && memcmp(r.data(), buffer.data(), r.size()) == 0) {
                std::cout << testinfo->test_suite_name() << "." << testinfo->name() << ":" << testinfo->line()
                          << ": \033[0;1;32mPassed\033[0m" << std::endl;
                return true;
            } else {
                hexdump("expected", r);
                hexdump("computed", buffer);
                std::cout << testinfo->test_suite_name() << "." << testinfo->name() << ":" << testinfo->line()
                          << ": \033[0;1;31mFailed\033[0m" << std::endl;

                return false;
            }
        } else {
            m_nodes[key] = buffer;

            return true;
        }
    }

    bool probe(std::string const &key, const BIGNUM *a)
    {
        std::vector<unsigned char> buffer(BN_num_bytes(a), 0);
        BN_bn2bin(a, buffer.data());

        return probe(key, buffer);
    }

    bool probe(std::string const &key, const mpi_t *a)
    {
        std::vector<unsigned char> buffer(mpi_bytes(a), 0);
        mpi_to_octets(a, buffer.data(), buffer.max_size(), NULL);

        return probe(key, buffer);
    }

    template <class T>
    void trace(std::string const &name, const T v)
    {
        if (m_verbose) { printer<T>::print(std::cout << name << ": ", v) << std::endl; }
    }

    void hexdump(std::string const &name, std::vector<unsigned char> const &vec)
    {
        std::cout << name << "(" << vec.size() << " bytes): ";
        printer<std::vector<unsigned char>>::print(std::cout, vec) << std::endl;
    }

  private:
    bool m_verbose;
    std::unordered_map<std::string, std::vector<unsigned char>> m_nodes;

    verifier() : m_verbose(false) {}
};

const unsigned int MIN_BITS = 512;
const unsigned int MAX_BITS = 4096;
const unsigned int MIN_BYTES = MPN_BITS_TO_BYTES(MIN_BITS); // min bytes of input MPIs
const unsigned int MAX_BYTES = MPN_BITS_TO_BYTES(MAX_BITS); // max bytes of input MPIs

static std::string random_hex_string(size_t len)
{
    std::string s;
    s.reserve(len);
    srand((unsigned)time(NULL) * getpid());
    static const char alphanum[] = "0123456789ABCDEFabcdef";
    for (size_t i = 0; i < len; ++i) { s += alphanum[rand() % (sizeof(alphanum) - 1)]; }

    return s;
}

static size_t random_size(unsigned int min = MIN_BYTES, unsigned int max = MAX_BYTES)
{
    size_t size;
    RAND_bytes((unsigned char *)&size, sizeof(size));
    return size % (max - min) + min;
}

TEST(MACRO, NARG)
{
    EXPECT_EQ(MPI_NARG(), 0);
    EXPECT_EQ(MPI_NARG(1), 1);
    EXPECT_EQ(MPI_NARG(1, 2, 3.0, "4", std::string("5")), 5);
}

// mpi construction and destruction
// create from hex-string
TEST(MPI, CreateFromString)
{
    size_t size = random_size(0, MAX_BYTES * 16);
    std::string val = std::string(((size * size) & 0x01) ? "-" : ""); // random sign
    val += random_hex_string(size);

    // openssl
    {
        BIGNUM *r = BN_new(); // BN_new to make sure no "Segmentation fault" error when generated string is empty
        BN_hex2bn(&r, val.c_str());
        verifier::get()->trace("* a", r);
        verifier::get()->erase("from_string");
        ASSERT_TRUE(verifier::get()->probe("from_string", r));
        BN_free(r);
    }

    // this implementation
    {
        mpi_t *r = NULL;
        mpi_from_string(&r, val.c_str());
        verifier::get()->trace("  a", r);
        EXPECT_TRUE(verifier::get()->probe("from_string", r));
        mpi_destory(r);
    }
}

// create from octets
TEST(MPI, CreateFromOctets)
{
    std::vector<unsigned char> buffer;
    {
        size_t size = random_size(0, MAX_BYTES);
        buffer.resize(size);
        RAND_bytes(buffer.data(), static_cast<int>(size));

        verifier::get()->trace("a", buffer);
    }

    // openssl
    {
        BIGNUM *r = BN_new();
        BN_bin2bn(buffer.data(), static_cast<int>(buffer.size()), r);
        verifier::get()->trace("* a", r);

        verifier::get()->erase("from_octets");
        ASSERT_TRUE(verifier::get()->probe("from_octets", r));
        BN_free(r);
    }

    // this implementation
    {
        mpi_t *r = NULL;
        mpi_from_octets(&r, buffer.data(), buffer.size());
        verifier::get()->trace("  a", r);

        EXPECT_TRUE(verifier::get()->probe("from_octets", r));
        mpi_destory(r);
    }
}

// multiple-precision integer arithmetic
// addition
TEST(MPI, Addition)
{
    std::vector<unsigned char> abuffer, bbuffer;
    {
        size_t size;
        {
            size = random_size();
            abuffer.resize(size);
            RAND_bytes(abuffer.data(), static_cast<int>(size));

            verifier::get()->trace("a", abuffer);
        }

        {
            size = random_size();
            bbuffer.resize(size);
            RAND_bytes(bbuffer.data(), static_cast<int>(size));

            verifier::get()->trace("b", bbuffer);
        }
    }

    // openssl
    {
        BIGNUM *r = BN_new(), *a = BN_new(), *b = BN_new();
        BN_bin2bn(abuffer.data(), static_cast<int>(abuffer.size()), a);
        BN_bin2bn(bbuffer.data(), static_cast<int>(bbuffer.size()), b);

        BN_add(r, a, b);
        verifier::get()->trace("* r = a + b", r);
        verifier::get()->erase("r = a + b");
        ASSERT_TRUE(verifier::get()->probe("r = a + b", r));

        BN_free(a);
        BN_free(b);
        BN_free(r);
    }

    // this implementation
    {
        mpi_t *a = mpi_create(abuffer.size() * BITS_PER_BYTE);
        mpi_t *b = mpi_create(bbuffer.size() * BITS_PER_BYTE);
        mpi_from_octets(&a, abuffer.data(), abuffer.size());
        mpi_from_octets(&b, bbuffer.data(), bbuffer.size());

        size_t sz = (mpi_bits(a) >= mpi_bits(b) ? mpi_bits(a) : mpi_bits(b)) + 1;
        mpi_t *r = mpi_create(sz);

        mpi_add(r, a, b);
        verifier::get()->trace("  r = a + b", r);
        EXPECT_TRUE(verifier::get()->probe("r = a + b", r));

        mpi_destory(a);
        mpi_destory(b);
        mpi_destory(r);
    }
}

// addition-assign
TEST(MPI, AdditionAssign)
{
    std::vector<unsigned char> rbuffer, abuffer;
    {
        size_t size;
        {
            size = random_size();
            rbuffer.resize(size);
            RAND_bytes(rbuffer.data(), static_cast<int>(size));

            verifier::get()->trace("a", rbuffer);
        }

        {
            size = random_size();
            abuffer.resize(size);
            RAND_bytes(abuffer.data(), static_cast<int>(size));

            verifier::get()->trace("b", abuffer);
        }
    }

    // openssl
    {
        BIGNUM *r = BN_new(), *b = BN_new();
        BN_bin2bn(rbuffer.data(), static_cast<int>(rbuffer.size()), r);
        BN_bin2bn(abuffer.data(), static_cast<int>(abuffer.size()), b);

        BN_add(r, r, b);
        verifier::get()->trace("* r += a", r);
        verifier::get()->erase("r += a");
        ASSERT_TRUE(verifier::get()->probe("r += a", r));

        BN_free(b);
        BN_free(r);
    }

    // this implementation
    {
        /* @test: create first to avoid test failure if octest buffer is empty */
        mpi_t *r = mpi_create(MAX_BYTES * BITS_PER_BYTE);
        mpi_t *b = mpi_create(MAX_BYTES * BITS_PER_BYTE);
        mpi_from_octets(&r, rbuffer.data(), rbuffer.size());
        mpi_from_octets(&b, abuffer.data(), abuffer.size());

        size_t sz = (mpi_bits(r) >= mpi_bits(b) ? mpi_bits(r) : mpi_bits(b)) + 1;
        r = mpi_expand(r, sz);

        mpi_add(r, r, b);
        verifier::get()->trace("  r += a", r);
        EXPECT_TRUE(verifier::get()->probe("r += a", r));

        mpi_destory(b);
        mpi_destory(r);
    }
}

// subtraction
TEST(MPI, Subtraction)
{
    std::vector<unsigned char> abuffer, bbuffer;
    {
        size_t size;
        {
            size = random_size();
            abuffer.resize(size);
            RAND_bytes(abuffer.data(), static_cast<int>(size));

            verifier::get()->trace("a", abuffer);
        }

        {
            size = random_size();
            bbuffer.resize(size);
            RAND_bytes(bbuffer.data(), static_cast<int>(size));

            verifier::get()->trace("b", bbuffer);
        }
    }

    // openssl
    {
        BIGNUM *r = BN_new(), *a = BN_new(), *b = BN_new();
        BN_bin2bn(abuffer.data(), static_cast<int>(abuffer.size()), a);
        BN_bin2bn(bbuffer.data(), static_cast<int>(bbuffer.size()), b);

        BN_sub(r, a, b);
        verifier::get()->trace("* r = a - b", r);
        verifier::get()->erase("r = a - b");
        ASSERT_TRUE(verifier::get()->probe("r = a - b", r));

        BN_free(a);
        BN_free(b);
        BN_free(r);
    }

    // this implementation
    {
        /* @test: create first to avoid test failure if octest buffer is empty */
        mpi_t *a = mpi_create(MAX_BYTES * BITS_PER_BYTE);
        mpi_t *b = mpi_create(MAX_BYTES * BITS_PER_BYTE);
        mpi_from_octets(&a, abuffer.data(), abuffer.size());
        mpi_from_octets(&b, bbuffer.data(), bbuffer.size());

        size_t sz = (mpi_bits(a) >= mpi_bits(b) ? mpi_bits(a) : mpi_bits(b));
        mpi_t *r = mpi_create(sz);

        mpi_sub(r, a, b);
        verifier::get()->trace("  r = a - b", r);
        EXPECT_TRUE(verifier::get()->probe("r = a - b", r));

        mpi_destory(a);
        mpi_destory(b);
        mpi_destory(r);
    }
}

TEST(MPI, SubtractionAssign)
{
    std::vector<unsigned char> rbuffer, bbuffer;
    {
        size_t size;
        {
            size = random_size();
            rbuffer.resize(size);
            RAND_bytes(rbuffer.data(), static_cast<int>(size));

            verifier::get()->trace("r", rbuffer);
        }

        {
            size = random_size();
            bbuffer.resize(size);
            RAND_bytes(bbuffer.data(), static_cast<int>(size));

            verifier::get()->trace("b", bbuffer);
        }
    }

    // openssl
    {
        BIGNUM *r = BN_new(), *b = BN_new();
        BN_bin2bn(rbuffer.data(), static_cast<int>(rbuffer.size()), r);
        BN_bin2bn(bbuffer.data(), static_cast<int>(bbuffer.size()), b);

        BN_sub(r, r, b);
        verifier::get()->trace("* r -= b", r);
        verifier::get()->erase("r -= b");
        ASSERT_TRUE(verifier::get()->probe("r -= b", r));

        BN_free(b);
        BN_free(r);
    }

    // this implementation
    {
        /* @test: create first to avoid test failure if octest buffer is empty */
        mpi_t *r = mpi_create(MAX_BYTES * BITS_PER_BYTE);
        mpi_t *b = mpi_create(MAX_BYTES * BITS_PER_BYTE);
        mpi_from_octets(&r, rbuffer.data(), rbuffer.size());
        mpi_from_octets(&b, bbuffer.data(), bbuffer.size());

        size_t sz = (mpi_bits(r) >= mpi_bits(b) ? mpi_bits(r) : mpi_bits(b));
        r = mpi_expand(r, sz);

        mpi_sub(r, r, b);
        verifier::get()->trace("  r -= b", r);
        EXPECT_TRUE(verifier::get()->probe("r -= b", r));

        mpi_destory(b);
        mpi_destory(r);
    }
}

// multiplication
TEST(MPI, Multiplication)
{
    std::vector<unsigned char> abuffer, bbuffer;
    {
        size_t size;
        {
            size = random_size(0, MAX_BYTES);
            abuffer.resize(size);
            RAND_bytes(abuffer.data(), static_cast<int>(size));

            verifier::get()->trace("a", abuffer);
        }

        {
            size = random_size(0, MAX_BYTES);
            bbuffer.resize(size);
            RAND_bytes(bbuffer.data(), static_cast<int>(size));

            verifier::get()->trace("b", bbuffer);
        }
    }

    // openssl
    {
        BIGNUM *r = BN_new(), *a = BN_new(), *b = BN_new();
        BN_bin2bn(abuffer.data(), static_cast<int>(abuffer.size()), a);
        BN_bin2bn(bbuffer.data(), static_cast<int>(bbuffer.size()), b);

        BN_CTX *ctx = BN_CTX_new();
        BN_mul(r, a, b, ctx);
        BN_CTX_free(ctx);
        verifier::get()->trace("* r = a * b", r);
        verifier::get()->erase("r = a * b");
        ASSERT_TRUE(verifier::get()->probe("r = a * b", r));

        BN_free(a);
        BN_free(b);
        BN_free(r);
    }

    // this implementation
    {
        /* @test: create first to avoid test failure if octest buffer is empty */
        mpi_t *a = mpi_create(abuffer.size() * BITS_PER_BYTE);
        mpi_t *b = mpi_create(bbuffer.size() * BITS_PER_BYTE);
        mpi_from_octets(&a, abuffer.data(), abuffer.size());
        mpi_from_octets(&b, bbuffer.data(), bbuffer.size());

        size_t sz = mpi_bits(a) + mpi_bits(b) + MPN_LIMB_BITS;
        mpi_t *r = mpi_create(sz);

        mpi_mul(r, a, b);
        verifier::get()->trace("  r = a * b", r);
        EXPECT_TRUE(verifier::get()->probe("r = a * b", r));

        mpi_destory(a);
        mpi_destory(b);
        mpi_destory(r);
    }
}

// square
TEST(MPI, Square)
{
    std::vector<unsigned char> buffer;
    {
        size_t size = random_size(0, MAX_BYTES);
        buffer.resize(size);
        RAND_bytes(buffer.data(), static_cast<int>(size));
        verifier::get()->trace("a", buffer);
    }

    // openssl
    {
        BIGNUM *r = BN_new(), *a = BN_new();
        BN_bin2bn(buffer.data(), static_cast<int>(buffer.size()), a);

        BN_CTX *ctx = BN_CTX_new();
        BN_sqr(r, a, ctx);
        BN_CTX_free(ctx);
        verifier::get()->trace("* r = a ^ 2", r);
        verifier::get()->erase("r = a ^ 2");
        ASSERT_TRUE(verifier::get()->probe("r = a ^ 2", r));

        BN_free(a);
        BN_free(r);
    }

    // this implementation
    {
        /* @test: create first to avoid test failure if octest buffer is empty */
        mpi_t *a = mpi_create(buffer.size() * BITS_PER_BYTE);
        mpi_from_octets(&a, buffer.data(), buffer.size());

        mpi_t *r = mpi_create((a != NULL ? a->size : 0) * 2 * MPN_LIMB_BITS);
        mpi_sqr(r, a);
        verifier::get()->trace("  r = a ^ 2", r);
        EXPECT_TRUE(verifier::get()->probe("r = a ^ 2", r));

        mpi_destory(a);
        mpi_destory(r);
    }
}

// division and modular
TEST(MPI, Division)
{
    std::vector<unsigned char> abuffer, bbuffer;
    {
        size_t size;
        {
            size = random_size(0, MAX_BYTES);
            abuffer.resize(size);
            RAND_bytes(abuffer.data(), static_cast<int>(size));
        }

        {
            size = random_size(0, MAX_BYTES);
            bbuffer.resize(size);
            RAND_bytes(bbuffer.data(), static_cast<int>(size));
        }

        // it's too simple for the division algorithm if abufflen < bbufflen, so swap them to make sure that
        // abufflen >= bbufflen
        if (abuffer.size() < bbuffer.size()) { abuffer.swap(bbuffer); }

        verifier::get()->trace("a", abuffer);
        verifier::get()->trace("b", bbuffer);
    }

    // openssl
    {
        BIGNUM *q = BN_new(), *r = BN_new(), *a = BN_new(), *b = BN_new();
        BN_bin2bn(abuffer.data(), static_cast<int>(abuffer.size()), a);
        BN_bin2bn(bbuffer.data(), static_cast<int>(bbuffer.size()), b);

        BN_CTX *ctx = BN_CTX_new();
        BN_div(q, r, a, b, ctx);
        BN_CTX_free(ctx);
        verifier::get()->trace("* q = a // b", q);
        verifier::get()->trace("* r = a % b", r);
        verifier::get()->erase("q = a // b");
        verifier::get()->erase("r = a % b");
        ASSERT_TRUE(verifier::get()->probe("q = a // b", q));
        ASSERT_TRUE(verifier::get()->probe("r = a % b", r));

        BN_free(a);
        BN_free(b);
        BN_free(q);
        BN_free(r);
    }

    // this implementation
    {
        mpi_t *a = mpi_create(abuffer.size() * BITS_PER_BYTE);
        mpi_t *b = mpi_create(bbuffer.size() * BITS_PER_BYTE);
        mpi_from_octets(&a, abuffer.data(), abuffer.size());
        mpi_from_octets(&b, bbuffer.data(), bbuffer.size());

        size_t qsize = 0, rsize = mpi_bits(b);
        if (mpi_bits(a) >= mpi_bits(b)) {
            rsize = mpi_bits(a) + MPN_LIMB_BITS;
            qsize = mpi_bits(a) - mpi_bits(b) + MPN_LIMB_BITS;
        }

        mpi_t *q = mpi_create(qsize);
        mpi_t *r = mpi_create(rsize);

        verifier::get()->trace("bits(a)", mpi_bits(a));
        verifier::get()->trace("bits(b)", mpi_bits(b));
        verifier::get()->trace("max_bits(q)", mpi_max_bits(q));
        verifier::get()->trace("max_bits(r)", mpi_max_bits(r));

        mpi_div(q, r, a, b);
        verifier::get()->trace("  q = a // b", q);
        verifier::get()->trace("  r = a % b", r);
        EXPECT_TRUE(verifier::get()->probe("q = a // b", q));
        EXPECT_TRUE(verifier::get()->probe("r = a % b", r));

        mpi_destory(a);
        mpi_destory(b);
        mpi_destory(q);
        mpi_destory(r);
    }
}

// left-shift
TEST(MPI, LeftShift)
{
    unsigned int n = 0;
    std::vector<unsigned char> buffer;
    {
        size_t size = random_size(0, MAX_BYTES);
        buffer.resize(size);
        RAND_bytes(buffer.data(), static_cast<int>(size));

        verifier::get()->trace("a", buffer);

        n = random_size(0, MAX_BYTES * BITS_PER_BYTE / 3);
        verifier::get()->trace("shifts", n);
    }

    // openssl
    {
        BIGNUM *r = BN_new();
        BN_bin2bn(buffer.data(), static_cast<int>(buffer.size()), r);
        BN_lshift(r, r, n);
        verifier::get()->trace("* r = a << n", r);

        verifier::get()->erase("r = a << n");
        ASSERT_TRUE(verifier::get()->probe("r = a << n", r));
        BN_free(r);
    }

    // this implementation
    {
        mpi_t *t = NULL;
        mpi_from_octets(&t, buffer.data(), buffer.size());

        mpi_t *r = mpi_create(buffer.size() * BITS_PER_BYTE + n);

        mpi_lshift(r, t, n);
        verifier::get()->trace("  r = a << n", r);
        verifier::get()->probe("r = a << n", r);

        mpi_destory(t);
        mpi_destory(r);
    }
}

// left-shift-and-assign
TEST(MPI, LeftShiftAssign)
{
    unsigned int n = 0;
    std::vector<unsigned char> buffer;
    {
        size_t size = random_size(0, MAX_BYTES);
        buffer.resize(size);
        RAND_bytes(buffer.data(), static_cast<int>(size));

        verifier::get()->trace("a", buffer);

        n = random_size(0, MAX_BYTES * BITS_PER_BYTE / 3);
        verifier::get()->trace("shifts", n);
    }

    // openssl
    {
        BIGNUM *r = BN_new();
        BN_bin2bn(buffer.data(), static_cast<int>(buffer.size()), r);

        BN_lshift(r, r, n);
        verifier::get()->trace("* r <<= n", r);
        verifier::get()->erase("r <<= n");
        ASSERT_TRUE(verifier::get()->probe("r <<= n", r));

        BN_free(r);
    }

    // this implementation
    {
        mpi_t *r = mpi_create(buffer.size() * BITS_PER_BYTE + n);
        mpi_from_octets(&r, buffer.data(), buffer.size());

        mpi_lshift(r, r, n);
        verifier::get()->trace("  r <<= n", r);
        EXPECT_TRUE(verifier::get()->probe("r <<= n", r));

        mpi_destory(r);
    }
}

// right-shift
TEST(MPI, RightShift)
{
    unsigned int n = 0;
    std::vector<unsigned char> buffer;
    {
        size_t size = random_size(0, MAX_BYTES);
        buffer.resize(size);
        RAND_bytes(buffer.data(), static_cast<int>(size));

        verifier::get()->trace("a", buffer);

        n = random_size(0, MAX_BYTES * BITS_PER_BYTE / 3);
        verifier::get()->trace("shifts", n);
    }

    // openssl
    {
        BIGNUM *r = BN_new();
        BN_bin2bn(buffer.data(), static_cast<int>(buffer.size()), r);
        BN_rshift(r, r, n);

        verifier::get()->trace("* r = a >> n", r);
        verifier::get()->erase("r = a >> n");
        ASSERT_TRUE(verifier::get()->probe("r = a >> n", r));

        BN_free(r);
    }

    // this implementation
    {
        mpi_t *t = NULL;
        mpi_from_octets(&t, buffer.data(), buffer.size());

        size_t sz = mpi_bits(t) > n ? (mpi_bits(t) - n) : 0;
        mpi_t *r = mpi_create(sz);

        mpi_rshift(r, t, n);
        verifier::get()->trace("  r = a >> n", r);
        EXPECT_TRUE(verifier::get()->probe("r = a >> n", r));

        mpi_destory(t);
        mpi_destory(r);
    }
}

// right-shift-and-assign
TEST(MPI, RightShiftAssign)
{
    unsigned int n = 0;
    std::vector<unsigned char> buffer;
    {
        size_t size = random_size(0, MAX_BYTES);
        buffer.resize(size);
        RAND_bytes(buffer.data(), static_cast<int>(size));

        verifier::get()->trace("a", buffer);

        n = random_size(0, MAX_BYTES * BITS_PER_BYTE / 3);
        verifier::get()->trace("shifts", n);
    }

    // openssl
    {
        BIGNUM *r = BN_new();
        BN_bin2bn(buffer.data(), static_cast<int>(buffer.size()), r);

        BN_rshift(r, r, n);
        verifier::get()->trace("* r >>= n", r);
        verifier::get()->erase("r >>= n");
        ASSERT_TRUE(verifier::get()->probe("r >>= n", r));

        BN_free(r);
    }

    // this implementation
    {
        mpi_t *r = NULL;
        mpi_from_octets(&r, buffer.data(), buffer.size());

        mpi_rshift(r, r, n);
        verifier::get()->trace("  r >>= n", r);
        EXPECT_TRUE(verifier::get()->probe("r >>= n", r));

        mpi_destory(r);
    }
}

#if 0 // FIXME
TEST(MPI, Exponentiation)
{
    mpn_limb_t e;
    std::vector<unsigned char> gbuffer;
    {
        size_t size;
        RAND_bytes((unsigned char *)&size, sizeof(size)), size %= (MAX_BYTES / 4 + 1);
        gbuffer.resize(size);
        RAND_bytes(gbuffer.data(), static_cast<int>(size));

        verifier::get()->trace("a", gbuffer);

        RAND_bytes((unsigned char *)&e, sizeof(e)), e %= 64;
        verifier::get()->trace("e", e);
    }

    // openssl
    {
        BIGNUM *r = BN_new(), *a = BN_new(), *p = BN_new();
        BN_bin2bn(gbuffer.data(), static_cast<int>(gbuffer.size()), a);
        BN_set_word(p, e);

        BN_CTX *ctx = BN_CTX_new();
        BN_exp(r, a, p, ctx);
        BN_CTX_free(ctx);

        verifier::get()->trace("* r", r);
        verifier::get()->erase("r = a ^ b");
        ASSERT_TRUE(verifier::get()->probe("r = a ^ b", r));

        BN_free(r);
    }

    {
        mpi_t *g = NULL;
        mpi_from_octets(&g, gbuffer.data(), gbuffer.size());

        size_t rbits = (mpi_bits(g) + 1) << ((MPN_LIMB_BITS - mpn_limb_nlz_consttime(e)) % MPN_LIMB_BITS);
        mpi_t *r = mpi_create(rbits);

        mpi_exp_limb(r, g, e);
        verifier::get()->trace("  r", r);
        EXPECT_TRUE(verifier::get()->probe("r = a ^ b", r));

        mpi_destory(g);
        mpi_destory(r);
    }
}
#endif

// gcd: greatest common divisor algorithms
TEST(MPI, BinaryGCD)
{
    mpn_limb_t x, y;
    {
        RAND_bytes((unsigned char *)&x, sizeof(x));
        RAND_bytes((unsigned char *)&y, sizeof(y));

        verifier::get()->trace("x", x);
        verifier::get()->trace("y", y);
    }

    // binary gcd algorithm: gcd(x, y)
    {
        mpn_limb_t r = mpn_limb_gcd(x, y);
        verifier::get()->trace("gcd(x, y)", r);

        EXPECT_TRUE(true);
    }

    // TODO: binary extended gcd algorithm
}

TEST(MPI, GCD)
{
    std::vector<unsigned char> abuffer, bbuffer;
    {
        size_t size;
        {
            size = random_size(0, MAX_BYTES);
            abuffer.resize(size);
            RAND_bytes(abuffer.data(), static_cast<int>(size));

            verifier::get()->trace("a", abuffer);
        }

        {
            size = random_size(0, MAX_BYTES);
            bbuffer.resize(size);
            RAND_bytes(bbuffer.data(), static_cast<int>(size));

            verifier::get()->trace("b", bbuffer);
        }
    }

    // openssl
    {
        BIGNUM *r = BN_new(), *a = BN_new(), *b = BN_new();
        BN_bin2bn(abuffer.data(), static_cast<int>(abuffer.size()), a);
        BN_bin2bn(bbuffer.data(), static_cast<int>(bbuffer.size()), b);

        BN_CTX *ctx = BN_CTX_new();
        BN_gcd(r, a, b, ctx);
        BN_CTX_free(ctx);
        verifier::get()->trace("* r = gcd(a, b)", r);
        verifier::get()->erase("r = gcd(a, b)");
        ASSERT_TRUE(verifier::get()->probe("r = gcd(a, b)", r));

        BN_free(a);
        BN_free(b);
        BN_free(r);
    }

    // this implementation
    {
        mpi_t *a = mpi_create(abuffer.size() * BITS_PER_BYTE);
        mpi_t *b = mpi_create(bbuffer.size() * BITS_PER_BYTE);
        mpi_from_octets(&a, abuffer.data(), abuffer.size());
        mpi_from_octets(&b, bbuffer.data(), bbuffer.size());

        mpi_t *r = mpi_create((MAX_BYTES + 1) * BITS_PER_BYTE);

        // Daniel J. Bernsteion and Bo-Yin Yang's constant-time gcd algorithm
        {
            mpi_gcd_consttime(r, a, b, NULL);
            verifier::get()->trace("  r = gcd(a, b)", r);
            EXPECT_TRUE(verifier::get()->probe("r = gcd(a, b)", r));
        }

        // Lehmer's gcd algorithm
        if (0) { // FIXME
            mpi_gcd(r, a, b, NULL);
            verifier::get()->trace("  r = gcd(a, b)", r);
            EXPECT_TRUE(verifier::get()->probe("r = gcd(a, b)", r));
        }

        mpi_destory(a);
        mpi_destory(b);
        mpi_destory(r);
    }
}

TEST(MPI, Montgomery)
{
    std::vector<unsigned char> nbuffer, gbuffer, ebuffer;
    {
        size_t size;
        {
            size = random_size();
            nbuffer.resize(size);
            RAND_bytes(nbuffer.data(), static_cast<int>(size));
            if (size > 0) {
                nbuffer[size - 1] |= 0x1; // odd
            }

            verifier::get()->trace("n", nbuffer);
        }

        {
            size = random_size(1, nbuffer.size() + 1);
            gbuffer.resize(size);
            RAND_bytes(gbuffer.data(), static_cast<int>(size));

            verifier::get()->trace("g", gbuffer);
        }

        {
            size = random_size(0, nbuffer.size() + 1);
            ebuffer.resize(size);
            RAND_bytes(ebuffer.data(), static_cast<int>(size));

            verifier::get()->trace("e", ebuffer);
        }
    }

    // openssl
    {
        BIGNUM *n = BN_new(), *g = BN_new(), *e = BN_new();
        {
            BN_bin2bn(nbuffer.data(), static_cast<int>(nbuffer.size()), n);
            BN_bin2bn(gbuffer.data(), static_cast<int>(gbuffer.size()), g);
            BN_bin2bn(ebuffer.data(), static_cast<int>(ebuffer.size()), e);
        }
        BN_CTX *ctx = BN_CTX_new();

        BN_MONT_CTX *mont = BN_MONT_CTX_new();
        ASSERT_TRUE(mont != NULL);
        BN_MONT_CTX_set(mont, n, ctx);

        BIGNUM *r = BN_new();
        BN_mod_exp_mont(r, g, e, n, ctx, mont);

        verifier::get()->trace("* r = g ^ e mod n", r);
        verifier::get()->erase("r = g ^ e mod n");
        ASSERT_TRUE(verifier::get()->probe("r = g ^ e mod n", r));

        BN_CTX_free(ctx);
        BN_MONT_CTX_free(mont);
        BN_free(n);
        BN_free(g);
        BN_free(e);
        BN_free(r);
    }

    // this implementation
    {
        mpi_t *n = mpi_create(nbuffer.size() * BITS_PER_BYTE);
        mpi_t *g = mpi_create(gbuffer.size() * BITS_PER_BYTE);
        mpi_t *e = mpi_create(ebuffer.size() * BITS_PER_BYTE);
        mpi_from_octets(&n, nbuffer.data(), nbuffer.size());
        mpi_from_octets(&g, gbuffer.data(), gbuffer.size());
        mpi_from_octets(&e, ebuffer.data(), ebuffer.size());

        mpn_montgomery_t *mont = mpn_montgomery_create(mpi_bits(n), 6 * n->size);
        ASSERT_TRUE(mont != NULL);
        mpi_montgomery_set_modulus(mont, n);

        mpi_t *r = mpi_create(mpi_bits(n));
        ASSERT_TRUE(r != NULL);

        {
            mpi_montgomery_exp(r, g, e, mont);

            verifier::get()->trace("  r = g ^ e mod n", r);
            EXPECT_TRUE(verifier::get()->probe("r = g ^ e mod n", r));
        }

        {
            mpi_montgomery_exp_consttime(r, g, e, mont);

            verifier::get()->trace("  r = g ^ e mod n", r);
            EXPECT_TRUE(verifier::get()->probe("r = g ^ e mod n", r));
        }

        mpn_montgomery_destory(mont);
        mpi_destory(n);
        mpi_destory(g);
        mpi_destory(e);
        mpi_destory(r);
    }
}

#if 0 // FIXME
TEST(MPI, GeneratePrime)
{
    unsigned int bits;
    std::vector<unsigned char> prime;
    {
        bits = random_size(MIN_BITS, MAX_BITS);
        prime.resize(MPN_BITS_TO_BYTES(bits));
        verifier::get()->trace("bits", bits);
    }

    // generate a prime
    {
        auto rand_bytes = [](void *state, unsigned char *buffer, unsigned int size) -> int {
            (void)state;
            RAND_bytes(buffer, static_cast<int>(size));
            return 0;
        };
        mpi_t *r = mpi_create(bits);
        mpi_generate_prime(r, bits, 0, NULL, NULL, rand_bytes, NULL);
        EXPECT_EQ(1, mpi_is_prime(r, 0, 0, NULL, rand_bytes, NULL));
        verifier::get()->trace("prime", r);
        EXPECT_EQ(0, mpi_to_octets(r, prime.data(), prime.size(), NULL));
        mpi_destory(r);
    }

    // verify using openssl
    {
        BIGNUM *r = BN_bin2bn(prime.data(), static_cast<int>(prime.size()), NULL);
        EXPECT_EQ(1, BN_is_prime_fasttest_ex(r, BN_prime_checks, NULL, 0, NULL));
        BN_free(r);
    }
}
#endif

TEST(MPI, VerifyPrime)
{
    unsigned int bits;
    std::vector<unsigned char> prime;
    {
        bits = random_size(MIN_BITS, MAX_BITS);
        prime.resize(MPN_BITS_TO_BYTES(bits));
        verifier::get()->trace("bits", bits);
    }

    // generate prime using openssl
    {
        BIGNUM *r = BN_new();
        BN_generate_prime_ex(r, bits, 0, NULL, NULL, NULL);
        verifier::get()->trace("prime", r);
        BN_bn2bin(r, prime.data());
        BN_free(r);
    }

    // verify using this implementation
    {
        mpi_t *r = NULL;
        mpi_from_octets(&r, prime.data(), prime.size());
        EXPECT_EQ(1, mpi_is_prime(
                         r, 0, 0, NULL,
                         [](void *state, unsigned char *buffer, unsigned int size) -> int {
                             (void)state;
                             RAND_bytes(buffer, static_cast<int>(size));
                             return 0;
                         },
                         NULL));
        mpi_destory(r);
    }
}

// multiple-precision modular arithmetic
//   classical modular multiplication
//   montgomery reduction
//   barrett reduction
//   reduction methods for moduli of special form
#include <mpi/mpi-rsa.h>
#include <openssl/rsa.h>

TEST(RSA, cipher)
{
    unsigned int bits = 2048;
    std::vector<unsigned char> E{0x01, 0x00, 0x01}, N, D, P, Q, DP, DQ, QINV;

    RSA *osslkey = RSA_new();
    rsa_key_t *mpikey = NULL;

    enum {
        GENERATE_BY_MPI,
        GENERATE_BY_OPENSSL,
    } generate_by = GENERATE_BY_OPENSSL;

    // generate rsa keypair
    if (generate_by == GENERATE_BY_OPENSSL) {
        BIGNUM *e = BN_bin2bn(E.data(), static_cast<int>(E.size()), NULL);
        RSA_generate_multi_prime_key(osslkey, bits, 2, e, NULL);
        BN_free(e);

        // export N
        N.resize(static_cast<std::size_t>(BN_num_bytes(RSA_get0_n(osslkey))));
        BN_bn2bin(RSA_get0_n(osslkey), N.data());

        // export D
        D.resize(static_cast<std::size_t>(BN_num_bytes(RSA_get0_d(osslkey))));
        BN_bn2bin(RSA_get0_d(osslkey), D.data());

        // export P
        P.resize(static_cast<std::size_t>(BN_num_bytes(RSA_get0_p(osslkey))));
        BN_bn2bin(RSA_get0_p(osslkey), P.data());

        // export Q
        Q.resize(static_cast<std::size_t>(BN_num_bytes(RSA_get0_q(osslkey))));
        BN_bn2bin(RSA_get0_q(osslkey), Q.data());

        // export DP
        DP.resize(static_cast<std::size_t>(BN_num_bytes(RSA_get0_dmp1(osslkey))));
        BN_bn2bin(RSA_get0_dmp1(osslkey), DP.data());

        // export DQ
        DQ.resize(static_cast<std::size_t>(BN_num_bytes(RSA_get0_dmq1(osslkey))));
        BN_bn2bin(RSA_get0_dmq1(osslkey), DQ.data());

        // export QINV
        QINV.resize(static_cast<std::size_t>(BN_num_bytes(RSA_get0_iqmp(osslkey))));
        BN_bn2bin(RSA_get0_iqmp(osslkey), QINV.data());

        // import to mpikey
        {
            mpi_t *pubexp = mpi_create(E.size() * BITS_PER_BYTE);
            ASSERT_EQ(0, mpi_from_octets(&pubexp, E.data(), E.size()));

            mpikey = rsa_new(mpi_bits(pubexp), bits, 2);
            {
                mpi_t *_n = NULL, *_e = NULL, *_d = NULL, *_dp = NULL, *_dq = NULL, *_qinv = NULL;
                mpi_from_octets(&_n, N.data(), N.size());
                mpi_from_octets(&_e, E.data(), E.size());
                mpi_from_octets(&_d, D.data(), D.size());
                mpi_from_octets(&_dp, DP.data(), DP.size());
                mpi_from_octets(&_dq, DQ.data(), DQ.size());
                mpi_from_octets(&_qinv, QINV.data(), QINV.size());

                ASSERT_EQ(0, rsa_import(mpikey, _n, _e, _d, _dp, _dq, _qinv));
                mpi_destory(_n);
                mpi_destory(_e);
                mpi_destory(_d);
                mpi_destory(_dp);
                mpi_destory(_dq);
                mpi_destory(_qinv);
            }

            mpi_destory(pubexp);
        }
    } else if (generate_by == GENERATE_BY_MPI) {
        mpi_t *pubexp = mpi_create(E.size() * BITS_PER_BYTE);
        ASSERT_EQ(0, mpi_from_octets(&pubexp, E.data(), E.size()));

        mpikey = rsa_generate_key(
            pubexp, bits, 2,
            [](void *state, unsigned char *buffer, unsigned int size) -> int {
                (void)state;
                RAND_bytes(buffer, (int)size);
                return 0;
            },
            NULL);
        mpi_destory(pubexp);
        if (mpikey != NULL) { std::cout << "pbits: " << mpikey->pbits << ", qbits: " << mpikey->qbits << std::endl; }

        // export N
        N.resize(MPN_BITS_TO_BYTES(mpikey->nbits));
        mpn_to_octets(N.data(), N.size(), mpikey->montN->modulus, MPN_BITS_TO_LIMBS(mpikey->nbits));

        // export D
        D.resize(MPN_BITS_TO_BYTES(mpikey->dbits));
        mpn_to_octets(D.data(), D.size(), mpikey->d, MPN_BITS_TO_LIMBS(mpikey->dbits));

        // export P
        P.resize(MPN_BITS_TO_BYTES(mpikey->pbits));
        mpn_to_octets(P.data(), P.size(), mpikey->montP->modulus, MPN_BITS_TO_LIMBS(mpikey->pbits));

        // export Q
        Q.resize(MPN_BITS_TO_BYTES(mpikey->qbits));
        mpn_to_octets(Q.data(), Q.size(), mpikey->montQ->modulus, MPN_BITS_TO_LIMBS(mpikey->qbits));

        // export DP
        DP.resize(MPN_BITS_TO_BYTES(mpikey->pbits));
        mpn_to_octets(DP.data(), DP.size(), mpikey->dp, MPN_BITS_TO_LIMBS(mpikey->pbits));

        // export DQ
        DQ.resize(MPN_BITS_TO_BYTES(mpikey->qbits));
        mpn_to_octets(DQ.data(), DQ.size(), mpikey->dq, MPN_BITS_TO_LIMBS(mpikey->qbits));

        // export QINV
        QINV.resize(MPN_BITS_TO_BYTES(mpikey->pbits));
        mpn_to_octets(QINV.data(), QINV.size(), mpikey->qinv, MPN_BITS_TO_LIMBS(mpikey->pbits));

        // import to osslkey
        {
            // TODO
        }
    } else {
        MPI_RAISE_EXCEPTION();
    }

    verifier::get()->trace("n", N);
    verifier::get()->trace("e", E);
    verifier::get()->trace("d", D);
    verifier::get()->trace("p", P);
    verifier::get()->trace("q", Q);
    verifier::get()->trace("dp", DP);
    verifier::get()->trace("dq", DQ);
    verifier::get()->trace("qinv", QINV);

    // keypair is ready, we can do the computations now
    std::vector<unsigned char> INPUT(MPN_BITS_TO_BYTES(bits)), OUTPUT(MPN_BITS_TO_BYTES(bits));

    // generate input
    {
        RAND_bytes(INPUT.data(), static_cast<int>(INPUT.size()));
        verifier::get()->trace("input", INPUT);
    }

    // openssl
    {
        // pub cipher
        RSA_public_encrypt(static_cast<int>(INPUT.size()), INPUT.data(), OUTPUT.data(), osslkey, RSA_NO_PADDING);
        verifier::get()->erase("pub cipher");
        ASSERT_TRUE(verifier::get()->probe("pub cipher", OUTPUT));
        verifier::get()->trace("pub cipher", OUTPUT);

        // prv cipher
        RSA_private_decrypt(static_cast<int>(INPUT.size()), INPUT.data(), OUTPUT.data(), osslkey, RSA_NO_PADDING);
        verifier::get()->erase("prv cipher");
        ASSERT_TRUE(verifier::get()->probe("prv cipher", OUTPUT));
        verifier::get()->trace("prv cipher", OUTPUT);
    }

    // this implementation
    {
        mpi_t *r = mpi_create(bits), *x = mpi_create(bits);
        mpi_from_octets(&x, INPUT.data(), INPUT.size());

        rsa_pub_cipher(r, x, mpikey);
        verifier::get()->trace("pub cipher", r);
        EXPECT_TRUE(verifier::get()->probe("pub cipher", r));
        rsa_prv_cipher(r, x, mpikey);
        verifier::get()->trace("prv cipher", r);
        EXPECT_TRUE(verifier::get()->probe("prv cipher", r));

        // Garner's algorithm
        // CRT: chinese remainder theorem for integers
        rsa_prv_cipher_crt(r, x, mpikey);
        verifier::get()->trace("prv cipher(crt)", r);
        EXPECT_TRUE(verifier::get()->probe("prv cipher", r));

        mpi_destory(r);
        mpi_destory(x);
    }

    rsa_free(mpikey);
    RSA_free(osslkey);
}

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>

int main(int argc, char **argv)
{
    verifier::get()->set_verbose(true);

    CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
