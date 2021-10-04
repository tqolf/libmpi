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
#include "mpn-binary.h"
#include "mpn-montgomery.h"

#if !(defined MPN_MONT_REDC_ASM)
/**
 * mpn montgomery: montgomery reduction
 *
 * @note:
 *   1. m0: low word of (1 / modulus) mod b
 *   2. r = T/R mod m
 */
void mpn_montgomery_reduce_bin(mpn_limb_t *r, mpn_limb_t *product, const mpn_limb_t *m, unsigned int msize,
                               mpn_limb_t m0)
{
    MPN_ASSERT(msize > 0);
    mpn_limb_t carry = 0, extension;

    for (unsigned int n = 0; n < (msize - 1); n++) {
        mpn_limb_t u = product[n] * m0;
        mpn_limb_t t = product[msize + n + 1] + carry;

        extension = mpn_mul_acc(product + n, m, msize, u);
        UADD_AB(carry, product[msize + n], product[msize + n], extension);
        t += carry;

        carry = t < product[msize + n + 1];
        product[msize + n + 1] = t;
    }

    m0 *= product[msize - 1];
    extension = mpn_mul_acc(product + msize - 1, m, msize, m0);
    UADD_AB(extension, product[2 * msize - 1], product[2 * msize - 1], extension);

    carry |= extension;
    carry -= mpn_sub_vectorized(r, product + msize, m, msize);
    /* condition copy: R = carry ? Product + mSize : R */
    mpn_masked_copy_consttime(r, product + msize, r, msize, carry);
}
#endif

/**
 * mpn montgomery: r[] = to_mont(a[])
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpn_montgomery_encode(mpn_limb_t *r, const mpn_limb_t *a, mpn_montgomery_t *mont)
{
#ifdef MPI_LOW_FOOTPRINT
    mpn_montgomery_mul(r, a, mont->montRR, mont);
#else
    MPN_ASSERT(r != NULL && a != NULL && mont != NULL);

    unsigned int msize = mont->modsize;
    mpn_limb_t *product = mpn_optimizer_get_limbs(mont->optimizer, 2 * msize);
    MPN_ASSERT(product != NULL);

    {
        /**
         * @performance:
         *   For __ARCH32E >= __ARCH32E_L9, ADX instruction would be the better choose
         */
        mpn_mul(product, a, msize, mont->montRR, msize);
        mpn_montgomery_reduce_bin(r, product, mont->modulus, msize, mont->k0);
    }

    mpn_optimizer_put_limbs(mont->optimizer, 2 * msize);
#endif
}

