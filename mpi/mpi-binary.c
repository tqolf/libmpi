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
#include "mpi-binary.h"
#include "mpi-compiler.h"

/**
 * leading zeros counting(constant-time version)
 */
unsigned int mpi_nlz_limb_consttime(mpi_limb_t x)
{
    unsigned int nlz = 0;
    mpi_limb_t mask;
#if (MPI_LIMB_BITS == 64)
    mask = mpi_is_zero_limb_consttime(x & 0xFFFFFFFF00000000);
    nlz += (unsigned int)(32 & mask);
    x = ((x << 32) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0xFFFF000000000000);
    nlz += (unsigned int)(16 & mask);
    x = ((x << 16) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0xFF00000000000000);
    nlz += (unsigned int)(8 & mask);
    x = ((x << 8) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0xF000000000000000);
    nlz += (unsigned int)(4 & mask);
    x = ((x << 4) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0xC000000000000000);
    nlz += (unsigned int)(2 & mask);
    x = ((x << 2) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x8000000000000000);
    nlz += (unsigned int)(1 & mask);
    x = ((x << 1) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x8000000000000000);
    nlz += (unsigned int)(1 & mask);
#elif MPI_LIMB_BITS == 32
    mask = mpi_is_zero_limb_consttime(x & 0xFFFF0000);
    nlz += (unsigned int)(16 & mask);
    x = ((x << 16) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0xFF000000);
    nlz += (unsigned int)(8 & mask);
    x = ((x << 8) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0xF0000000);
    nlz += (unsigned int)(4 & mask);
    x = ((x << 4) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0xC0000000);
    nlz += (unsigned int)(2 & mask);
    x = ((x << 2) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x80000000);
    nlz += (unsigned int)(1 & mask);
    x = ((x << 1) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x80000000);
    nlz += (unsigned int)(1 & mask);
#elif MPI_LIMB_BITS == 16
    mask = mpi_is_zero_limb_consttime(x & 0xFF00);
    nlz += (unsigned int)(8 & mask);
    x = ((x << 8) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0xF000);
    nlz += (unsigned int)(4 & mask);
    x = ((x << 4) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0xC000);
    nlz += (unsigned int)(2 & mask);
    x = ((x << 2) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x8000);
    nlz += (unsigned int)(1 & mask);
    x = ((x << 1) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x8000);
    nlz += (unsigned int)(1 & mask);
#endif
    return nlz;
}

/**
 * trailing zeros counting(constant-time version)
 */
unsigned int mpi_ntz_limb_consttime(mpi_limb_t x)
{
    unsigned int ntz = mpi_is_zero_limb_consttime(x) & 1;

    mpi_limb_t mask;
#if (MPI_LIMB_BITS == 64)
    mask = mpi_is_zero_limb_consttime(x & 0x00000000FFFFFFFF);
    ntz += (unsigned int)(32 & mask);
    x = ((x >> 32) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x000000000000FFFF);
    ntz += (unsigned int)(16 & mask);
    x = ((x >> 16) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x00000000000000FF);
    ntz += (unsigned int)(8 & mask);
    x = ((x >> 8) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x000000000000000F);
    ntz += (unsigned int)(4 & mask);
    x = ((x >> 4) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x0000000000000003);
    ntz += (unsigned int)(2 & mask);
    x = ((x >> 2) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x0000000000000001);
    ntz += (unsigned int)(1 & mask);
#elif (MPI_LIMB_BITS == 32)
    mask = mpi_is_zero_limb_consttime(x & 0x0000FFFF);
    ntz += (unsigned int)(16 & mask);
    x = ((x >> 16) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x000000FF);
    ntz += (unsigned int)(8 & mask);
    x = ((x >> 8) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x0000000F);
    ntz += (unsigned int)(4 & mask);
    x = ((x >> 4) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x00000003);
    ntz += (unsigned int)(2 & mask);
    x = ((x >> 2) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x00000001);
    ntz += (unsigned int)(1 & mask);
#elif (MPI_LIMB_BITS == 16)
    mask = mpi_is_zero_limb_consttime(x & 0x00FF);
    ntz += (unsigned int)(8 & mask);
    x = ((x >> 8) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x000F);
    ntz += (unsigned int)(4 & mask);
    x = ((x >> 4) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x0003);
    ntz += (unsigned int)(2 & mask);
    x = ((x >> 2) & mask) | (x & ~mask);

    mask = mpi_is_zero_limb_consttime(x & 0x0001);
    ntz += (unsigned int)(1 & mask);
#else
#error Unkown target cpu.
#endif

    return ntz;
}

