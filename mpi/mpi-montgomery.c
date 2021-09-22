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
#include "mpi.h"
#include "mpi-binary.h"
#include "mpi-montgomery.h"

#if !(defined MPI_MONT_REDC_ASM)
/**
 * mpi montgomery: montgomery reduction
 *
 * @note:
 *   1. m0: low word of (1 / modulus) mod b
 *   2. r = T/R mod m
 */
void mpi_montgomery_reduce_bin(mpi_limb_t *r, mpi_limb_t *product, const mpi_limb_t *m, unsigned int msize, mpi_limb_t m0)
{
    MPI_ASSERT(msize > 0);
    mpi_limb_t carry = 0, extension;

    for (unsigned int n = 0; n < (msize - 1); n++) {
        mpi_limb_t u = product[n] * m0;
        mpi_limb_t t = product[msize + n + 1] + carry;

        extension = mpi_umul_acc_bin(product + n, m, msize, u);
        UADD_AB(carry, product[msize + n], product[msize + n], extension);
        t += carry;

        carry = t < product[msize + n + 1];
        product[msize + n + 1] = t;
    }

    m0 *= product[msize - 1];
    extension = mpi_umul_acc_bin(product + msize - 1, m, msize, m0);
    UADD_AB(extension, product[2 * msize - 1], product[2 * msize - 1], extension);

    carry |= extension;
    carry -= mpi_usub_school_bin(r, product + msize, m, msize);
    /* condition copy: R = carry ? Product + mSize : R */
    mpi_masked_copy_consttime(r, product + msize, r, msize, carry);
}
#endif

/**
 * mpi montgomery: r[] = to_mont(a[])
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. memory size from the pool: modsize * sizeof(mpi_limb_t)
 */
void mpi_montgomery_enc_bin(mpi_limb_t *r, const mpi_limb_t *a, mpi_montgomery_t *mont)
{
#ifdef MPI_LOW_FOOTPRINT
    mpi_montgomery_mul_bin(r, a, mont->montRR, mont);
#else
    MPI_ASSERT(r != NULL && a != NULL && mont != NULL);

    unsigned int msize = mont->modsize;
    mpi_limb_t *product = mpi_optimizer_get_limbs(mont->optimizer, 2 * msize);
    MPI_ASSERT(product != NULL);

    {
        /**
         * @performance:
         *   For __ARCH32E >= __ARCH32E_L9, ADX instruction would be the better choose
         */
        mpi_umul_bin(product, a, msize, mont->montRR, msize);
        mpi_montgomery_reduce_bin(r, product, mont->modulus, msize, mont->k0);
    }

    mpi_optimizer_put_limbs(mont->optimizer, 2 * msize);
#endif
}

/**
 * mpi montgomery: r[] = from_mont(a)
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. memory size from the pool: modsize * sizeof(mpi_limb_t)
 */
void mpi_montgomery_dec_bin(mpi_limb_t *r, const mpi_limb_t *a, mpi_montgomery_t *mont)
{
    MPI_ASSERT(r != NULL && a != NULL && mont != NULL);

    unsigned int msize = mont->modsize;
    mpi_limb_t *product = mpi_optimizer_get_limbs(mont->optimizer, 2 * msize);
    MPI_ASSERT(NULL != product);

    {
        ZEXPAND(product, 2 * msize, a, msize);
        mpi_montgomery_reduce_bin(r, product, mont->modulus, msize, mont->k0);
    }

    mpi_optimizer_put_limbs(mont->optimizer, 2 * msize);
}

/**
 * mpi montgomery: r = (a + b) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. length of b: modsize
 *   4. memory size from the pool: modsize * sizeof(mpi_limb_t)
 */
void mpi_montgomery_add_bin(mpi_limb_t *r, const mpi_limb_t *a, const mpi_limb_t *b, mpi_montgomery_t *mont)
{
    MPI_ASSERT(r != NULL && a != NULL && b != NULL && mont != NULL);

    unsigned int msize = mont->modsize;
    mpi_limb_t *buffer = mpi_optimizer_get_limbs(mont->optimizer, msize);
    MPI_ASSERT(NULL != buffer);

    {
        mpi_limb_t extension = mpi_uadd_school_bin(r, a, b, msize);
        extension -= mpi_usub_school_bin(buffer, r, mont->modulus, msize);
        mpi_masked_move_consttime(r, buffer, msize, extension == 0);
    }

    mpi_optimizer_put_limbs(mont->optimizer, msize);
}