/**
 * mpn montgomery: r[] = from_mont(a)
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpn_montgomery_decode(mpn_limb_t *r, const mpn_limb_t *a, mpn_montgomery_t *mont)
{
    MPN_ASSERT(r != NULL && a != NULL && mont != NULL);

    unsigned int msize = mont->modsize;
    mpn_limb_t *product = mpn_optimizer_get_limbs(mont->optimizer, 2 * msize);
    MPN_ASSERT(NULL != product);

    {
        ZEXPAND(product, 2 * msize, a, msize);
        mpn_montgomery_reduce_bin(r, product, mont->modulus, msize, mont->k0);
    }

    mpn_optimizer_put_limbs(mont->optimizer, 2 * msize);
}

/**
 * mpn montgomery: r = (a + b) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. length of b: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpn_montgomery_add(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, mpn_montgomery_t *mont)
{
    MPN_ASSERT(r != NULL && a != NULL && b != NULL && mont != NULL);

    unsigned int msize = mont->modsize;
    mpn_limb_t *buffer = mpn_optimizer_get_limbs(mont->optimizer, msize);
    MPN_ASSERT(NULL != buffer);

    {
        mpn_limb_t extension = mpn_add_vectorized(r, a, b, msize);
        extension -= mpn_sub_vectorized(buffer, r, mont->modulus, msize);
        mpn_masked_move_consttime(r, buffer, msize, extension == 0);
    }

    mpn_optimizer_put_limbs(mont->optimizer, msize);
}

/**
 * mpn montgomery: r = (a - b) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. length of b: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpn_montgomery_sub(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, mpn_montgomery_t *mont)
{
    MPN_ASSERT(r != NULL && a != NULL && b != NULL && mont != NULL);

    unsigned int msize = mont->modsize;
    mpn_limb_t *buffer = mpn_optimizer_get_limbs(mont->optimizer, msize);
    MPN_ASSERT(NULL != buffer);

    {
        mpn_limb_t extension = mpn_sub_vectorized(r, a, b, msize);
        mpn_add_vectorized(buffer, r, mont->modulus, msize);
        mpn_masked_move_consttime(r, buffer, msize, extension != 0);
    }

    mpn_optimizer_put_limbs(mont->optimizer, msize);
}

/**
 * mpn montgomery: r = -b mod m = (m - b) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpn_montgomery_negative(mpn_limb_t *r, const mpn_limb_t *a, mpn_montgomery_t *mont)
{
    MPN_ASSERT(r != NULL && a != NULL && mont != NULL);

    unsigned int msize = mont->modsize;
    mpn_limb_t *buffer = mpn_optimizer_get_limbs(mont->optimizer, msize);
    MPN_ASSERT(NULL != buffer);

    {
        const mpn_limb_t *m = mont->modulus;
        mpn_limb_t extension = mpn_sub_vectorized(r, m, a, msize);
        extension -= mpn_sub_vectorized(buffer, r, m, msize);
        mpn_masked_move_consttime(r, buffer, msize, extension == 0);
    }

    mpn_optimizer_put_limbs(mont->optimizer, msize);
}

/**
 * mpn montgomery: r = (a / 2) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpn_montgomery_halve(mpn_limb_t *r, const mpn_limb_t *a, mpn_montgomery_t *mont)
{
    MPN_ASSERT(r != NULL && a != NULL && mont != NULL);

    unsigned int msize = mont->modsize;
    mpn_limb_t *buffer = mpn_optimizer_get_limbs(mont->optimizer, msize);
    MPN_ASSERT(NULL != buffer);

    {
        mpn_limb_t mask = 0 - (a[0] & 1);
        const mpn_limb_t *m = mont->modulus;
        for (unsigned int i = 0; i < msize; i++) { buffer[i] = m[i] & mask; }

        buffer[msize] = mpn_add_vectorized(buffer, buffer, a, msize);
        mpn_rshift(buffer, buffer, msize + 1, 1);

        COPY(r, buffer, msize);
    }

    mpn_optimizer_put_limbs(mont->optimizer, msize);
}

/**
 * mpn montgomery: r = (a * 2) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpn_montgomery_double(mpn_limb_t *r, const mpn_limb_t *a, mpn_montgomery_t *mont)
{
    mpn_montgomery_add(r, a, a, mont);
}

/**
 * mpn montgomery: r = (a * 3) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpn_montgomery_triple(mpn_limb_t *r, const mpn_limb_t *a, mpn_montgomery_t *mont)
{
    MPN_ASSERT(r != NULL && a != NULL && mont != NULL);

    unsigned int msize = mont->modsize;
    mpn_limb_t *buffer = mpn_optimizer_get_limbs(mont->optimizer, msize);
    MPN_ASSERT(NULL != buffer);

    {
        mpn_montgomery_add(buffer, a, a, mont);
        mpn_montgomery_add(r, a, buffer, mont);
    }

    mpn_optimizer_put_limbs(mont->optimizer, msize);
}

/**
 * mpn montgomery: r = prod mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of rod: modsize
 *   4. memory size from the pool: N/A
 */