/**
 * mpi(binary): get bit size of mpi
 */
unsigned int mpi_bits_bin(const mpi_limb_t *data, unsigned int size)
{
    if (data == NULL || size == 0) { return 0; }
    unsigned int nlz;
    COUNT_LEADING_ZEROS_BUILTIN(nlz, data[size - 1]);

    return (size * MPI_LIMB_BITS - nlz);
}

/**
 * mpi(binary): get bit size of mpi(constant-time version)
 */
unsigned int mpi_bits_consttime_bin(const mpi_limb_t *data, unsigned int size)
{
    if (data == NULL || size == 0) { return 0; }
    return (size * MPI_LIMB_BITS - mpi_nlz_limb_consttime(data[size - 1]));
}

/**
 * mpi(binary): get actual size of mpi chunk
 */
unsigned int mpi_fix_size_bin(const mpi_limb_t *data, unsigned int size)
{
    MPI_ASSERT(data != NULL);
    unsigned int fixedsz = size;
    if (fixedsz > 0) {
        for (const mpi_limb_t *d = &(data[fixedsz - 1]); fixedsz > 0; fixedsz--) {
            if (*d-- != 0) break;
        }
    }

    return fixedsz;
}

/**
 * mpi(binary): get actual size of mpi chunk(constant-time version)
 */
unsigned int mpi_fix_size_consttime_bin(const mpi_limb_t *data, unsigned int size)
{
    MPI_ASSERT(data != NULL);
    unsigned int fixedsz = size;
    mpi_limb_t zscan = (mpi_limb_t)(-1);
    for (; size > 0; size--) {
        zscan &= mpi_is_zero_limb_consttime(data[size - 1]);
        fixedsz -= 1 & zscan;
    }
    return (unsigned int)((1 & zscan) | ((mpi_limb_t)fixedsz & ~zscan));
}

/**
 * mpi(binary): unsigned comparison
 *
 * @note:
 *   1. return 1 if a[] > b[]; 0 if a[] = b[]; -1 if a[] < b[]
 */
int mpi_ucmp_bin(const mpi_limb_t *a, unsigned int asize, const mpi_limb_t *b, unsigned int bsize)
{
    MPI_ASSERT(a != NULL && b != NULL);
    if (asize == bsize) {
        unsigned int len = asize;
        while (len-- > 0) {
            if (a[len] != b[len]) { return a[len] > b[len] ? 1 : -1; }
        }
        return 0;
    } else {
        return asize > bsize ? 1 : -1;
    }
}

#ifndef MPI_UADD_SCHOOL_ASM
/**
 * mpi(binary): carry, r[:n] = a[:n] + b[:n]
 */
mpi_limb_t mpi_uadd_school_bin(mpi_limb_t *r, const mpi_limb_t *a, const mpi_limb_t *b, unsigned int n)
{
    mpi_limb_t carry = 0;
    for (unsigned int i = 0; i < n; i++) { UADD_ABC(carry, r[i], a[i], b[i], carry); }

    return carry;
}
#endif

/**
 * mpi(binary): carry, r[] = a[] + b[]
 */
mpi_limb_t mpi_uadd_bin(mpi_limb_t *r, unsigned int rroom, const mpi_limb_t *a, unsigned int asize, const mpi_limb_t *b, unsigned int bsize)
{
    MPI_ASSERT(r != NULL && a != NULL && b != NULL);
    MPI_ASSERT(asize >= bsize && rroom >= asize);

    mpi_limb_t *rr = r;
    const mpi_limb_t *aa = a, *bb = b;

    mpi_limb_t carry = mpi_uadd_school_bin(rr, aa, bb, bsize);
    rr += bsize;
    aa += bsize;

    unsigned int diff = asize - bsize;
    while (diff > 0) {
        UADD_AB(carry, *rr, *aa, carry);
        aa++;
        rr++;
        diff--;
    }
    ZEROIZE(r, asize, rroom);

    if (carry != 0 && rroom > asize) { *rr = carry; }

    return carry;
}

#ifndef MPI_UINC_SCHOOL_ASM
/**
 * mpi(binary): carry, r[:n] = a[:n] + w
 */
mpi_limb_t mpi_uinc_school_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int size, mpi_limb_t w)
{
    mpi_limb_t carry = w;

    unsigned int i;
    for (i = 0; i < size && carry != 0; i++) { UADD_AB(carry, r[i], a[i], carry); }
    if (r != a) {
        for (; i < size; i++) { r[i] = a[i]; }
    }

    return carry;
}
#endif

/**
 * mpi(binary): carry, r[] = a[] + w
 */