/**
 * mpi montgomery: r = (a - b) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. length of b: modsize
 *   4. memory size from the pool: modsize * sizeof(mpi_limb_t)
 */
void mpi_montgomery_sub_bin(mpi_limb_t *r, const mpi_limb_t *a, const mpi_limb_t *b, mpi_montgomery_t *mont)
{
    MPI_ASSERT(r != NULL && a != NULL && b != NULL && mont != NULL);

    unsigned int msize = mont->modsize;
    mpi_limb_t *buffer = mpi_optimizer_get_limbs(mont->optimizer, msize);
    MPI_ASSERT(NULL != buffer);

    {
        mpi_limb_t extension = mpi_usub_school_bin(r, a, b, msize);
        mpi_uadd_school_bin(buffer, r, mont->modulus, msize);
        mpi_masked_move_consttime(r, buffer, msize, extension != 0);
    }

    mpi_optimizer_put_limbs(mont->optimizer, msize);
}

/**
 * mpi montgomery: r = -b mod m = (m - b) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   4. memory size from the pool: modsize * sizeof(mpi_limb_t)
 */
void mpi_montgomery_neg_bin(mpi_limb_t *r, const mpi_limb_t *a, mpi_montgomery_t *mont)
{
    MPI_ASSERT(r != NULL && a != NULL && mont != NULL);

    unsigned int msize = mont->modsize;
    mpi_limb_t *buffer = mpi_optimizer_get_limbs(mont->optimizer, msize);
    MPI_ASSERT(NULL != buffer);

    {
        const mpi_limb_t *m = mont->modulus;
        mpi_limb_t extension = mpi_usub_school_bin(r, m, a, msize);
        extension -= mpi_usub_school_bin(buffer, r, m, msize);
        mpi_masked_move_consttime(r, buffer, msize, extension == 0);
    }

    mpi_optimizer_put_limbs(mont->optimizer, msize);
}

/**
 * mpi montgomery: r = (a / 2) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. memory size from the pool: modsize * sizeof(mpi_limb_t)
 */
void mpi_montgomery_div2_bin(mpi_limb_t *r, const mpi_limb_t *a, mpi_montgomery_t *mont)
{
    MPI_ASSERT(r != NULL && a != NULL && mont != NULL);

    unsigned int msize = mont->modsize;
    mpi_limb_t *buffer = mpi_optimizer_get_limbs(mont->optimizer, msize);
    MPI_ASSERT(NULL != buffer);

    {
        mpi_limb_t mask = 0 - (a[0] & 1);
        const mpi_limb_t *m = mont->modulus;
        for (unsigned int i = 0; i < msize; i++) { buffer[i] = m[i] & mask; }

        buffer[msize] = mpi_uadd_school_bin(buffer, buffer, a, msize);
        mpi_rshift_bin(buffer, buffer, msize + 1, 1);

        COPY(r, buffer, msize);
    }

    mpi_optimizer_put_limbs(mont->optimizer, msize);
}

/**
 * mpi montgomery: r = (a * 2) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   4. memory size from the pool: modsize * sizeof(mpi_limb_t)
 */
void mpi_montgomery_mul2_bin(mpi_limb_t *r, const mpi_limb_t *a, mpi_montgomery_t *mont)
{
    mpi_montgomery_add_bin(r, a, a, mont);
}

/**
 * mpi montgomery: r = (a * 3) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   4. memory size from the pool: modsize * sizeof(mpi_limb_t)
 */
void mpi_montgomery_mul3_bin(mpi_limb_t *r, const mpi_limb_t *a, mpi_montgomery_t *mont)
{
    MPI_ASSERT(r != NULL && a != NULL && mont != NULL);

    unsigned int msize = mont->modsize;
    mpi_limb_t *buffer = mpi_optimizer_get_limbs(mont->optimizer, msize);
    MPI_ASSERT(NULL != buffer);

    {
        mpi_montgomery_add_bin(buffer, a, a, mont);
        mpi_montgomery_add_bin(r, a, buffer, mont);
    }

    mpi_optimizer_put_limbs(mont->optimizer, msize);
}