void mpn_montgomery_reduce(mpn_limb_t *r, mpn_limb_t *prod, mpn_montgomery_t *mont)
{
    MPN_ASSERT(r != NULL && prod != NULL && mont != NULL);

    /**
     * @performance:
     *   for ARCH < ARCH_M7 or ARCH32E < ARCH32E_M7, this implementation would be the better choose
     */
#ifdef MPI_USE_C_MONTGOMERY_RED_BIN
    /* mont mul */
    mpn_limb_t carry = 0;
    unsigned int msize = mont->modsize;
    const mpn_limb_t *pm = mont->modulus, k0 = mont->k0;

    for (unsigned int i = 0; i < msize; i++, prod++) {
        mpn_limb_t rL, rH, extention, temp;

        /* u = prod[0]*k0 mod B */
        mpn_limb_t u = prod[0] * k0;

        /* (extention, temp) = prod[0] + m[0]*u (note temp ==0) */
        UMUL_AB(rH, rL, pm[0], u);
        UADD_AB(extention, temp, prod[0], rL);
        extention += rH;

        for (unsigned int j = 1; j < msize; j++) {
            mpn_limb_t c;
            UMUL_AB(rH, rL, pm[j], u);                    /* (H,L) = m[j]*u */
            UADD_AB(extention, temp, prod[j], extention); /* carry in extention,temp */
            UADD_AB(c, prod[j], temp, rL);                /* carry in c */
            extention += rH + c;                          /* accumulates both carrys above */
        }
        UADD_ABC(carry, prod[msize], prod[msize], extention, carry);
    }

    {
        carry -= mpn_sub_vectorized(r, prod, pm, msize);
        mpn_masked_move_consttime(r, prod, msize, carry != 0);
    }
#else
    mpn_montgomery_reduce_bin(r, prod, mont->modulus, mont->modsize, mont->k0);
#endif
}

/**
 * mpn montgomery: r = (a * b) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. length of b: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t) * 2
 */
void mpn_montgomery_mul(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, mpn_montgomery_t *mont)
{
    MPN_ASSERT(r != NULL && a != NULL && b != NULL && mont != NULL);

    /**
     * @performance:
     *   for ARCH < ARCH_M7 or ARCH32E < ARCH32E_M7, this implementation would be the better choose
     */
#ifdef MPI_USE_C_MONTGOMERY_MUL_BIN
    unsigned int msize = mont->modsize;

    mpn_limb_t *buffer = mpn_optimizer_get_limbs(mont->optimizer, msize);
    MPN_ASSERT(NULL != buffer);

    {
        mpn_limb_t carry = 0;
        const mpn_limb_t *pm = mont->modulus, m0 = mont->k0;

        /* clear buffer */
        ZEROIZE(buffer, 0, msize);

        /* mont mul */
        for (unsigned int i = 0; i < msize; i++) {
            mpn_limb_t bb = b[i];

            mpn_limb_t extAB = 0, extMU = 0;
            mpn_limb_t abL, abH, muL, muH, u;

            // (extAB, abL) = T = buffer[0] + a[0]*b
            UMUL_AB(abH, abL, a[0], bb);
            UADD_ABC(extAB, abL, buffer[0], abL, extAB);
            extAB += abH;

            // u = t * m0 mod B
            u = abL * m0;

            // (extMU, abL) = T + m[0]*u (note abL ==0)
            UMUL_AB(muH, muL, pm[0], u);
            UADD_ABC(extMU, abL, abL, muL, extMU);
            extMU += muH;

            for (unsigned int j = 1; j < msize; j++) {
                UMUL_AB(abH, abL, a[j], bb);
                UADD_ABC(extAB, abL, buffer[j], abL, extAB);
                extAB += abH;

                UMUL_AB(muH, muL, pm[j], u);
                UADD_ABC(extMU, abL, abL, muL, extMU);
                extMU += muH;

                buffer[j - 1] = abL;
            }
            UADD_ABC(carry, buffer[msize - 1], extAB, extMU, carry);
        }

        carry -= mpn_sub_vectorized(r, buffer, pm, msize);
        mpn_masked_move_consttime(r, buffer, msize, carry != 0);
    }

    mpn_optimizer_put_limbs(mont->optimizer, msize);
#else
    unsigned int msize = mont->modsize;
    mpn_limb_t *product = mpn_optimizer_get_limbs(mont->optimizer, 2 * msize);
    MPN_ASSERT(NULL != product);

    {
        /**
         * @performance:
         *   For __ARCH32E >= __ARCH32E_L9, ADX instruction would be the better choose
         */
        mpn_mul(product, a, msize, b, msize);
        mpn_montgomery_reduce_bin(r, product, mont->modulus, msize, mont->k0);
    }

    mpn_optimizer_put_limbs(mont->optimizer, 2 * msize);
#endif
}