mpi_limb_t mpi_uinc_bin(mpi_limb_t *r, unsigned int rroom, const mpi_limb_t *a, unsigned int asize, mpi_limb_t w)
{
    MPI_ASSERT(r != NULL && a != NULL);
    MPI_ASSERT(rroom >= asize);

    mpi_limb_t carry = mpi_uinc_school_bin(r, a, asize, w);
    if (carry != 0 && rroom > asize) { r[asize] = carry; }

    return carry;
}

#ifndef MPI_USUB_SCHOOL_ASM
/**
 * mpi(binary) subtraction: borrow, r[:n] = a[:n] - b[:n]
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: MAX(bit_size(a), bit_size(b)) + 1
 */
mpi_limb_t mpi_usub_school_bin(mpi_limb_t *r, const mpi_limb_t *a, const mpi_limb_t *b, unsigned int n)
{
    mpi_limb_t borrow = 0;
    for (unsigned int i = 0; i < n; i++) { USUB_ABC(borrow, r[i], a[i], b[i], borrow); }

    return borrow;
}
#endif

/**
 * mpi(binary) subtraction: size, r[] = a[] - b[]
 */
unsigned int mpi_usub_bin(mpi_limb_t *r, unsigned int rroom, const mpi_limb_t *a, unsigned int asize, const mpi_limb_t *b, unsigned int bsize)
{
    MPI_ASSERT(r != NULL && a != NULL && b != NULL);
    MPI_ASSERT(asize >= bsize && asize <= rroom);

    mpi_limb_t *rr = r;
    const mpi_limb_t *aa = a, *bb = b;

    mpi_limb_t borrow = mpi_usub_school_bin(rr, aa, bb, bsize);
    rr += bsize;
    aa += bsize;
    bb += bsize;

    unsigned int diff = asize - bsize;
    while (diff > 0) {
        mpi_limb_t t = *aa++;
        *rr++ = t - (borrow);
        borrow = t < borrow;
        diff--;
    }

    unsigned int maxsz = asize;
    while (maxsz != 0 && *--rr == 0) { maxsz--; }
    return mpi_fix_size_bin(r, maxsz);
}

#ifndef MPI_UDEC_SCHOOL_ASM
/**
 * mpi(binary): borrow, r[:n] = a[:n] - w
 */
mpi_limb_t mpi_udec_school_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int asize, mpi_limb_t w)
{
    unsigned int i;
    mpi_limb_t borrow = w;
    for (i = 0; i < asize && borrow; i++) { USUB_AB(borrow, r[i], a[i], borrow); }
    if (r != a) {
        for (; i < asize; i++) { r[i] = a[i]; }
    }
    return borrow;
}
#endif

/**
 * mpi(binary): size, r[] = a[] - w
 */
unsigned int mpi_udec_bin(mpi_limb_t *r, unsigned int rroom, const mpi_limb_t *a, unsigned int asize, mpi_limb_t w)
{
    MPI_ASSERT(r != NULL && a != NULL);
    MPI_ASSERT(rroom >= asize);
    MPI_ASSERT(asize > 1 || a[0] >= w);

    unsigned int size = asize;
    mpi_limb_t borrow = mpi_udec_school_bin(r, a, size, w);

    if (borrow != 0 && size < rroom) { r[size++] = 0 - borrow; }
    return size;
}

#ifndef MPI_UMUL_ASM
/**
 * mpi(binary) multiplication: extension, r[:asize+bsize] = a[:asize] * b[:bsize]
 * @note:
 *   1. (IMPORTANT)make sure size of |r| isn't less than |asize| + |bsize|
 *   2. the return is the highest unit |mpi_limb_t|
 */
mpi_limb_t mpi_umul_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int asize, const mpi_limb_t *b, unsigned int bsize)
{
    mpi_limb_t extension = 0;
    ZEROIZE(r, 0, asize + bsize);

    for (unsigned int i = 0; i < bsize; i++) {
        mpi_limb_t t = b[i];

        unsigned int j;
        for (j = 0, extension = 0; j < asize; j++) {
            mpi_limb_t rH, rL;

            UMUL_AB(rH, rL, a[j], t);
            UADD_ABC(extension, r[i + j], r[i + j], rL, extension);
            extension += rH;
        }
        r[i + j] = extension;
    }

    return extension;
}
#endif

#ifndef MPI_UMUL_ADD_ASM
/**
 * mpi(binary) multiply-and-add: extension, r[] += a[] * b
 * @note:
 *   1. (IMPORTANT)make sure size of |r| isn't less than |asize|
 *   2. the return is extension of result of multiply-and-add.
 */