/**
 * mpi montgomery: r = prod mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of rod: modsize
 *   4. memory size from the pool: N/A
 */
void mpi_montgomery_red_bin(mpi_limb_t *r, mpi_limb_t *prod, mpi_montgomery_t *mont)
{
    MPI_ASSERT(r != NULL && prod != NULL && mont != NULL);

    /**
     * @performance:
     *   for ARCH < ARCH_M7 or ARCH32E < ARCH32E_M7, this implementation would be the better choose
     */
#ifdef MPI_USE_C_MONTGOMERY_RED_BIN
    /* mont mul */
    mpi_limb_t carry = 0;
    unsigned int msize = mont->modsize;
    const mpi_limb_t *pm = mont->modulus, k0 = mont->k0;

    for (unsigned int i = 0; i < msize; i++, prod++) {
        mpi_limb_t rL, rH, extention, temp;

        /* u = prod[0]*k0 mod B */
        mpi_limb_t u = prod[0] * k0;

        /* (extention, temp) = prod[0] + m[0]*u (note temp ==0) */
        UMUL_AB(rH, rL, pm[0], u);
        UADD_AB(extention, temp, prod[0], rL);
        extention += rH;

        for (unsigned int j = 1; j < msize; j++) {
            mpi_limb_t c;
            UMUL_AB(rH, rL, pm[j], u);                    /* (H,L) = m[j]*u */
            UADD_AB(extention, temp, prod[j], extention); /* carry in extention,temp */
            UADD_AB(c, prod[j], temp, rL);                /* carry in c */
            extention += rH + c;                          /* accumulates both carrys above */
        }
        UADD_ABC(carry, prod[msize], prod[msize], extention, carry);
    }

    {
        carry -= mpi_usub_school_bin(r, prod, pm, msize);
        mpi_masked_move_consttime(r, prod, msize, carry != 0);
    }
#else
    mpi_montgomery_reduce_bin(r, prod, mont->modulus, mont->modsize, mont->k0);
#endif
}

/**
 * mpi montgomery: r = (a * b) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. length of b: modsize
 *   4. memory size from the pool: modsize * sizeof(mpi_limb_t) * 2
 */