/**
 * mpn montgomery: r = (a ^ 2) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t) * 2
 */
void mpn_montgomery_square(mpn_limb_t *r, const mpn_limb_t *a, mpn_montgomery_t *mont)
{
#ifdef MPI_LOW_FOOTPRINT
    return mpn_montgomery_mul(r, a, a, mont);
#else
    MPN_ASSERT(r != NULL && a != NULL && mont != NULL);

    unsigned int msize = mont->modsize;
    mpn_limb_t *product = mpn_optimizer_get_limbs(mont->optimizer, 2 * msize);
    MPN_ASSERT(NULL != product);

    {
        /**
         * @performance:
         *   For __ARCH32E >= __ARCH32E_L9, ADX instruction would be the better choose
         */
        mpn_sqr(product, a, msize);
        mpn_montgomery_reduce_bin(r, product, mont->modulus, msize, mont->k0);
    }

    mpn_optimizer_put_limbs(mont->optimizer, 2 * msize);
#endif
}

/**
 * montfomery factor k0 = -((modulus^-1 mod B) %B)
 */
mpn_limb_t mpn_montgomery_factor(mpn_limb_t m0)
{
    mpn_limb_t x = 2, y = 1;
    mpn_limb_t mask = 2 * x - 1;
    for (unsigned i = 2; i <= MPN_LIMB_BITS; i++, x <<= 1) {
        mpn_limb_t rH, rL;
        UMUL_AB(rH, rL, m0, y);
        if (x < (rL & mask)) { /* x < ((m0*y) mod (2*x)) */
            y += x;
        }
        mask += mask + 1;
        (void)rH;
    }
    return (mpn_limb_t)(0 - y);
}

#ifndef MPI_USE_SLIDING_WINDOW_EXP
/**
 * mpn montgomery: binary montgomery exponentiation
 *
 * @note:
 *   1. inputs and outputs are in montgomery domain
 *   2. possible inplace mode
 *   3. minimal size temporary memory chunk: modsize
 */
unsigned int mpn_montgomery_exp(mpn_limb_t *y, const mpn_limb_t *x, unsigned int xsize, const mpn_limb_t *e,
                                unsigned int ebits, mpn_montgomery_t *mont)
{
    unsigned int msize = mont->modsize;
    unsigned int esize = MPN_BITS_TO_LIMBS(ebits);

    if (mpn_is_zero_consttime(e, esize)) { // special case: x ^ 0 = 1
        COPY(y, mont->montR, msize);
    } else if (mpn_is_zero_consttime(x, xsize)) { // special case: 0 ^ e = 0
        ZEROIZE(y, 0, msize);
    } else { /* general case */
        /* allocate buffers */
        mpn_limb_t *dataT = mpn_optimizer_get_limbs(mont->optimizer, msize);
        MPN_ASSERT(dataT != NULL);

        /* copy and expand base to the modulus length */
        ZEXPAND(dataT, msize, x, xsize);
        /* copy */
        COPY(y, dataT, msize);

        esize = mpn_limbs(e, esize);

        /* execute most significant part pE */
        {
            mpn_limb_t ee = e[esize - 1];
            unsigned int n = mpn_limb_nlz_consttime(ee) + 1;

            ee <<= n;
            for (; n < MPN_LIMB_BITS; ee <<= 1, n++) {
                /* squaring R = R*R mod Modulus */
                mpn_montgomery_square(y, y, mont);
                /* and multiply R = R*X mod Modulus */
                if (ee & ((mpn_limb_t)1 << (MPN_LIMB_BITS - 1))) { mpn_montgomery_mul(y, y, dataT, mont); }
            }

            /* execute rest bits of E */
            for (--esize; esize > 0; esize--) {
                ee = e[esize - 1];

                for (n = 0; n < MPN_LIMB_BITS; ee <<= 1, n++) {
                    /* squaring: R = R*R mod Modulus */
                    mpn_montgomery_square(y, y, mont);

                    if (ee & ((mpn_limb_t)1 << (MPN_LIMB_BITS - 1))) { mpn_montgomery_mul(y, y, dataT, mont); }
                }
            }
        }

        mpn_optimizer_put_limbs(mont->optimizer, msize);
    }

    return msize;
}