mpi_limb_t mpi_umul_acc_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int asize, mpi_limb_t b)
{
    mpi_limb_t extension = 0;
    for (unsigned int i = 0; i < asize; i++) {
        mpi_limb_t rH, rL;

        UMUL_AB(rH, rL, a[i], b);
        UADD_ABC(extension, r[i], r[i], rL, extension);
        extension += rH;
    }
    return extension;
}
#endif

#ifndef MPI_USQR_ASM
/**
 * mpi(binary) square: r[] = a[] ^ 2
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: 2 * bit_size(a)
 */
mpi_limb_t mpi_usqr_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int asize)
{
    unsigned int i;
    mpi_limb_t extension;
    mpi_limb_t rH, rL;

    /* init result */
    r[0] = 0;
    for (i = 1, extension = 0; i < asize; i++) {
        UMUL_AB(rH, rL, a[i], a[0]);
        UADD_AB(extension, r[i], rL, extension);
        extension += rH;
    }
    r[i] = extension;

    /* add other a[i] * a[j] */
    for (i = 1; i < asize - 1; i++) {
        mpi_limb_t t = a[i];
        unsigned int j;
        for (j = i + 1, extension = 0; j < asize; j++) {
            UMUL_AB(rH, rL, a[j], t);
            UADD_ABC(extension, r[i + j], rL, r[i + j], extension);
            extension += rH;
        }
        r[i + j] = extension;
    }

    /* double a[i] * a[j] */
    for (i = 1, extension = 0; i < (2 * asize - 1); i++) { UADD_ABC(extension, r[i], r[i], r[i], extension); }
    r[i] = extension;

    /* add a[i]^2 */
    for (i = 0, extension = 0; i < asize; i++) {
        UMUL_AB(rH, rL, a[i], a[i]);
        UADD_ABC(extension, r[2 * i], r[2 * i], rL, extension);
        UADD_ABC(extension, r[2 * i + 1], r[2 * i + 1], rH, extension);
    }
    return r[2 * asize - 1];
}
#endif

/**
 * mpi(binary): left shift
 *
 * @note:
 *   1. required bit_size(r) >= bit_size(a) + nbits
 *   2. the return is number of |mpi_limb_t| of the result |r|
 *   3. r == a is acceptable
 */
unsigned int mpi_lshift_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int asize, unsigned int nbits)
{
    MPI_ASSERT(r != NULL && a != NULL);

    unsigned int nw = nbits / MPI_LIMB_BITS; // shift words
    unsigned int nb = nbits % MPI_LIMB_BITS; // shift bits

    unsigned int rsize = asize + nw;
    if (nb != 0) {
        mpi_limb_t hi = a[asize - 1], lo;
        lo = hi >> (MPI_LIMB_BITS - nb);
        if (lo != 0) { r[rsize++] = lo; }

        for (unsigned int i = asize - 1; i > 0; i--) {
            lo = a[i - 1];
            r[i + nw] = (hi << nb) | (lo >> (MPI_LIMB_BITS - nb));
            hi = lo;
        }
        r[nw] = a[0] << nb;
    } else {
        mpi_limb_t *rr = &r[nw];
        for (unsigned int i = asize; i > 0; i--) { rr[i - 1] = a[i - 1]; }
    }
    if (nw != 0) { ZEROIZE(r, 0, nw); }

    return rsize;
}

/**
 * mpi(binary): right shift
 *
 * @note:
 *   1. required bit_size(r) >= bit_size(a) - nbits
 *   2. the return is number of |mpi_limb_t| of the result |r|
 *   3. r == a is acceptable
 */
unsigned int mpi_rshift_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int asize, unsigned int nbits)
{
    MPI_ASSERT(r != NULL && a != NULL);
    MPI_ASSERT(asize > nbits / MPI_LIMB_BITS);

    unsigned int nw = nbits / MPI_LIMB_BITS; // shift words
    unsigned int nb = nbits % MPI_LIMB_BITS; // shift bits

    unsigned int rsize = asize - nw;
    if (nb > 0) {
        mpi_limb_t hi, lo = a[nw];
        for (unsigned int n = nw; n < asize - 1; n++) {
            hi = a[n + 1];
            r[n - nw] = (lo >> nb) | (hi << (MPI_LIMB_BITS - nb));
            lo = hi;
        }
        lo >>= nb;
        if (lo != 0) {
            r[rsize - 1] = lo;
        } else {
            rsize--;
        }
    } else {
        COPY(r, &a[nw], rsize);
    }

    return rsize;
}