void mpi_montgomery_mul_bin(mpi_limb_t *r, const mpi_limb_t *a, const mpi_limb_t *b, mpi_montgomery_t *mont)
{
    MPI_ASSERT(r != NULL && a != NULL && b != NULL && mont != NULL);

    /**
     * @performance:
     *   for ARCH < ARCH_M7 or ARCH32E < ARCH32E_M7, this implementation would be the better choose
     */
#ifdef MPI_USE_C_MONTGOMERY_MUL_BIN
    unsigned int msize = mont->modsize;

    mpi_limb_t *buffer = mpi_optimizer_get_limbs(mont->optimizer, msize);
    MPI_ASSERT(NULL != buffer);

    {
        mpi_limb_t carry = 0;
        const mpi_limb_t *pm = mont->modulus, m0 = mont->k0;

        /* clear buffer */
        ZEROIZE(buffer, 0, msize);

        /* mont mul */
        for (unsigned int i = 0; i < msize; i++) {
            mpi_limb_t bb = b[i];

            mpi_limb_t extAB = 0, extMU = 0;
            mpi_limb_t abL, abH, muL, muH, u;

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

        carry -= mpi_usub_school_bin(r, buffer, pm, msize);
        mpi_masked_move_consttime(r, buffer, msize, carry != 0);
    }

    mpi_optimizer_put_limbs(mont->optimizer, msize);
#else
    unsigned int msize = mont->modsize;
    mpi_limb_t *product = mpi_optimizer_get_limbs(mont->optimizer, 2 * msize);
    MPI_ASSERT(NULL != product);

    {
        /**
         * @performance:
         *   For __ARCH32E >= __ARCH32E_L9, ADX instruction would be the better choose
         */
        mpi_umul_bin(product, a, msize, b, msize);
        mpi_montgomery_reduce_bin(r, product, mont->modulus, msize, mont->k0);
    }

    mpi_optimizer_put_limbs(mont->optimizer, 2 * msize);
#endif
}

/**
 * mpi montgomery: r = (a ^ 2) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   4. memory size from the pool: modsize * sizeof(mpi_limb_t) * 2
 */
void mpi_montgomery_sqr_bin(mpi_limb_t *r, const mpi_limb_t *a, mpi_montgomery_t *mont)
{
#ifdef MPI_LOW_FOOTPRINT
    return mpi_montgomery_mul_bin(r, a, a, mont);
#else
    MPI_ASSERT(r != NULL && a != NULL && mont != NULL);

    unsigned int msize = mont->modsize;
    mpi_limb_t *product = mpi_optimizer_get_limbs(mont->optimizer, 2 * msize);
    MPI_ASSERT(NULL != product);

    {
        /**
         * @performance:
         *   For __ARCH32E >= __ARCH32E_L9, ADX instruction would be the better choose
         */
        mpi_usqr_bin(product, a, msize);
        mpi_montgomery_reduce_bin(r, product, mont->modulus, msize, mont->k0);
    }

    mpi_optimizer_put_limbs(mont->optimizer, 2 * msize);
#endif
}

/**
 * montfomery factor k0 = -((modulus^-1 mod B) %B)
 */
mpi_limb_t mpi_montgomery_factor(mpi_limb_t m0)
{
    mpi_limb_t x = 2, y = 1;
    mpi_limb_t mask = 2 * x - 1;
    for (unsigned i = 2; i <= MPI_LIMB_BITS; i++, x <<= 1) {
        mpi_limb_t rH, rL;
        UMUL_AB(rH, rL, m0, y);
        if (x < (rL & mask)) { /* x < ((m0*y) mod (2*x)) */
            y += x;
        }
        mask += mask + 1;
        (void)rH;
    }
    return (mpi_limb_t)(0 - y);
}

#ifndef MPI_USE_SLIDING_WINDOW_EXP
/**
 * mpi montgomery: binary montgomery exponentiation
 *
 * @note:
 *   1. inputs and outputs are in montgomery domain
 *   2. possible inplace mode
 *   3. minimal size temporary memory chunk: modsize
 */
unsigned int mpi_montgomery_exp_bin(mpi_limb_t *y, const mpi_limb_t *x, unsigned int xsize, const mpi_limb_t *e, unsigned int ebits, mpi_montgomery_t *mont)
{
    unsigned int msize = mont->modsize;
    unsigned int esize = MPI_BITS_TO_LIMBS(ebits);

    if (mpi_is_zero_consttime_bin(e, esize)) { // special case: x ^ 0 = 1
        COPY(y, mont->montR, msize);
    } else if (mpi_is_zero_consttime_bin(x, xsize)) { // special case: 0 ^ e = 0
        ZEROIZE(y, 0, msize);
    } else { /* general case */
        /* allocate buffers */
        mpi_limb_t *dataT = mpi_optimizer_get_limbs(mont->optimizer, msize);
        MPI_ASSERT(dataT != NULL);

        /* copy and expand base to the modulus length */
        ZEXPAND(dataT, msize, x, xsize);
        /* copy */
        COPY(y, dataT, msize);

        esize = mpi_fix_size_bin(e, esize);

        /* execute most significant part pE */
        {
            mpi_limb_t ee = e[esize - 1];
            unsigned int n = mpi_nlz_limb_consttime(ee) + 1;

            ee <<= n;
            for (; n < MPI_LIMB_BITS; ee <<= 1, n++) {
                /* squaring R = R*R mod Modulus */
                mpi_montgomery_sqr_bin(y, y, mont);
                /* and multiply R = R*X mod Modulus */
                if (ee & ((mpi_limb_t)1 << (MPI_LIMB_BITS - 1))) { mpi_montgomery_mul_bin(y, y, dataT, mont); }
            }

            /* execute rest bits of E */
            for (--esize; esize > 0; esize--) {
                ee = e[esize - 1];

                for (n = 0; n < MPI_LIMB_BITS; ee <<= 1, n++) {
                    /* squaring: R = R*R mod Modulus */
                    mpi_montgomery_sqr_bin(y, y, mont);

                    if (ee & ((mpi_limb_t)1 << (MPI_LIMB_BITS - 1))) { mpi_montgomery_mul_bin(y, y, dataT, mont); }
                }
            }
        }

        mpi_optimizer_put_limbs(mont->optimizer, msize);
    }

    return msize;
}

/**
 * mpi montgomery: binary montgomery exponentiation(constant-time version)
 *
 * @note:
 *   1. inputs and outputs are in montgomery domain
 *   2. possible inplace mode
 *   3. minimal size temporary memory chunk: modsize * 2
 */
unsigned int mpi_montgomery_exp_consttime_bin(mpi_limb_t *y, const mpi_limb_t *x, unsigned int xsize, const mpi_limb_t *e, unsigned int ebits, mpi_montgomery_t *mont)
{
    unsigned int msize = mont->modsize;
    unsigned int esize = MPI_BITS_TO_LIMBS(ebits);

    if (mpi_is_zero_consttime_bin(e, esize)) { // special case: x ^ 0 = 1
        COPY(y, mont->montR, msize);
    } else if (mpi_is_zero_consttime_bin(x, xsize)) { // special case: 0 ^ e = 0
        ZEROIZE(y, 0, msize);
    } else { /* general case */
        /* allocate buffers */
        mpi_limb_t *dataT = mpi_optimizer_get_limbs(mont->optimizer, msize);
        mpi_limb_t *sscmB = mpi_optimizer_get_limbs(mont->optimizer, msize);
        MPI_ASSERT(dataT != NULL && sscmB != NULL);

        /* mont(1) */
        mpi_limb_t *montR = mont->montR;

        /* copy and expand base to the modulus length */
        ZEXPAND(dataT, msize, x, xsize);
        /* init result */
        COPY(y, montR, msize);

        /* execute bits of E */
        for (; esize > 0; esize--) {
            mpi_limb_t ee = e[esize - 1];
            for (unsigned n = MPI_LIMB_BITS; n > 0; ee <<= 1, n--) {
                /* sscmB = msb(ee) ? X : mont(1) */
                mpi_limb_t mask = mpi_test_msb_limb_consttime(ee);
                mpi_masked_copy_consttime(sscmB, dataT, montR, msize, mask);

                /* squaring Y = Y^2 */
                mpi_montgomery_sqr_bin(y, y, mont);
                /* and multiplication: Y = Y * sscmB */
                mpi_montgomery_mul_bin(y, y, sscmB, mont);
            }
        }

        mpi_optimizer_put_limbs(mont->optimizer, msize);
        mpi_optimizer_put_limbs(mont->optimizer, msize);
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
    /* size of resource to store 2 ^ winsize values of msize * sizeof(mpi_limb_t) each */
    unsigned int size = (1 << winsize) * msize;
    return (unsigned int)MPI_ALIGNED_SIZE(size, MPI_CACHE_LINE_BYTES / sizeof(mpi_limb_t));
}

static void mpi_scramble_put(mpi_limb_t *tbl, unsigned int idx, const mpi_limb_t *val, unsigned int vLen, unsigned int winsize)
{
    unsigned int width = 1 << winsize;
    for (unsigned int i = 0, j = idx; i < vLen; i++, j += width) { tbl[j] = val[i]; }
}

static void mpi_scramble_get(mpi_limb_t *val, unsigned int vLen, const mpi_limb_t *tbl, unsigned int idx, unsigned int winsize)
{
    unsigned int width = 1 << winsize;
    for (unsigned int i = 0, j = idx; i < vLen; i++, j += width) { val[i] = tbl[j]; }
}

static void mpi_scramble_get_sscm(mpi_limb_t *val, unsigned int vlen, const mpi_limb_t *tbl, unsigned int idx, unsigned int winsize)
{
    mpi_limb_t mask[1 << MPI_LOG_CACHE_LINE_SIZE];

    unsigned int width = 1 << winsize;
    for (unsigned n = 0; n < width; n++) { mask[n] = mpi_is_zero_limb_consttime((mpi_limb_t)n ^ (mpi_limb_t)idx); }

    for (unsigned i = 0; i < vlen; i++, tbl += width) {
        mpi_limb_t acc = 0;
        for (unsigned n = 0; n < width; n++) { acc |= tbl[n] & mask[n]; }
        val[i] = acc;
    }
}

/**
 * optimal size of fixed window exponentiation
 */
MPI_INLINE unsigned int mont_exp_win_size(unsigned int bits)
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
 * mpi montgomery: fixed-size window montgomery exponentiation
 *
 * @note:
 *   1. inputs and outputs are in montgomery domain
 *   2. possible inplace mode
 *   3. size of buffer: precomuted table of multipliers[(2 ^ w) * msize] + temp result if inplace operation[msize] + power expasin[msize + 1]
 */
unsigned int mpi_montgomery_exp_bin(mpi_limb_t *y, const mpi_limb_t *x, unsigned int xsize, const mpi_limb_t *e, unsigned int ebits, mpi_montgomery_t *mont)
{
    unsigned int msize = mont->modsize;
    unsigned int esize = MPI_BITS_TO_LIMBS(ebits);

    if (mpi_is_zero_consttime_bin(e, esize)) { // special case: x ^ 0 = 1
        COPY(y, mont->montR, msize);
    } else if (mpi_is_zero_consttime_bin(x, xsize)) { // special case: 0 ^ e = 0
        ZEROIZE(y, 0, msize);
    } else { /* general case */
        unsigned int winsize = mont_exp_win_size(ebits);
        unsigned int nprecomute = 1 << winsize;
        mpi_limb_t mask = (mpi_limb_t)(nprecomute - 1);

        mpi_limb_t *table = mpi_optimizer_get_limbs(mont->optimizer, mpi_scramble_buffer_size(msize, winsize)); /* pre-computed table */
        mpi_limb_t *dataTT = mpi_optimizer_get_limbs(mont->optimizer, msize + 1);                               /* tmp unscrambled table entry */
        mpi_limb_t *dataEE = dataTT;                                                                            /* zero expanded exponent | "masked" multipler (X|1) */

        /* copy and expand base to the modulus length */
        ZEXPAND(dataTT, msize, x, xsize);

        /* initialize recource */
        COPY(table, mont->montR, msize);
        COPY(table + msize, dataTT, msize);
        for (unsigned int n = 2; n < nprecomute; n++) { mpi_montgomery_mul_bin(table + n * msize, table + (n - 1) * msize, dataTT, mont); }

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
                for (unsigned int n = 0; n < winsize; n++) { mpi_montgomery_sqr_bin(y, y, mont); }

                /* extract next window value */
                echunk = *((uint32_t *)((uint16_t *)dataEE + ebit / 16));
                shift = ebit & 0xF;
                winVal = (echunk >> shift) & mask;

                /* muptiply precomputed value  */
                mpi_montgomery_mul_bin(y, y, table + winVal * (uint32_t)msize, mont);
            }
        }

        mpi_optimizer_put_limbs(mont->optimizer, msize + 1);
        mpi_optimizer_put_limbs(mont->optimizer, mpi_scramble_buffer_size(msize, winsize));
    }

    return msize;
}

