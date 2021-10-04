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
#include <mpi/mpi-binary.h>
#include <mpi/mpi-montgomery.h>

#include "benchmark.hpp"
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <unistd.h>

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wconversion"
#elif defined(__clang__)
#pragma clang diagnostic ignored "-Wconversion"
#endif

static std::string random_hex_string(size_t len)
{
    std::string s;
    s.reserve(len);
    srand((unsigned)time(NULL) * getpid());
    static const char alphanum[] = "0123456789ABCDEFabcdef";
    for (size_t i = 0; i < len; ++i) { s += alphanum[rand() % (sizeof(alphanum) - 1)]; }

    return s;
}

static size_t random_size(unsigned int min = 0, unsigned int max = 8192)
{
    size_t size;
    RAND_bytes((unsigned char *)&size, sizeof(size));
    return size % (max - min) + min;
}

int main(void)
{
    // create mpi from hex string
    // and convert mpi to hex string
    {
        size_t size = 1024;
        std::string val = random_hex_string(size);

        // openssl
        {
            BIGNUM *r = BN_new(); // BN_new to make sure no "Segmentation fault" error when generated string
                                  // is empty

            {
                Benchmark bench("from-string(ossl)", 0);
                if (bench.run([&]() -> bool {
                        BN_hex2bn(&r, val.c_str());
                        return true;
                    })) {
                    bench.print();
                }
            }

            {
                Benchmark bench("to-string(ossl)", 0);
                if (bench.run([&]() -> bool {
                        free(BN_bn2hex(r));
                        return true;
                    })) {
                    bench.print();
                }
            }

            BN_free(r);
        }

        // this implementation
        {
            mpi_t *r = NULL;

            {
                Benchmark bench("from-string(mpi)", 0);
                if (bench.run([&]() -> bool {
                        return mpi_from_string(&r, val.c_str()) == 0;
                    })) {
                    bench.print();
                }
            }

            {
                Benchmark bench("to-string(mpi)", 0);
                if (bench.run([&]() -> bool {
                        free(mpi_to_string(r));
                        return true;
                    })) {
                    bench.print();
                }
            }

            mpi_destory(r);
        }
    }

    // create mpi from octets string
    // and convert mpi to octets string
    {
        std::vector<unsigned char> buffer;
        {
            size_t size = 256;
            buffer.resize(size);
            RAND_bytes(buffer.data(), static_cast<int>(size));
        }

        // openssl
        {
            BIGNUM *r = BN_new();

            {
                Benchmark bench("from-octets(ossl)", 0);
                if (bench.run([&]() -> bool {
                        BN_bin2bn(buffer.data(), static_cast<int>(buffer.size()), r);
                        return true;
                    })) {
                    bench.print();
                }
            }

            {
                Benchmark bench("to-octets(mpi)", 0);
                if (bench.run([&]() -> bool {
                        BN_bn2bin(r, buffer.data());
                        return true;
                    })) {
                    bench.print();
                }
            }

            BN_free(r);
        }

        // this implementation
        {
            mpi_t *r = NULL;

            {
                Benchmark bench("from-octets(mpi)", 0);
                if (bench.run([&]() -> bool {
                        return mpi_from_octets(&r, buffer.data(), buffer.size()) == 0;
                    })) {
                    bench.print();
                }
            }

            {
                Benchmark bench("to-octets(mpi)", 0);
                if (bench.run([&]() -> bool {
                        mpi_to_octets(r, buffer.data(), buffer.size(), NULL);
                        return true;
                    })) {
                    bench.print();
                }
            }

            mpi_destory(r);
        }
    }

    // addition, addition-and-assign
    {
        std::vector<unsigned char> abuffer, bbuffer;
        {
            size_t size = 256;
            {
                abuffer.resize(size);
                RAND_bytes(abuffer.data(), static_cast<int>(size));
            }

            {
                bbuffer.resize(size);
                RAND_bytes(bbuffer.data(), static_cast<int>(size));
            }
        }

        // openssl
        {
            BIGNUM *r = BN_new(), *a = BN_new(), *b = BN_new();
            BN_bin2bn(abuffer.data(), static_cast<int>(abuffer.size()), a);
            BN_bin2bn(bbuffer.data(), static_cast<int>(bbuffer.size()), b);

            Benchmark bench("add(ossl)", 0);
            if (bench.run([&]() -> bool {
                    BN_add(r, a, b);
                    return true;
                })) {
                bench.print();
            }

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

            Benchmark bench("add(mpi)", 0);
            if (bench.run([&]() -> bool {
                    return mpi_add(r, a, b) == 0;
                })) {
                bench.print();
            }

            mpi_destory(a);
            mpi_destory(b);
            mpi_destory(r);
        }
    }

    {
        std::vector<unsigned char> rbuffer, bbuffer;
        {
            size_t size = 256;
            {
                rbuffer.resize(size);
                RAND_bytes(rbuffer.data(), static_cast<int>(size));
            }

            {
                bbuffer.resize(size);
                RAND_bytes(bbuffer.data(), static_cast<int>(size));
            }
        }

        // openssl
        {
            BIGNUM *r = BN_new(), *b = BN_new();
            BN_bin2bn(rbuffer.data(), static_cast<int>(rbuffer.size()), r);
            BN_bin2bn(bbuffer.data(), static_cast<int>(bbuffer.size()), b);

            Benchmark bench("add-assign(ossl)", 0);
            if (bench.run([&]() -> bool {
                    BN_add(r, r, b);
                    return true;
                })) {
                bench.print();
            }

            BN_free(b);
            BN_free(r);
        }

        // this implementation
        {
            /* @test: create first to avoid test failure if octest buffer is empty */
            mpi_t *r = mpi_create(std::max(rbuffer.size(), bbuffer.size()) * BITS_PER_BYTE + 1);
            mpi_t *b = mpi_create(bbuffer.size() * BITS_PER_BYTE);
            mpi_from_octets(&r, rbuffer.data(), rbuffer.size());
            mpi_from_octets(&b, bbuffer.data(), bbuffer.size());

            size_t sz = (mpi_bits(r) >= mpi_bits(b) ? mpi_bits(r) : mpi_bits(b)) + 1;
            r = mpi_expand(r, sz);

            Benchmark bench("add-assign(mpi)", 0);
            if (bench.run([&]() -> bool {
                    return mpi_add(r, r, b) == 0;
                })) {
                bench.print();
            }

            mpi_destory(b);
            mpi_destory(r);
        }
    }

    // subtraction, subtraction-and-assign
    {
        std::vector<unsigned char> abuffer, bbuffer;
        {
            size_t size = 256;
            {
                abuffer.resize(size);
                RAND_bytes(abuffer.data(), static_cast<int>(size));
            }

            {
                bbuffer.resize(size);
                RAND_bytes(bbuffer.data(), static_cast<int>(size));
            }
        }

        // openssl
        {
            BIGNUM *r = BN_new(), *a = BN_new(), *b = BN_new();
            BN_bin2bn(abuffer.data(), static_cast<int>(abuffer.size()), a);
            BN_bin2bn(bbuffer.data(), static_cast<int>(bbuffer.size()), b);

            Benchmark bench("sub(ossl)", 0);
            if (bench.run([&]() -> bool {
                    BN_sub(r, a, b);
                    return true;
                })) {
                bench.print();
            }

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

            size_t sz = (mpi_bits(a) >= mpi_bits(b) ? mpi_bits(a) : mpi_bits(b));
            mpi_t *r = mpi_create(sz);

            Benchmark bench("sub(mpi)", 0);
            if (bench.run([&]() -> bool {
                    return mpi_sub(r, a, b) == 0;
                })) {
                bench.print();
            }

            mpi_destory(a);
            mpi_destory(b);
            mpi_destory(r);
        }
    }

    {
        std::vector<unsigned char> rbuffer, bbuffer;
        {
            size_t size = 256;
            {
                rbuffer.resize(size);
                RAND_bytes(rbuffer.data(), static_cast<int>(size));
            }

            {
                bbuffer.resize(size);
                RAND_bytes(bbuffer.data(), static_cast<int>(size));
            }
        }

        // openssl
        {
            BIGNUM *r = BN_new(), *b = BN_new();
            BN_bin2bn(rbuffer.data(), static_cast<int>(rbuffer.size()), r);
            BN_bin2bn(bbuffer.data(), static_cast<int>(bbuffer.size()), b);

            Benchmark bench("sub-assign(ossl)", 0);
            if (bench.run([&]() -> bool {
                    BN_sub(r, r, b);
                    return true;
                })) {
                bench.print();
            }

            BN_free(b);
            BN_free(r);
        }

        // this implementation
        {
            /* @test: create first to avoid test failure if octest buffer is empty */
            mpi_t *r = mpi_create(std::max(rbuffer.size(), bbuffer.size()) * BITS_PER_BYTE + 1);
            mpi_t *b = mpi_create(bbuffer.size() * BITS_PER_BYTE);
            mpi_from_octets(&r, rbuffer.data(), rbuffer.size());
            mpi_from_octets(&b, bbuffer.data(), bbuffer.size());

            size_t sz = (mpi_bits(r) >= mpi_bits(b) ? mpi_bits(r) : mpi_bits(b));
            r = mpi_expand(r, sz);

            Benchmark bench("sub-assign(mpi)", 0);
            if (bench.run([&]() -> bool {
                    return mpi_sub(r, r, b) == 0;
                })) {
                bench.print();
            }

            mpi_destory(b);
            mpi_destory(r);
        }
    }

    // multiplication, square
    {
        std::vector<unsigned char> abuffer, bbuffer;
        {
            size_t size = 256;
            {
                abuffer.resize(size);
                RAND_bytes(abuffer.data(), static_cast<int>(size));
            }

            {
                bbuffer.resize(size);
                RAND_bytes(bbuffer.data(), static_cast<int>(size));
            }
        }

        // openssl
        {
            BIGNUM *r = BN_new(), *a = BN_new(), *b = BN_new();
            BN_bin2bn(abuffer.data(), static_cast<int>(abuffer.size()), a);
            BN_bin2bn(bbuffer.data(), static_cast<int>(bbuffer.size()), b);

            BN_CTX *ctx = BN_CTX_new();

            Benchmark bench("mul(ossl)", 0);
            if (bench.run([&]() -> bool {
                    BN_mul(r, a, b, ctx);
                    return true;
                })) {
                bench.print();
            }

            BN_CTX_free(ctx);

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

            Benchmark bench("mul(mpi)", 0);
            if (bench.run([&]() -> bool {
                    return mpi_mul(r, a, b) == 0;
                })) {
                bench.print();
            }

            mpi_destory(a);
            mpi_destory(b);
            mpi_destory(r);
        }
    }

    {
        std::vector<unsigned char> buffer;
        {
            size_t size = 256;
            buffer.resize(size);
            RAND_bytes(buffer.data(), static_cast<int>(size));
        }

        // openssl
        {
            BIGNUM *r = BN_new(), *a = BN_new();
            BN_bin2bn(buffer.data(), static_cast<int>(buffer.size()), a);

            BN_CTX *ctx = BN_CTX_new();

            Benchmark bench("sqr(ossl)", 0);
            if (bench.run([&]() -> bool {
                    BN_sqr(r, a, ctx);
                    return true;
                })) {
                bench.print();
            }

            BN_CTX_free(ctx);

            BN_free(a);
            BN_free(r);
        }

        // this implementation
        {
            /* @test: create first to avoid test failure if octest buffer is empty */
            mpi_t *a = mpi_create(buffer.size() * BITS_PER_BYTE);
            mpi_from_octets(&a, buffer.data(), buffer.size());

            mpi_t *r = mpi_create((a != NULL ? a->size : 0) * 2 * MPN_LIMB_BITS);

            Benchmark bench("sqr(mpi)", 0);
            if (bench.run([&]() -> bool {
                    return mpi_sqr(r, a) == 0;
                })) {
                bench.print();
            }

            mpi_destory(a);
            mpi_destory(r);
        }
    }

    // division, modular
    {
        std::vector<unsigned char> abuffer, bbuffer;
        {
            size_t size;
            {
                size = 512;
                abuffer.resize(size);
                RAND_bytes(abuffer.data(), static_cast<int>(size));
            }

            {
                size = 256;
                bbuffer.resize(size);
                RAND_bytes(bbuffer.data(), static_cast<int>(size));
            }

            // it's too simple for the division algorithm if abufflen < bbufflen, so swap them to make sure
            // that abufflen >= bbufflen
            if (abuffer.size() < bbuffer.size()) { abuffer.swap(bbuffer); }
        }

        // openssl
        {
            BIGNUM *q = BN_new(), *r = BN_new(), *a = BN_new(), *b = BN_new();
            BN_bin2bn(abuffer.data(), static_cast<int>(abuffer.size()), a);
            BN_bin2bn(bbuffer.data(), static_cast<int>(bbuffer.size()), b);

            BN_CTX *ctx = BN_CTX_new();

            Benchmark bench("div(ossl)", 0);
            if (bench.run([&]() -> bool {
                    BN_div(q, r, a, b, ctx);
                    return true;
                })) {
                bench.print();
            }

            BN_CTX_free(ctx);

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
            a = mpi_expand(a, mpi_bits(a) + MPN_LIMB_BITS); // @IMPORTANT: one additional word required

            size_t qsize = 0, rsize = mpi_bits(b);
            if (mpi_bits(a) >= mpi_bits(b)) {
                rsize = mpi_bits(a);
                qsize = mpi_bits(a) - mpi_bits(b) + MPN_LIMB_BITS;
            }

            mpi_t *q = mpi_create(qsize);
            mpi_t *r = mpi_create(rsize);

            Benchmark bench("div(mpi)", 0);
            if (bench.run([&]() -> bool {
                    return mpi_div(q, r, a, b) == 0;
                })) {
                bench.print();
            }

            mpi_destory(a);
            mpi_destory(b);
            mpi_destory(q);
            mpi_destory(r);
        }
    }

    // exponentiation

    // left-shift, left-shift-and-assign

    // right-shift, right-shift-and-assign

    // greatest common divisor
    // conclusion:
    //   1. perf(mpi_gcd(r, a, b, NULL)) > perf(mpi_gcd_consttime(r, a, b, NULL)), about 30+(even 100+)
    //   times faster
    {
        std::vector<unsigned char> abuffer, bbuffer;
        {
            size_t size;
            {
                size = 256;
                abuffer.resize(size);
                RAND_bytes(abuffer.data(), static_cast<int>(size));
            }

            {
                size = 512;
                bbuffer.resize(size);
                RAND_bytes(bbuffer.data(), static_cast<int>(size));
            }
        }

        // openssl
        {
            BIGNUM *r = BN_new(), *a = BN_new(), *b = BN_new();
            BN_bin2bn(abuffer.data(), static_cast<int>(abuffer.size()), a);
            BN_bin2bn(bbuffer.data(), static_cast<int>(bbuffer.size()), b);

            BN_CTX *ctx = BN_CTX_new();

            {
                Benchmark bench("gcd_consttime(ossl)", 0);
                if (bench.run([&]() -> bool {
                        BN_gcd(r, a, b, ctx);
                        return true;
                    })) {
                    bench.print();
                }
            }


            BN_CTX_free(ctx);

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

            mpi_t *r = mpi_create(std::max(mpi_bits(a), mpi_bits(b)) + MPN_LIMB_BITS);

            // Daniel J. Bernsteion and Bo-Yin Yang's constant-time gcd algorithm
            {
                Benchmark bench("gcd_consttime(mpi)", 0);
                if (bench.run([&]() -> bool {
                        return mpi_gcd_consttime(r, a, b, NULL) == 0;
                    })) {
                    bench.print();
                }
            }

            // Lehmer's gcd algorithm
            if (0) { // FIXME: have bugs
                Benchmark bench("gcd(mpi)", 0);
                if (bench.run([&]() -> bool {
                        return mpi_gcd(r, a, b, NULL) == 0;
                    })) {
                    bench.print();
                }
            }

            mpi_destory(a);
            mpi_destory(b);
            mpi_destory(r);
        }
    }

    // montgomery modular operation
    // montgomery exponentiation
    {
        std::vector<unsigned char> nbuffer, gbuffer, ebuffer;
        {
            size_t size;
            {
                size = 256;
                nbuffer.resize(size);
                RAND_bytes(nbuffer.data(), static_cast<int>(size));
                if (size > 0) {
                    nbuffer[size - 1] |= 0x1; // odd
                }
            }

            {
                size = 256;
                gbuffer.resize(size);
                RAND_bytes(gbuffer.data(), static_cast<int>(size));
            }

            {
                size = 256;
                ebuffer.resize(size);
                RAND_bytes(ebuffer.data(), static_cast<int>(size));
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
            BN_MONT_CTX_set(mont, n, ctx);

            BIGNUM *r = BN_new();

            Benchmark bench("montgomery-exp(ossl)", 0);
            if (bench.run([&]() -> bool {
                    return BN_mod_exp_mont(r, g, e, n, ctx, mont) == 1;
                })) {
                bench.print();
            }

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

            mpi_montgomery_t *mont = mpi_montgomery_create(mpi_bits(n), 6 * n->size);
            mpi_montgomery_set_modulus(mont, n);

            mpi_t *r = mpi_create(mpi_bits(n));

            {
                Benchmark bench("montgomery-exp(mpi)", 0);
                if (bench.run([&]() -> bool {
                        return mpi_montgomery_exp(r, g, e, mont) == 0;
                    })) {
                    bench.print();
                }
            }

            {
                Benchmark bench("montgomery_exp_consttime(mpi)", 0);
                if (bench.run([&]() -> bool {
                        return mpi_montgomery_exp_consttime(r, g, e, mont) == 0;
                    })) {
                    bench.print();
                }
            }

            mpi_montgomery_destory(mont);
            mpi_destory(n);
            mpi_destory(g);
            mpi_destory(e);
            mpi_destory(r);
        }
    }

    // prime generation/verification
    // prime generation
    {
        unsigned int bits = 2048;

        // generate prime: openssl
        {
            BIGNUM *prime = BN_new();

            Benchmark bench("generate_prime(ossl)", 0);
            if (bench.run(
                    [&]() -> bool {
                        return BN_generate_prime_ex(prime, bits, 0, NULL, NULL, NULL) == 1;
                    },
                    6.0)) {
                bench.print();
            }

            BN_free(prime);
        }

        // generate prime: this implementation
        {
            mpi_t *prime = mpi_create(bits);

            Benchmark bench("generate_prime(mpi)", 0);
            if (bench.run(
                    [&]() -> bool {
                        return mpi_generate_prime(
                                   prime, bits, 0, NULL, NULL,
                                   [](void *state, unsigned char *buffer, unsigned int size) -> int {
                                       (void)state;
                                       RAND_bytes(buffer, (int)size);
                                       return 0;
                                   },
                                   NULL)
                               == 0;
                    },
                    6.0)) {
                bench.print();
            }

            mpi_destory(prime);
        }
    }

    // prime verification
    {
        unsigned int bits = 2048;
        std::vector<unsigned char> prime;
        {
            prime.resize(MPI_BITS_TO_BYTES(bits));
        }

        // generate prime using openssl
        {
            BIGNUM *r = BN_new();
            BN_generate_prime_ex(r, bits, 0, NULL, NULL, NULL);
            BN_bn2bin(r, prime.data());

            Benchmark bench("is_prime(ossl)", 0);
            if (bench.run([&]() -> bool {
                    return BN_is_prime_fasttest_ex(r, BN_prime_checks, NULL, 0, NULL) == 1;
                })) {
                bench.print();
            }

            BN_free(r);
        }

        {
            mpi_t *r = NULL;
            mpi_from_octets(&r, prime.data(), prime.size());

            Benchmark bench("is_prime(mpi)", 0);
            if (bench.run([&]() -> bool {
                    return mpi_is_prime(
                               r, 0, 0, NULL,
                               [](void *state, unsigned char *buffer, unsigned int size) -> int {
                                   (void)state;
                                   RAND_bytes(buffer, static_cast<int>(size));
                                   return 0;
                               },
                               NULL)
                           == 1;
                })) {
                bench.print();
            }
            mpi_destory(r);
        }
    }


    std::cout << std::endl << "Others" << std::endl;
    // MUL2: r = a * 2 = a + a = a << 1
    // conclusion:
    //   1. perf(mpi_add(r, a, a)) > perf(mpi_lshift(r, a, 1)), about 3 times faster
    //   2. a * 32 = a << 5, from this case, mpi_lshift is the better choise.
    {
        mpi_t *a = NULL;
        {
            unsigned char buff[256];
            size_t bufflen = random_size(0, sizeof(buff));
            RAND_bytes(buff, bufflen);

            mpi_from_octets(&a, buff, bufflen);
        }
        mpi_t *r = mpi_create(mpi_bits(a) + 1);

        {
            Benchmark bench("MUL2(a * 2 = a + a)", 0);
            if (bench.run([&]() -> bool {
                    return mpi_add(r, a, a) == 0;
                })) {
                bench.print();
            }
        }

        {
            Benchmark bench("MUL2(a * 2 = a << 1)", 0);
            if (bench.run([&]() -> bool {
                    return mpi_lshift(r, a, 1) == 0;
                })) {
                bench.print();
            }
        }

        mpi_destory(a);
        mpi_destory(r);
    }

    return 0;
}