#ifndef MPI_UDIV_ASM
/**
 * borrow, r[] = r[] - a[] * m
 *
 * rsize >= asize
 */
static mpi_limb_t mpi_usub_mul_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int asize, mpi_limb_t m)
{
    mpi_limb_t borrow = 0;
    for (unsigned int i = 0; i < asize; i++) {
        mpi_limb_t aH, aL;
        UMUL_AB(aH, aL, a[i], m);                 // aH, aL = a[i] * m
        USUB_ABC(borrow, r[i], r[i], aL, borrow); // borrow, r[i] = r[i] - aL  - borrow
        borrow += aH;                             // WILL NEVER overflow
    }

    return borrow;
}
#endif

/**
 * mpi(binary) division: xsize, q, x(q = x / y, x = x % y)
 */
unsigned int mpi_udiv_bin(mpi_limb_t *q, unsigned int *qsize, mpi_limb_t *x, unsigned int xsize, mpi_limb_t *y, unsigned int ysize)
{
#if (defined MPI_UDIV_ASM) && (MPI_LIMB_BITS == 64)
    /**
     * mpi(bin, half) division: xsize, q, x(q = x / y, x = x % y)
     */
    unsigned int mpi_udiv_bin_u32(uint32_t * q, unsigned int *qsize, uint32_t *x, unsigned int xsize, uint32_t *y, unsigned int ysize);

    unsigned int size = mpi_udiv_bin_u32((uint32_t *)q, qsize, (uint32_t *)x, xsize * 2, (uint32_t *)y, ysize * 2);
    if (size & 1) ((uint32_t *)x)[size] = 0;
    size = (unsigned int)((size + 2 - 1) / 2);
    if (q != NULL && qsize != NULL) {
        if (*qsize & 1) ((uint32_t *)q)[*qsize] = 0;
        *qsize = (unsigned int)((*qsize + 2 - 1) / 2);
    }

    return size;
#else
    if (x == NULL || y == NULL || ysize == 0) {
        MPI_RAISE_ERROR(-EINVAL);
        MPI_RAISE_EXCEPTION();
        return 0;
    }
    if (xsize < ysize) {
        if (q != NULL && qsize != NULL) {
            ZEROIZE(q, 0, *qsize);
            *qsize = 0;
        }
        return xsize;
    }
    if (q != NULL && qsize != NULL) {
        if (*qsize < xsize - ysize + 1) {
            MPI_RAISE_ERROR(-ERANGE);
            return 0;
        }
    }

    /* special case */
    if (ysize == 1) {
        unsigned int qsz = mpi_udiv_limb_bin(q, x, xsize, &x[0], y[0]);
        if (q != NULL && qsize != NULL) { *qsize = qsz; }

        return 1;
    }

    /* common case */
    {
        unsigned int qsz = xsize - ysize + 1;
        unsigned int shifts = mpi_nlz_limb_consttime(y[ysize - 1]);

        /* normalization */
        x[xsize] = 0;
        if (shifts != 0) {
            mpi_lshift_bin(x, x, xsize, shifts);
            mpi_lshift_bin(y, y, ysize, shifts);
        }

        /* zeroize */
        if (q != NULL && qsize != NULL) { ZEROIZE(q, 0, qsz); }

        /* division */
        {
            mpi_limb_t yy = y[ysize - 1];
            for (unsigned int i = xsize; i >= ysize; i--) {
                /* estimate digit of quotient */
                mpi_limb_t __q, __r = 0;

                MPI_ASSERT(x[i] <= yy); // WILL NEVER happen
                if (x[i] == yy) {
                    __q = MPI_LIMB_MASK;
                } else {
                    UDIV_NND(__q, __r, x[i], x[i - 1], yy);

                    /* tune estimation above */
                    {
                        mpi_limb_t rH, rL, xxx = x[xsize - 3], yyy = y[ysize - 2];
                        UMUL_AB(rH, rL, __q, y[ysize - 2]);
                        for (;;) {
                            /**
                             * break condition:
                             *   q[i - t -1] * (y[t] * b + y[t - 1]) > x[i] * b ^ 2 + x[i - 1] * b + x[i - 2]
                             *
                             * and we kown that:
                             *   x[i] * b + x[i - 1] = q * y[t] + r
                             *
                             * so the break condition can be presented as:
                             *   q * y[t - 1] > r * b + x[i - 2]
                             */
                            if (rH < __r || (rH == __r && rL <= xxx)) { break; }
                            __q--;
                            __r += yy;

                            if (__r < yy) { break; }
                            if (rL < yyy) { rH--; }
                            rL -= yyy;
                        }
                    }
                }

                if (__q > 0) {
                    mpi_limb_t extend = mpi_usub_mul_bin(&x[i - ysize], y, ysize, __q);
                    extend = (x[i] -= extend);
                    if (extend > 0) { /* subtracted too much */
                        extend = mpi_uadd_school_bin(&x[i - ysize], &x[i - ysize], y, ysize);
                        x[i] += extend;
                        __q--;
                    }
                }

                /* store quotation digit */
                if (q != NULL && qsize != NULL) { q[i - ysize] = __q; }
            }
        }

        /* de-normalization */
        if (shifts != 0) {
            mpi_rshift_bin(x, x, xsize, shifts);
            mpi_rshift_bin(y, y, ysize, shifts);
        }
        xsize = mpi_fix_size_bin(x, xsize);

        if (q != NULL && qsize != NULL) { *qsize = mpi_fix_size_bin(q, qsz); }

        return xsize;
    }
#endif
}