/**
 * mpn montgomery: binary montgomery exponentiation(constant-time version)
 *
 * @note:
 *   1. inputs and outputs are in montgomery domain
 *   2. possible inplace mode
 *   3. minimal size temporary memory chunk: modsize * 2
 */
unsigned int mpn_montgomery_exp_consttime(mpn_limb_t *y, const mpn_limb_t *x, unsigned int xsize, const mpn_limb_t *e,
                                          unsigned int ebits, mpn_montgomery_t *mont)
{
    unsigned int msize = mont->modsize;
    unsigned int esize = MPN_BITS_TO_LIMBS(ebits);

    if (mpn_is_zero_consttime(e, esize)) { // special case: x ^ 0 = 1
        COPY(y, mont->montR, msize);
    } else if (mpn_is_zero_consttime(x, xsize)) { // special case: 0 ^ e = 0
        ZEROIZE(y, 0, msize);
    } else { /* general case */
        /* allocate buffers */
        mpn_limb_t *dataT = mpn_optimizer_get_limbs(mont->optimizer, msize);
        mpn_limb_t *sscmB = mpn_optimizer_get_limbs(mont->optimizer, msize);
        MPN_ASSERT(dataT != NULL && sscmB != NULL);

        /* mont(1) */
        mpn_limb_t *montR = mont->montR;

        /* copy and expand base to the modulus length */
        ZEXPAND(dataT, msize, x, xsize);
        /* init result */
        COPY(y, montR, msize);

        /* execute bits of E */
        for (; esize > 0; esize--) {
            mpn_limb_t ee = e[esize - 1];
            for (unsigned n = MPN_LIMB_BITS; n > 0; ee <<= 1, n--) {
                /* sscmB = msb(ee) ? X : mont(1) */
                mpn_limb_t mask = mpn_limb_test_msb_consttime(ee);
                mpn_masked_copy_consttime(sscmB, dataT, montR, msize, mask);

                /* squaring Y = Y^2 */
                mpn_montgomery_square(y, y, mont);
                /* and multiplication: Y = Y * sscmB */
                mpn_montgomery_mul(y, y, sscmB, mont);
            }
        }

        mpn_optimizer_put_limbs(mont->optimizer, msize);
        mpn_optimizer_put_limbs(mont->optimizer, msize);
    }

    return msize;
}

#else

#ifndef MPI_CACHE_LINE_BYTES
#define MPI_CACHE_LINE_BYTES    64 // in linux system, you can get via `getconf LEVEL1_DCACHE_LINESIZE`
#define MPI_LOG_CACHE_LINE_SIZE 6  // lb(MPI_CACHE_LINE_BYTES)
#endif

/** Fixed window exponentiation scramble/unscramble */
static unsigned int mpi_scramble_buffer_size(unsigned int msize, unsigned int winsize)
{
    /* size of resource to store 2 ^ winsize values of msize * sizeof(mpn_limb_t) each */
    unsigned int size = (1 << winsize) * msize;
    return (unsigned int)MPI_ALIGNED_SIZE(size, MPI_CACHE_LINE_BYTES / sizeof(mpn_limb_t));
}

static void mpi_scramble_put(mpn_limb_t *tbl, unsigned int idx, const mpn_limb_t *val, unsigned int vLen,
                             unsigned int winsize)
{
    unsigned int width = 1 << winsize;
    for (unsigned int i = 0, j = idx; i < vLen; i++, j += width) { tbl[j] = val[i]; }
}

static void mpi_scramble_get(mpn_limb_t *val, unsigned int vLen, const mpn_limb_t *tbl, unsigned int idx,
                             unsigned int winsize)
{
    unsigned int width = 1 << winsize;
    for (unsigned int i = 0, j = idx; i < vLen; i++, j += width) { val[i] = tbl[j]; }
}