/**
 * mpi montgomery: fixed-size window montgomery exponentiation(constant-time version)
 *
 * @note:
 *   1. inputs and outputs are in montgomery domain
 *   2. possible inplace mode
 *   3. size of buffer: precomuted table of multipliers[(2 ^ w) * msize] + temp result if inplace operation[msize] + unscrmbled table entry[msize] + power expasin[msize + 1]
 */
unsigned int mpi_montgomery_exp_consttime_bin(mpi_limb_t *y, const mpi_limb_t *x, unsigned int xsize, const mpi_limb_t *e, unsigned int ebits, mpi_montgomery_t *mont)
{
    unsigned int msize = mont->modsize;
    unsigned int esize = MPI_BITS_TO_LIMBS(ebits);

    if (mpi_is_zero_consttime_bin(e, esize)) { // special case: x ^ 0 = 1
        COPY(y, mont->montR, msize);
    } else if (mpi_is_zero_consttime_bin(x, xsize)) { // special case: 0 ^ e = 0
        ZEROIZE(y, 0, msize);
    } else { /* general case */
        unsigned int winsize = mont_exp_win_size(ebits);
        unsigned int nprecomute = 1 << winsize;
        mpi_limb_t mask = (mpi_limb_t)(nprecomute - 1);

        mpi_limb_t *table = mpi_optimizer_get_limbs(mont->optimizer, mpi_scramble_buffer_size(msize, winsize)); /* pre-computed table */
        mpi_limb_t *dataTT = mpi_optimizer_get_limbs(mont->optimizer, msize);                                   /* tmp unscrambled table entry */
        mpi_limb_t *dataRR = mpi_optimizer_get_limbs(mont->optimizer, msize + 1);                               /* zero expanded exponent | "masked" multipler (X|1) */
        mpi_limb_t *dataEE = dataRR;

        /* copy and expand base to the modulus length */
        ZEXPAND(dataTT, msize, x, xsize);

        /* initialize recource */
        mpi_scramble_put(table, 0, mont->montR, msize, winsize);
        COPY(dataRR, dataTT, msize);
        mpi_scramble_put(table, 1, dataTT, msize, winsize);
        for (unsigned int n = 2; n < nprecomute; n++) {
            mpi_montgomery_mul_bin(dataTT, dataTT, dataRR, mont);
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
                for (unsigned int n = 0; n < winsize; n++) { mpi_montgomery_sqr_bin(y, y, mont); }

                /* extract next window value */
                echunk = *((uint32_t *)((uint16_t *)dataEE + ebit / 16));
                shift = ebit & 0xF;
                winVal = (echunk >> shift) & mask;

                /* exptact precomputed value and muptiply */
                mpi_scramble_get_sscm(dataTT, msize, table, (unsigned int)winVal, winsize);

                mpi_montgomery_mul_bin(y, y, dataTT, mont);
            }
        }

        mpi_optimizer_put_limbs(mont->optimizer, msize + 1);
        mpi_optimizer_put_limbs(mont->optimizer, msize);
        mpi_optimizer_put_limbs(mont->optimizer, mpi_scramble_buffer_size(msize, winsize));
    }

    return msize;
}
#endif