/**
 * mpi(binary) modular: x[] = x[] % y[]
 */
unsigned int mpi_umod_bin(mpi_limb_t *x, unsigned int xsize, mpi_limb_t *y, unsigned int ysize)
{
    return mpi_udiv_bin(NULL, NULL, x, xsize, y, ysize);
}

/**
 * mpi(binary): division(n by 1)
 *
 * @note:
 *   1. required length of q should be not smaller than size
 */
unsigned int mpi_udiv_limb_bin(mpi_limb_t q[], const mpi_limb_t x[], unsigned int size, mpi_limb_t *r, mpi_limb_t d)
{
    mpi_limb_t __r = 0, __q;
    for (unsigned int i = size; i > 0; i--) {
        UDIV_NND(__q, __r, __r, x[i - 1], d);
        if (q != NULL) { q[i - 1] = __q; }
    }
    if (r != NULL) { r[0] = __r; }

    return mpi_fix_size_bin(q, size);
}

/**
 * greatest common divisor(mpi_limb_t)
 */
mpi_limb_t mpi_gcd_limb(mpi_limb_t a, mpi_limb_t b)
{
    mpi_limb_t g = 1;

    if (a < b) {
        mpi_limb_t t = a;
        a = b;
        b = t;
    }
    while ((a & 0x1) == 0 && (b & 0x1) == 0) {
        a >>= 1;
        b >>= 1;
        g <<= 1;
    }
    while (a != 0) {
        while ((a & 0x1) == 0) { a >>= 1; }
        while ((b & 0x1) == 0) { b >>= 1; }
        if (a >= b) {
            a = (a - b) >> 1;
        } else {
            b = (b - a) >> 1;
        }
    }

    return g * b;
}

/**
 * @brief:
 *   multiply with accumulation, computes r <- r + a * b
 *
 * @return:
 *   1. real size of the r
 *   2. 0 if there are no enought buffer size to write to r[MAX(size_r + 1, size_a + size_b) - 1]
 *
 * @note:
 *   1. DO NOT run in inplace mode
 *   2. The minimum buffer size for the r must be (size_a + size_b - 1)
 *   3. The maximum buffer size for the r is MAX(size_r + 1, size_a + size_b)
 */
static unsigned int umul_acc_bin(mpi_limb_t *r, unsigned int rsize, const mpi_limb_t *a, unsigned int asize, const mpi_limb_t *b, unsigned int bsize)
{
    unsigned int tsize = asize + bsize;
    if (tsize > rsize) {
        ZEROIZE(r, rsize, tsize - 1); /* cleanup the rest of destination buffer */
        rsize = tsize;
    }

    {
        mpi_limb_t expansion = 0;
        for (unsigned int i = 0; i < bsize && !expansion; i++) {
            expansion = mpi_umul_acc_bin(r + i, a, asize, b[i]);
            if (expansion) { expansion = mpi_uinc_school_bin(r + i + asize, r + i + asize, rsize - i - asize, expansion); }
        }

        if (expansion) {
            return 0; // no enought buffer size to write
        } else {
            return mpi_fix_size_bin(r, rsize);
        }
    }
}

/**
 * @brief: multiplicative inversion
 *
 * @params:
 *   a/asize: source (value) BigNum A whose size is asize
 *   m/msize: source (modulus) BigNum M whose size is msize
 *   invbuf: buffer of inv
 *   abuff  : buffer of A
 *   mbuf  : buffer of M
 *   r  : result BigNum
 */