static void mpi_scramble_get_sscm(mpn_limb_t *val, unsigned int vlen, const mpn_limb_t *tbl, unsigned int idx,
                                  unsigned int winsize)
{
    mpn_limb_t mask[1 << MPI_LOG_CACHE_LINE_SIZE];

    unsigned int width = 1 << winsize;
    for (unsigned n = 0; n < width; n++) { mask[n] = mpn_limb_is_zero_consttime((mpn_limb_t)n ^ (mpn_limb_t)idx); }

    for (unsigned i = 0; i < vlen; i++, tbl += width) {
        mpn_limb_t acc = 0;
        for (unsigned n = 0; n < width; n++) { acc |= tbl[n] & mask[n]; }
        val[i] = acc;
    }
}

/**
 * optimal size of fixed window exponentiation
 */
MPN_INLINE unsigned int mont_exp_win_size(unsigned int bits)
{
    // clang-format off
    unsigned int size = bits > 4096 ? 6 : /* 4097 - ...  */
                        bits > 2666 ? 5 : /* 2667 - 4096 */
                        bits >  717 ? 4 : /*  718 - 2666 */
                        bits >  178 ? 3 : /*  179 -  717 */
                        bits >   41 ? 2 : /*   42 -  178 */
                        1;                /*    1 -   41 */
    // clang-format on

    // limited by cache-line size
    return size <= MPI_LOG_CACHE_LINE_SIZE ? size : MPI_LOG_CACHE_LINE_SIZE;
}

/**
 * mpn montgomery: fixed-size window montgomery exponentiation
 *
 * @note:
 *   1. inputs and outputs are in montgomery domain
 *   2. possible inplace mode
 *   3. size of buffer: precomuted table of multipliers[(2 ^ w) * msize] + temp result if inplace
 * operation[msize] + power expasin[msize + 1]
 */
unsigned int mpn_montgomery_exp(mpn_limb_t *y, const mpn_limb_t *x, unsigned int xsize, const mpn_limb_t *e,
                                unsigned int ebits, mpn_montgomery_t *mont)
{
    unsigned int msize = mont->modsize;
    unsigned int esize = MPN_BITS_TO_LIMBS(ebits);

    if (mpn_is_zero_consttime(e, esize)) { // special case: x ^ 0 = 1
        COPY(y, mont->montR, msize);
    } else if (mpn_is_zero_consttime(x, xsize)) { // special case: 0 ^ e = 0
        ZEROIZE(y, 0, msize);
    } else { /* general case */
        unsigned int winsize = mont_exp_win_size(ebits);
        unsigned int nprecomute = 1 << winsize;
        mpn_limb_t mask = (mpn_limb_t)(nprecomute - 1);

        mpn_limb_t *table =
            mpn_optimizer_get_limbs(mont->optimizer, mpi_scramble_buffer_size(msize, winsize)); /* pre-computed table */
        mpn_limb_t *dataTT = mpn_optimizer_get_limbs(mont->optimizer, msize + 1); /* tmp unscrambled table entry */
        mpn_limb_t *dataEE = dataTT; /* zero expanded exponent | "masked" multipler (X|1) */

        /* copy and expand base to the modulus length */
        ZEXPAND(dataTT, msize, x, xsize);

        /* initialize recource */
        COPY(table, mont->montR, msize);
        COPY(table + msize, dataTT, msize);
        for (unsigned int n = 2; n < nprecomute; n++) {
            mpn_montgomery_mul(table + n * msize, table + (n - 1) * msize, dataTT, mont);
        }

        /* expand exponent*/
        ZEXPAND(dataEE, esize + 1, e, esize);
        ebits = ((ebits + winsize - 1) / winsize) * winsize;

        /* exponentiation */
        {
            /* position of the 1-st (left) window */
            int ebit = (int)(ebits - winsize);

            /* Note: Static analysis can generate error/warning on the expression below.

            The value of "bitSizeE" is limited, ((modulusBitSize > bitSizeE > 0),
            it is checked in initialization phase by (ippsRSA_GetSizePublickey() and ippsRSA_InitPublicKey).
            Buffer "dataEE" assigned for copy of e, is 1 (64-bit) chunk longer than size of RSA modulus,
            therefore the access "*((uint32_t*)((uint16_t*)dataEE+ eBit/16))" is always inside the boundary.
            */
            /* extract 1-st window value */
            uint32_t echunk = *((uint32_t *)((uint16_t *)dataEE + ebit / 16));
            int shift = ebit & 0xF;
            uint32_t winVal = (echunk >> shift) & mask;

            /* initialize result */
            COPY(y, table + winVal * (uint32_t)msize, msize);

            for (ebit -= winsize; ebit >= 0; ebit -= winsize) {
                /* do square window times */
                for (unsigned int n = 0; n < winsize; n++) { mpn_montgomery_square(y, y, mont); }

                /* extract next window value */
                echunk = *((uint32_t *)((uint16_t *)dataEE + ebit / 16));
                shift = ebit & 0xF;
                winVal = (echunk >> shift) & mask;

                /* muptiply precomputed value  */
                mpn_montgomery_mul(y, y, table + winVal * (uint32_t)msize, mont);
            }
        }

        mpn_optimizer_put_limbs(mont->optimizer, msize + 1);
        mpn_optimizer_put_limbs(mont->optimizer, mpi_scramble_buffer_size(msize, winsize));
    }

    return msize;
}