MPI_INLINE size_t montgomery_size(unsigned int mbits)
{
    size_t size = sizeof(mpi_montgomery_t) + MPI_LIMB_BYTES;
    size += MPI_BITS_TO_LIMBS(mbits) * sizeof(mpi_limb_t) * 4 /* modulus[msize], montR[msize], montR^2[2 * msize] */;

    return size;
}

/**
 * mpi montgomery: create montgomery context
 *
 */
mpi_montgomery_t *mpi_montgomery_create(unsigned int mbits, unsigned int psize)
{
    if (mbits == 0) {
        MPI_RAISE_ERROR(-EINVAL, "modbits MUST bigger than 0.");
        return NULL;
    }
    if (psize < MPI_BITS_TO_LIMBS(mbits)) {
        MPI_RAISE_ERROR(-EINVAL, "pool-size MUST bigger than %u.", MPI_BITS_TO_LIMBS(mbits));
        return NULL;
    }

    mpi_montgomery_t *mont = NULL;
    if ((mont = (mpi_montgomery_t *)MPI_ALLOCATE(montgomery_size(mbits))) != NULL) {
        unsigned int msize = MPI_BITS_TO_LIMBS(mbits);
        mpi_limb_t *chunk = mpi_aligned_pointer((unsigned char *)mont + sizeof(mpi_montgomery_t), MPI_LIMB_BYTES);

        mont->modbits = mbits;
        mont->modsize = msize;
        mont->modulus = chunk;           /* msize */
        mont->montR = (chunk += msize);  /* msize */
        mont->montRR = (chunk += msize); /* 2 * msize */

        mont->optimizer = mpi_optimizer_create(psize);
    }

    return mont;
}