unsigned int mpi_umod_inv_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int asize, const mpi_limb_t *m, unsigned int msize, mpi_optimizer_t *optimizer)
{
    MPI_ASSERT(optimizer != NULL);
    MPI_ASSERT(asize == mpi_fix_size_bin(a, asize));
    MPI_ASSERT(msize == mpi_fix_size_bin(m, msize));

    /* special cases */
    {
        if (asize == 1 && a[0] == 1) { /* inv(1) = 1 */
            r[0] = 1;
            return 1;
        }
    }

    /* common cases */
    {
        unsigned int size = 0;
        mpi_limb_t *mtemp = mpi_optimizer_get_limbs(optimizer, msize);
        mpi_limb_t *abuff = mpi_optimizer_get_limbs(optimizer, asize);
        mpi_limb_t *mbuff = mpi_optimizer_get_limbs(optimizer, msize);
        mpi_limb_t *invbuf = mpi_optimizer_get_limbs(optimizer, msize);
        MPI_ASSERT(mtemp != NULL && abuff != NULL && mbuff != NULL && invbuf != NULL);

        COPY(mtemp, m, msize);

        unsigned int modsize = msize;
        mpi_limb_t *X1 = r, *X2 = mbuff, *Q = invbuf;
        unsigned int x1size = 1, x2size = 1, qsize;

        COPY(abuff, a, asize);
        ZEROIZE(X1, 0, modsize);
        ZEROIZE(X2, 0, modsize);
        X2[0] = 1;

        for (;;) {
            msize = mpi_udiv_bin(Q, &qsize, mtemp, msize, abuff, asize);
            x1size = umul_acc_bin(X1, modsize, Q, qsize, X2, x2size);

            if (msize == 1 && m[0] == 1) {
                // ZEROIZE(X2, x2size, modsize);
                x2size = umul_acc_bin(X2, modsize, X1, x1size, abuff, asize);
                COPY(mtemp, X2, modsize);
                mpi_usub_school_bin(r, m, X1, modsize);

                size = mpi_fix_size_bin(r, modsize);

                goto operation_end;
            } else if (msize == 1 && m[0] == 0) {
                mpi_umul_bin(mtemp, X1, x1size, abuff, asize);
                size = 0; /* gcd = a */

                goto operation_end;
            }

            asize = mpi_udiv_bin(Q, &qsize, abuff, asize, mtemp, msize);
            x2size = umul_acc_bin(X2, modsize, Q, qsize, X1, x1size);

            if (asize == 1 && abuff[0] == 1) {
                ZEROIZE(X1, x1size, modsize);
                x1size = umul_acc_bin(X1, modsize, X2, x2size, m, msize);
                COPY(mtemp, X1, modsize);
                COPY(r, X2, x2size);
                size = x2size;

                goto operation_end;
            } else if (asize == 1 && abuff[0] == 0) {
                /* gcd = m */
                COPY(X1, m, msize);
                mpi_umul_bin(mtemp, X2, x2size, X1, msize);

                size = 0;

                goto operation_end;
            }
        }

    operation_end:
        mpi_optimizer_put_limbs(optimizer, msize);
        mpi_optimizer_put_limbs(optimizer, msize);
        mpi_optimizer_put_limbs(optimizer, asize);
        mpi_optimizer_put_limbs(optimizer, msize);

        return size;
    }
}

/**
 * mpi(binary): ALL-ones if buff[::] is zero, otherwise ALL-zeros
 */
mpi_limb_t mpi_is_zero_bin(const mpi_limb_t *buff, unsigned int bufflen)
{
    for (unsigned int i = 0; i < bufflen; i++) {
        if (buff[i] != 0) { return 0; }
    }
    return MPI_LIMB_MASK;
}

/**
 * mpi(binary): ALL-ones if buff[::] is zero, otherwise ALL-zeros(constant-time version)
 */
mpi_limb_t mpi_is_zero_consttime_bin(const mpi_limb_t *buff, unsigned int bufflen)
{
    mpi_limb_t diff = 0;
    for (unsigned int i = 0; i < bufflen; i++) { diff |= buff[i]; }
    return mpi_is_zero_limb_consttime(diff);
}

/**
 * mpi(binary): generate in range (lo[] < r < hi[])
 *
 * @note:
 *   1. length of |r| >= hilen
 */