/**
 * mpn montgomery: fixed-size window montgomery exponentiation(constant-time version)
 *
 * @note:
 *   1. inputs and outputs are in montgomery domain
 *   2. possible inplace mode
 *   3. size of buffer: precomuted table of multipliers[(2 ^ w) * msize] + temp result if inplace
 * operation[msize] + unscrmbled table entry[msize] + power expasin[msize + 1]
 */
unsigned int mpn_montgomery_exp_consttime(mpn_limb_t *y, const mpn_limb_t *x, unsigned int xsize, const mpn_limb_t *e,
                                          unsigned int ebits, mpn_montgomery_t *mont)
{
    unsigned int msize = mont->modsize;
    unsigned int esize = MPN_BITS_TO_LIMBS(ebits);

    if (mpn_is_zero_consttime(e, esize)) { // special case: x ^ 0 = 1
        COPY(y, mont->montR, msize);
    } else if (mpn_is_zero_consttime(x, xsize)) { // special case: 0 ^ e = 0
        ZEROIZE(y, 0, msize);
    } else { /* general case */
        unsigned int winsize = mont_exp_win_size(ebits);
        unsigned int nprecomute = 1 << winsize;
        mpn_limb_t mask = (mpn_limb_t)(nprecomute - 1);

        mpn_limb_t *table =
            mpn_optimizer_get_limbs(mont->optimizer, mpi_scramble_buffer_size(msize, winsize)); /* pre-computed table */
        mpn_limb_t *dataTT = mpn_optimizer_get_limbs(mont->optimizer, msize); /* tmp unscrambled table entry */
        mpn_limb_t *dataRR =
            mpn_optimizer_get_limbs(mont->optimizer, msize + 1); /* zero expanded exponent | "masked" multipler (X|1) */
        mpn_limb_t *dataEE = dataRR;

        /* copy and expand base to the modulus length */
        ZEXPAND(dataTT, msize, x, xsize);

        /* initialize recource */
        mpi_scramble_put(table, 0, mont->montR, msize, winsize);
        COPY(dataRR, dataTT, msize);
        mpi_scramble_put(table, 1, dataTT, msize, winsize);
        for (unsigned int n = 2; n < nprecomute; n++) {
            mpn_montgomery_mul(dataTT, dataTT, dataRR, mont);
            mpi_scramble_put(table, n, dataTT, msize, winsize);
        }

        /* expand exponent*/
        ZEXPAND(dataEE, msize + 1, e, esize);
        ebits = ((ebits + winsize - 1) / winsize) * winsize;

        /* exponentiation */
        {
            /* position of the 1-st (left) window */
            int ebit = (int)(ebits - winsize);

            /* extract 1-st window value */
            uint32_t echunk = *((uint32_t *)((uint16_t *)dataEE + ebit / 16));
            int shift = ebit & 0xF;
            uint32_t winVal = (echunk >> shift) & mask;

            /* initialize result */
            mpi_scramble_get_sscm(y, msize, table, (unsigned int)winVal, winsize);

            for (ebit -= winsize; ebit >= 0; ebit -= winsize) {
                /* do square window times */
                for (unsigned int n = 0; n < winsize; n++) { mpn_montgomery_square(y, y, mont); }

                /* extract next window value */
                echunk = *((uint32_t *)((uint16_t *)dataEE + ebit / 16));
                shift = ebit & 0xF;
                winVal = (echunk >> shift) & mask;

                /* exptact precomputed value and muptiply */
                mpi_scramble_get_sscm(dataTT, msize, table, (unsigned int)winVal, winsize);

                mpn_montgomery_mul(y, y, dataTT, mont);
            }
        }

        mpn_optimizer_put_limbs(mont->optimizer, msize + 1);
        mpn_optimizer_put_limbs(mont->optimizer, msize);
        mpn_optimizer_put_limbs(mont->optimizer, mpi_scramble_buffer_size(msize, winsize));
    }

    return msize;
}
#endif