/**
 * mpi montgomery: destory montgomery context
 *
 */
void mpi_montgomery_destory(mpi_montgomery_t *mont)
{
    if (mont != NULL) {
        mpi_optimizer_destory(mont->optimizer);
        void *(*volatile memset_ensure)(void *, int, size_t) = memset;
        memset_ensure(mont, 0, montgomery_size(mont->modbits));
        MPI_DEALLOCATE(mont);
    }
}

/**
 * mpi montgomery: intialize montgomery context with modulus
 *
 */
int mpi_montgomery_set_modulus_bin(mpi_montgomery_t *mont, const mpi_limb_t *modulus, unsigned int mbits)
{
    if (mont == NULL || modulus == NULL) {
        MPI_RAISE_ERROR(-EINVAL);
        return -EINVAL;
    }

    {
        unsigned int msize = MPI_BITS_TO_LIMBS(mbits);
        if (msize != mont->modsize) { return -EINVAL; }

        /* store modulus */
        if (mont->modulus != modulus) { ZEXPAND(mont->modulus, msize, modulus, msize); }

        /* montgomery factor */
        mont->k0 = mpi_montgomery_factor(modulus[0]);

        /* montgomery identity (R) */
        ZEROIZE(mont->montR, 0, msize);
        mont->montR[msize] = 1;

        mpi_umod_bin(mont->montR, msize + 1, mont->modulus, msize);

        /* montgomery domain converter (RR) */
        ZEROIZE(mont->montRR, 0, msize);
        COPY(mont->montRR + msize, mont->montR, msize);

        mpi_umod_bin(mont->montRR, 2 * msize, mont->modulus, msize);
    }

    return 0;
}