int mpi_random_range_bin(mpi_limb_t *r, unsigned int maxtries, const mpi_limb_t *lo, unsigned int lolen, const mpi_limb_t *hi, unsigned int hilen, int (*rand_bytes)(void *, unsigned char *, unsigned int), void *rand_state)
{
    mpi_limb_t topmask = MPI_MASK_LIMB_HI(mpi_bits_consttime_bin(hi, hilen));

    for (unsigned int i = 0; i < maxtries; i++) {
        int err;
        if ((err = rand_bytes(rand_state, (unsigned char *)r, hilen * MPI_LIMB_BYTES)) != 0) {
            MPI_RAISE_ERROR(err, "generation failed, iterations = %u, err = %d", i, err);
            return err;
        }
        r[hilen - 1] &= topmask;
        unsigned int randlen = mpi_fix_size_bin(r, hilen);

        if (mpi_ucmp_bin(r, randlen, lo, lolen) > 0 && mpi_ucmp_bin(r, randlen, hi, hilen) < 0) { return 0; }
    }

    return -EBUSY;
}

/**
 *  mpi(binary): create mpi from big-endian octets
 */
unsigned int mpi_from_octets_bin(mpi_limb_t *r, unsigned int size, const unsigned char *in, unsigned int inlen)
{
    unsigned int h = 0;
    unsigned int left = inlen;
    while (left > 0) {
        mpi_limb_t l = 0;
        unsigned m = (MPI_LIMB_BYTES <= left) ? MPI_LIMB_BYTES : left;
        for (unsigned i = m; i > 0; i--) { // big-endian
            l = (l << BITS_PER_BYTE) | in[left - i];
        }
        MPI_ASSERT(h < size);
        r[h++] = l;
        left -= m;
    }

    return mpi_fix_size_bin(r, h);
}

/**
 *  mpi(binary): convert mpi to big-endian octets
 */
unsigned int mpi_to_octets_bin(unsigned char *out, unsigned int outsize, const mpi_limb_t *a, unsigned int size)
{
    MPI_ASSERT(out != NULL);
    MPI_ASSERT(a != NULL && size != 0);

    unsigned int reqsize = MPI_BITS_TO_BYTES(mpi_bits_consttime_bin(a, size));
    MPI_ASSERT(outsize >= reqsize);

    unsigned char *p = out;
    unsigned int left = reqsize;
    const mpi_limb_t *w = &a[size - 1];
    if ((left & (MPI_LIMB_BYTES - 1)) != 0) {
        unsigned char buff[MPI_LIMB_BYTES];
        MPI_LIMB_TO_OCTETS(buff, *w);
        unsigned int n = left & (MPI_LIMB_BYTES - 1);
        memcpy(p, &buff[MPI_LIMB_BYTES - n], n);
        w--;
        p += n;
        left -= n;
    }
    while (left >= MPI_LIMB_BYTES) {
        MPI_LIMB_TO_OCTETS(p, *w);
        left -= MPI_LIMB_BYTES;
        p += MPI_LIMB_BYTES;
        w--;
    }

    return reqsize;
}

/**
 *  mpi(binary): create mpi from hex string
 */
unsigned int mpi_from_string_bin(mpi_limb_t *r, unsigned int size, const char *in, unsigned int inlen)
{
    unsigned int h = 0;
    while (inlen > 0) {
        mpi_limb_t l = 0;
        unsigned int m = (MPI_LIMB_BYTES * 2 <= inlen) ? (MPI_LIMB_BYTES * 2) : inlen;
        for (unsigned int i = m; i > 0; i--) { // big-endian
            char c = in[inlen - i];
            l = (l << BITS_PER_CHAR) | ((c > '9' ? c + 9 : c) & 0x0F /* to hex-char if isxdigit(c) which checked before */);
        }
        MPI_ASSERT(h < size);
        r[h++] = l;
        inlen -= m;
    }
    return mpi_fix_size_bin(r, h);
}

/**
 *  mpi(binary): convert mpi to hex string
 */
unsigned int mpi_to_string_bin(char *out, unsigned int outsize, const mpi_limb_t *a, unsigned int size)
{
    if (size == 0) {
        MPI_ASSERT(outsize >= 2);
        strcpy(out, "0");
        return 1;
    }

    char *p = out;
    unsigned notfirst = 0; /* to strip leading zeros */
    static const char HEX_CHARS[] = "0123456789ABCDEF";

    for (unsigned int i = size; i > 0; i--) {
        mpi_limb_t vv = a[i - 1];
        for (int j = MPI_LIMB_BITS - BITS_PER_BYTE; j >= 0; j -= BITS_PER_BYTE) {
            unsigned char c = (unsigned char)((vv >> j) & 0xff);
            if (notfirst || c != 0) {
                if (notfirst || c & 0xf0) { *p++ = HEX_CHARS[(c >> BITS_PER_CHAR)]; }
                *p++ = HEX_CHARS[c & 0x0f];
                notfirst = 1;
            }
        }
    }
    *p = '\0';

    return (unsigned int)(p - out);
}