MPN_INLINE size_t montgomery_size(unsigned int mbits)
{
    size_t size = sizeof(mpn_montgomery_t) + MPN_LIMB_BYTES;
    size += MPN_BITS_TO_LIMBS(mbits) * sizeof(mpn_limb_t) * 4 /* modulus[msize], montR[msize], montR^2[2 * msize] */;

    return size;
}

/**
 * mpn montgomery: create montgomery context
 *
 */
mpn_montgomery_t *mpn_montgomery_create(unsigned int mbits, unsigned int psize)
{
    if (mbits == 0) {
        MPI_RAISE_ERROR(-EINVAL, "modbits MUST bigger than 0.");
        return NULL;
    }
    if (psize < MPN_BITS_TO_LIMBS(mbits)) {
        MPI_RAISE_ERROR(-EINVAL, "pool-size MUST bigger than %u.", MPN_BITS_TO_LIMBS(mbits));
        return NULL;
    }

    mpn_montgomery_t *mont = NULL;
    if ((mont = (mpn_montgomery_t *)MPI_ALLOCATE(montgomery_size(mbits))) != NULL) {
        unsigned int msize = MPN_BITS_TO_LIMBS(mbits);
        mpn_limb_t *chunk = mpi_aligned_pointer((unsigned char *)mont + sizeof(mpn_montgomery_t), MPN_LIMB_BYTES);

        mont->modbits = mbits;
        mont->modsize = msize;
        mont->modulus = chunk;           /* msize */
        mont->montR = (chunk += msize);  /* msize */
        mont->montRR = (chunk += msize); /* 2 * msize */

        mont->optimizer = mpn_optimizer_create(psize);
    }

    return mont;
}

/**
 * mpn montgomery: destory montgomery context
 *
 */
void mpn_montgomery_destory(mpn_montgomery_t *mont)
{
    if (mont != NULL) {
        mpn_optimizer_destory(mont->optimizer);
        void *(*volatile memset_ensure)(void *, int, size_t) = memset;
        memset_ensure(mont, 0, montgomery_size(mont->modbits));
        MPI_DEALLOCATE(mont);
    }
}

/**
 * mpn montgomery: intialize montgomery context with modulus
 *
 */
int mpn_montgomery_set_modulus_bin(mpn_montgomery_t *mont, const mpn_limb_t *modulus, unsigned int mbits)
{
    if (mont == NULL || modulus == NULL) {
        MPI_RAISE_ERROR(-EINVAL);
        return -EINVAL;
    }

    {
        unsigned int msize = MPN_BITS_TO_LIMBS(mbits);
        if (msize != mont->modsize) { return -EINVAL; }

        /* store modulus */
        if (mont->modulus != modulus) { ZEXPAND(mont->modulus, msize, modulus, msize); }

        /* montgomery factor */
        mont->k0 = mpn_montgomery_factor(modulus[0]);

        /* montgomery identity (R) */
        ZEROIZE(mont->montR, 0, msize);
        mont->montR[msize] = 1;

        mpn_mod(mont->montR, msize + 1, mont->modulus, msize);

        /* montgomery domain converter (RR) */
        ZEROIZE(mont->montRR, 0, msize);
        COPY(mont->montRR + msize, mont->montR, msize);

        mpn_mod(mont->montRR, 2 * msize, mont->modulus, msize);
    }

    return 0;
}