/**
 * mpi montgomery: intialize montgomery context with modulus
 *
 */
int mpi_montgomery_set_modulus(mpi_montgomery_t *mont, const mpi_t *modulus)
{
    if (mont == NULL || modulus == NULL) {
        MPI_RAISE_ERROR(-EINVAL);
        return -EINVAL;
    }

    {
        unsigned int msize = modulus->size;
        /* store modulus */
        ZEXPAND(mont->modulus, msize, modulus->data, msize);

        /* montgomery factor */
        mont->k0 = mpi_montgomery_factor(modulus->data[0]);

        /* montgomery identity (R) */
        ZEROIZE(mont->montR, 0, msize);
        mont->montR[msize] = 1;
        mpi_umod_bin(mont->montR, msize + 1, modulus->data, msize);

        /* montgomery domain converter (RR) */
        ZEROIZE(mont->montRR, 0, msize);
        COPY(mont->montRR + msize, mont->montR, msize);
        mpi_umod_bin(mont->montRR, 2 * msize, modulus->data, msize);
    }

    return 0;
}

/**
 * mpi montgomery: exponentiation
 *
 */
int mpi_montgomery_exp(mpi_t *r, const mpi_t *x, const mpi_t *e, mpi_montgomery_t *mont)
{
    if (r == NULL || x == NULL || e == NULL || mont == NULL) {
        MPI_RAISE_ERROR(-EINVAL);
        return -EINVAL;
    }

    unsigned int msize = mont->modsize;
    if (r->room < msize || x->size > msize) {
        MPI_RAISE_ERROR(-EINVAL);
        return -EINVAL;
    }
    if (mpi_bits(x) == 0) { return mpi_zeroize(r); }
    if (mpi_bits(e) == 0) { return mpi_set_limb(r, 1); }

    {
        /* copy and expand base to the modulus length */
        ZEXPAND(r->data, msize, x->data, x->size);

        /* convert base to Montgomery domain */
        mpi_montgomery_enc_bin(r->data, r->data, mont);

        /* exponentiation */
        mpi_montgomery_exp_bin(r->data, r->data, msize, e->data, mpi_bits(e), mont);

        /* convert result back to regular domain */
        mpi_montgomery_dec_bin(r->data, r->data, mont);
    }

    r->sign = MPI_SIGN_NON_NEGTIVE;
    r->size = mpi_fix_size_bin(r->data, msize);

    return 0;
}

/**
 * mpi montgomery: exponentiation(constant-time version)
 *
 */
int mpi_montgomery_exp_consttime(mpi_t *r, const mpi_t *x, const mpi_t *e, mpi_montgomery_t *mont)
{
    if (r == NULL || x == NULL || e == NULL || mont == NULL) {
        MPI_RAISE_ERROR(-EINVAL);
        return -EINVAL;
    }

    unsigned int msize = mont->modsize;
    if (r->room < msize) {
        MPI_RAISE_ERROR(-EINVAL);
        return -EINVAL;
    }
    if (mpi_bits(x) == 0) { return mpi_zeroize(r); }
    if (mpi_bits(e) == 0) { return mpi_set_limb(r, 1); }

    {
        /* copy and expand base to the modulus length */
        ZEXPAND(r->data, msize, x->data, x->size);

        /* convert base to Montgomery domain */
        mpi_montgomery_enc_bin(r->data, r->data, mont);

        /* exponentiation */
        mpi_montgomery_exp_consttime_bin(r->data, r->data, msize, e->data, mpi_bits(e), mont);

        /* convert result back to regular domain */
        mpi_montgomery_dec_bin(r->data, r->data, mont);
    }

    r->sign = MPI_SIGN_NON_NEGTIVE;
    r->size = mpi_fix_size_bin(r->data, msize);

    return 0;
}
