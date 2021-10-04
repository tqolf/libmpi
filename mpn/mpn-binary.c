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

/**
 * leading zeros counting(constant-time version)
 */
unsigned int mpn_limb_nlz_consttime(mpn_limb_t x)
{
    unsigned int nlz = 0;
    mpn_limb_t mask;
#if (MPN_LIMB_BITS == 64)
    mask = mpn_limb_is_zero_consttime(x & 0xFFFFFFFF00000000);
    nlz += (unsigned int)(32 & mask);
    x = ((x << 32) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xFFFF000000000000);
    nlz += (unsigned int)(16 & mask);
    x = ((x << 16) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xFF00000000000000);
    nlz += (unsigned int)(8 & mask);
    x = ((x << 8) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xF000000000000000);
    nlz += (unsigned int)(4 & mask);
    x = ((x << 4) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xC000000000000000);
    nlz += (unsigned int)(2 & mask);
    x = ((x << 2) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x8000000000000000);
    nlz += (unsigned int)(1 & mask);
    x = ((x << 1) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x8000000000000000);
    nlz += (unsigned int)(1 & mask);
#elif MPN_LIMB_BITS == 32
    mask = mpn_limb_is_zero_consttime(x & 0xFFFF0000);
    nlz += (unsigned int)(16 & mask);
    x = ((x << 16) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xFF000000);
    nlz += (unsigned int)(8 & mask);
    x = ((x << 8) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xF0000000);
    nlz += (unsigned int)(4 & mask);
    x = ((x << 4) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xC0000000);
    nlz += (unsigned int)(2 & mask);
    x = ((x << 2) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x80000000);
    nlz += (unsigned int)(1 & mask);
    x = ((x << 1) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x80000000);
    nlz += (unsigned int)(1 & mask);
#elif MPN_LIMB_BITS == 16
    mask = mpn_limb_is_zero_consttime(x & 0xFF00);
    nlz += (unsigned int)(8 & mask);
    x = ((x << 8) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xF000);
    nlz += (unsigned int)(4 & mask);
    x = ((x << 4) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xC000);
    nlz += (unsigned int)(2 & mask);
    x = ((x << 2) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x8000);
    nlz += (unsigned int)(1 & mask);
    x = ((x << 1) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x8000);
    nlz += (unsigned int)(1 & mask);
#endif
    return nlz;
}

/**
 * trailing zeros counting(constant-time version)
 */
unsigned int mpn_limb_ntz_consttime(mpn_limb_t x)
{
    unsigned int ntz = mpn_limb_is_zero_consttime(x) & 1;

    mpn_limb_t mask;
#if (MPN_LIMB_BITS == 64)
    mask = mpn_limb_is_zero_consttime(x & 0x00000000FFFFFFFF);
    ntz += (unsigned int)(32 & mask);
    x = ((x >> 32) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x000000000000FFFF);
    ntz += (unsigned int)(16 & mask);
    x = ((x >> 16) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x00000000000000FF);
    ntz += (unsigned int)(8 & mask);
    x = ((x >> 8) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x000000000000000F);
    ntz += (unsigned int)(4 & mask);
    x = ((x >> 4) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x0000000000000003);
    ntz += (unsigned int)(2 & mask);
    x = ((x >> 2) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x0000000000000001);
    ntz += (unsigned int)(1 & mask);
#elif (MPN_LIMB_BITS == 32)
    mask = mpn_limb_is_zero_consttime(x & 0x0000FFFF);
    ntz += (unsigned int)(16 & mask);
    x = ((x >> 16) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x000000FF);
    ntz += (unsigned int)(8 & mask);
    x = ((x >> 8) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x0000000F);
    ntz += (unsigned int)(4 & mask);
    x = ((x >> 4) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x00000003);
    ntz += (unsigned int)(2 & mask);
    x = ((x >> 2) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x00000001);
    ntz += (unsigned int)(1 & mask);
#elif (MPN_LIMB_BITS == 16)
    mask = mpn_limb_is_zero_consttime(x & 0x00FF);
    ntz += (unsigned int)(8 & mask);
    x = ((x >> 8) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x000F);
    ntz += (unsigned int)(4 & mask);
    x = ((x >> 4) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x0003);
    ntz += (unsigned int)(2 & mask);
    x = ((x >> 2) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x0001);
    ntz += (unsigned int)(1 & mask);
#else
#error Unkown target cpu.
#endif

    return ntz;
}

/**
 * mpn: get most significant bit
 */
unsigned int mpn_bits(const mpn_limb_t *data, unsigned int size)
{
    if (data == NULL || size == 0) { return 0; }
    unsigned int nlz;
    COUNT_LEADING_ZEROS_BUILTIN(nlz, data[size - 1]);

    return (size * MPN_LIMB_BITS - nlz);
}

/**
 * mpn: get most significant bit(constant-time version)
 */
unsigned int mpn_bits_consttime(const mpn_limb_t *data, unsigned int size)
{
    if (data == NULL || size == 0) { return 0; }
    return (size * MPN_LIMB_BITS - mpn_limb_nlz_consttime(data[size - 1]));
}

/**
 * mpn: get most significant limb
 */
unsigned int mpn_limbs(const mpn_limb_t *data, unsigned int size)
{
    MPN_ASSERT(data != NULL);
    unsigned int fixedsz = size;
    if (fixedsz > 0) {
        for (const mpn_limb_t *d = &(data[fixedsz - 1]); fixedsz > 0; fixedsz--) {
            if (*d-- != 0) break;
        }
    }

    return fixedsz;
}

/**
 * mpn: get most significant limb(constant-time version)
 */
unsigned int mpn_limbs_consttime(const mpn_limb_t *data, unsigned int size)
{
    MPN_ASSERT(data != NULL);
    unsigned int fixedsz = size;
    mpn_limb_t zscan = (mpn_limb_t)(-1);
    for (; size > 0; size--) {
        zscan &= mpn_limb_is_zero_consttime(data[size - 1]);
        fixedsz -= 1 & zscan;
    }
    return (unsigned int)((1 & zscan) | ((mpn_limb_t)fixedsz & ~zscan));
}

/**
 * mpn: unsigned comparison
 *
 * @note:
 *   1. return 1 if a[] > b[]; 0 if a[] = b[]; -1 if a[] < b[]
 */
int mpn_cmp(const mpn_limb_t *a, unsigned int asize, const mpn_limb_t *b, unsigned int bsize)
{
    MPN_ASSERT(a != NULL && b != NULL);
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

#ifndef MPN_UADD_VECTORIZED_ASM
/**
 * mpn: carry, r[:n] = a[:n] + b[:n]
 */
mpn_limb_t mpn_add_vectorized(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, unsigned int n)
{
    mpn_limb_t carry = 0;
    for (unsigned int i = 0; i < n; i++) { UADD_ABC(carry, r[i], a[i], b[i], carry); }

    return carry;
}
#endif

/**
 * mpn: carry, r[] = a[] + b[]
 */
mpn_limb_t mpn_add(mpn_limb_t *r, unsigned int rroom, const mpn_limb_t *a, unsigned int asize, const mpn_limb_t *b,
                   unsigned int bsize)
{
    MPN_ASSERT(r != NULL && a != NULL && b != NULL);
    MPN_ASSERT(asize >= bsize && rroom >= asize);

    mpn_limb_t *rr = r;
    const mpn_limb_t *aa = a, *bb = b;

    mpn_limb_t carry = mpn_add_vectorized(rr, aa, bb, bsize);
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

#ifndef MPN_UINC_VECTORIZED_ASM
/**
 * mpn: carry, r[:n] = a[:n] + w
 */
mpn_limb_t mpn_inc_vectorized(mpn_limb_t *r, const mpn_limb_t *a, unsigned int size, mpn_limb_t w)
{
    mpn_limb_t carry = w;

    unsigned int i;
    for (i = 0; i < size && carry != 0; i++) { UADD_AB(carry, r[i], a[i], carry); }
    if (r != a) {
        for (; i < size; i++) { r[i] = a[i]; }
    }

    return carry;
}
#endif

/**
 * mpn: carry, r[] = a[] + w
 */
mpn_limb_t mpn_inc(mpn_limb_t *r, unsigned int rroom, const mpn_limb_t *a, unsigned int asize, mpn_limb_t w)
{
    MPN_ASSERT(r != NULL && a != NULL);
    MPN_ASSERT(rroom >= asize);

    mpn_limb_t carry = mpn_inc_vectorized(r, a, asize, w);
    if (carry != 0 && rroom > asize) { r[asize] = carry; }

    return carry;
}

#ifndef MPN_USUB_VECTORIZED_ASM
/**
 * mpn subtraction: borrow, r[:n] = a[:n] - b[:n]
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: MAX(bit_size(a), bit_size(b)) + 1
 */
mpn_limb_t mpn_sub_vectorized(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, unsigned int n)
{
    mpn_limb_t borrow = 0;
    for (unsigned int i = 0; i < n; i++) { USUB_ABC(borrow, r[i], a[i], b[i], borrow); }

    return borrow;
}
#endif

/**
 * mpn subtraction: size, r[] = a[] - b[]
 */
unsigned int mpn_sub(mpn_limb_t *r, unsigned int rroom, const mpn_limb_t *a, unsigned int asize, const mpn_limb_t *b,
                     unsigned int bsize)
{
    MPN_ASSERT(r != NULL && a != NULL && b != NULL);
    MPN_ASSERT(asize >= bsize && asize <= rroom);

    mpn_limb_t *rr = r;
    const mpn_limb_t *aa = a, *bb = b;

    mpn_limb_t borrow = mpn_sub_vectorized(rr, aa, bb, bsize);
    rr += bsize;
    aa += bsize;
    bb += bsize;

    unsigned int diff = asize - bsize;
    while (diff > 0) {
        mpn_limb_t t = *aa++;
        *rr++ = t - (borrow);
        borrow = t < borrow;
        diff--;
    }

    unsigned int maxsz = asize;
    while (maxsz != 0 && *--rr == 0) { maxsz--; }
    return mpn_limbs(r, maxsz);
}

#ifndef MPN_UDEC_VECTORIZED_ASM
/**
 * mpn: borrow, r[:n] = a[:n] - w
 */
mpn_limb_t mpn_dec_vectorized(mpn_limb_t *r, const mpn_limb_t *a, unsigned int asize, mpn_limb_t w)
{
    unsigned int i;
    mpn_limb_t borrow = w;
    for (i = 0; i < asize && borrow; i++) { USUB_AB(borrow, r[i], a[i], borrow); }
    if (r != a) {
        for (; i < asize; i++) { r[i] = a[i]; }
    }
    return borrow;
}
#endif

/**
 * mpn: size, r[] = a[] - w
 */
unsigned int mpn_dec(mpn_limb_t *r, unsigned int rroom, const mpn_limb_t *a, unsigned int asize, mpn_limb_t w)
{
    MPN_ASSERT(r != NULL && a != NULL);
    MPN_ASSERT(rroom >= asize);
    MPN_ASSERT(asize > 1 || a[0] >= w);

    unsigned int size = asize;
    mpn_limb_t borrow = mpn_dec_vectorized(r, a, size, w);

    if (borrow != 0 && size < rroom) { r[size++] = 0 - borrow; }
    return size;
}

#ifndef MPN_UMUL_ASM
/**
 * mpn multiplication: extension, r[:asize+bsize] = a[:asize] * b[:bsize]
 * @note:
 *   1. (IMPORTANT)make sure size of |r| isn't less than |asize| + |bsize|
 *   2. the return is the highest unit |mpn_limb_t|
 */
mpn_limb_t mpn_mul(mpn_limb_t *r, const mpn_limb_t *a, unsigned int asize, const mpn_limb_t *b, unsigned int bsize)
{
    mpn_limb_t extension = 0;
    ZEROIZE(r, 0, asize + bsize);

    for (unsigned int i = 0; i < bsize; i++) {
        mpn_limb_t t = b[i];

        unsigned int j;
        for (j = 0, extension = 0; j < asize; j++) {
            mpn_limb_t rH, rL;

            UMUL_AB(rH, rL, a[j], t);
            UADD_ABC(extension, r[i + j], r[i + j], rL, extension);
            extension += rH;
        }
        r[i + j] = extension;
    }

    return extension;
}
#endif

#ifndef MPN_UMUL_ADD_ASM
/**
 * mpn multiply-and-add: extension, r[] += a[] * b
 * @note:
 *   1. (IMPORTANT)make sure size of |r| isn't less than |asize|
 *   2. the return is extension of result of multiply-and-add.
 */
mpn_limb_t mpn_mul_acc(mpn_limb_t *r, const mpn_limb_t *a, unsigned int asize, mpn_limb_t b)
{
    mpn_limb_t extension = 0;
    for (unsigned int i = 0; i < asize; i++) {
        mpn_limb_t rH, rL;

        UMUL_AB(rH, rL, a[i], b);
        UADD_ABC(extension, r[i], r[i], rL, extension);
        extension += rH;
    }
    return extension;
}
#endif

#ifndef MPN_USQR_ASM
/**
 * mpn square: r[] = a[] ^ 2
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: 2 * bit_size(a)
 */
mpn_limb_t mpn_sqr(mpn_limb_t *r, const mpn_limb_t *a, unsigned int asize)
{
    unsigned int i;
    mpn_limb_t extension;
    mpn_limb_t rH, rL;

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
        mpn_limb_t t = a[i];
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
 * mpn: left shift
 *
 * @note:
 *   1. required bit_size(r) >= bit_size(a) + nbits
 *   2. the return is number of |mpn_limb_t| of the result |r|
 *   3. r == a is acceptable
 */
unsigned int mpn_lshift(mpn_limb_t *r, const mpn_limb_t *a, unsigned int asize, unsigned int nbits)
{
    MPN_ASSERT(r != NULL && a != NULL);

    unsigned int nw = nbits / MPN_LIMB_BITS; // shift words
    unsigned int nb = nbits % MPN_LIMB_BITS; // shift bits

    unsigned int rsize = asize + nw;
    if (LIKELY(nb > 0)) {
        mpn_limb_t hi = a[asize - 1], lo;
        lo = hi >> (MPN_LIMB_BITS - nb);
        if (lo != 0) { r[rsize++] = lo; }

        for (unsigned int i = asize - 1; i > 0; i--) {
            lo = a[i - 1];
            r[i + nw] = (hi << nb) | (lo >> (MPN_LIMB_BITS - nb));
            hi = lo;
        }
        r[nw] = a[0] << nb;
    } else {
        mpn_limb_t *rr = &r[nw];
        for (unsigned int i = asize; i > 0; i--) { rr[i - 1] = a[i - 1]; }
    }
    if (nw != 0) { ZEROIZE(r, 0, nw); }

    return rsize;
}

/**
 * mpn: right shift
 *
 * @note:
 *   1. required bit_size(r) >= bit_size(a) - nbits
 *   2. the return is number of |mpn_limb_t| of the result |r|
 *   3. r == a is acceptable
 */
unsigned int mpn_rshift(mpn_limb_t *r, const mpn_limb_t *a, unsigned int asize, unsigned int nbits)
{
    MPN_ASSERT(r != NULL && a != NULL);
    MPN_ASSERT(asize > nbits / MPN_LIMB_BITS);

    unsigned int nw = nbits / MPN_LIMB_BITS; // shift words
    unsigned int nb = nbits % MPN_LIMB_BITS; // shift bits

    unsigned int rsize = asize - nw;
    if (LIKELY(nb > 0)) {
        mpn_limb_t hi, lo = a[nw];
        for (unsigned int n = nw; n < asize - 1; n++) {
            hi = a[n + 1];
            r[n - nw] = (lo >> nb) | (hi << (MPN_LIMB_BITS - nb));
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

#ifndef MPN_UDIV_ASM
/**
 * borrow, r[] = r[] - a[] * m
 *
 * rsize >= asize
 */
static mpn_limb_t mpn_sub_mul(mpn_limb_t *r, const mpn_limb_t *a, unsigned int asize, mpn_limb_t m)
{
    mpn_limb_t borrow = 0;
    for (unsigned int i = 0; i < asize; i++) {
        mpn_limb_t aH, aL;
        UMUL_AB(aH, aL, a[i], m);                 // aH, aL = a[i] * m
        USUB_ABC(borrow, r[i], r[i], aL, borrow); // borrow, r[i] = r[i] - aL  - borrow
        borrow += aH;                             // WILL NEVER overflow
    }

    return borrow;
}
#endif


/**
 * mpn: division(n by 1)
 *
 * @note:
 *   1. required length of q should be not smaller than size
 */
unsigned int mpn_div_limb(mpn_limb_t q[], const mpn_limb_t x[], unsigned int size, mpn_limb_t *r, mpn_limb_t d)
{
    mpn_limb_t __r = 0, __q;
    for (unsigned int i = size; i > 0; i--) {
        UDIV_NND(__q, __r, __r, x[i - 1], d);
        if (q != NULL) { q[i - 1] = __q; }
    }
    if (r != NULL) { r[0] = __r; }

    return mpn_limbs(q, size);
}

#ifndef DIV_QR_2_PI2_THRESHOLD
#define DIV_QR_2_PI2_THRESHOLD (~((size_t)0))
#endif

static mpn_limb_t mpn_div_qr_2n_pi1(mpn_limb_t *qp, mpn_limb_t *rp, const mpn_limb_t *np, size_t nn, mpn_limb_t d1,
                                    mpn_limb_t d0, mpn_limb_t di)
{
    size_t i;
    mpn_limb_t qh, r1, r0;

    MPN_ASSERT(nn >= 2);
    MPN_ASSERT(d1 & MPN_LIMB_HIGHBIT);

    np += nn - 2;
    r1 = np[1];
    r0 = np[0];

    qh = 0;
    if (r1 >= d1 && (r1 > d1 || r0 >= d0)) {
#if MPN_NAIL_BITS == 0
        USUB_AABB(r1, r0, r1, r0, d1, d0);
#else
        r0 = r0 - d0;
        r1 = r1 - d1 - (r0 >> MPN_LIMB_BITS - 1);
        r0 &= MPN_LIMB_MASK;
#endif
        qh = 1;
    }

    for (i = nn - 2 - 1; i >= 0; i--) {
        mpn_limb_t n0, q;
        n0 = np[-1];
        UDIV_NNNDD(q, r1, r0, r1, r0, n0, d1, d0, di);
        np--;
        qp[i] = q;
    }

    rp[1] = r1;
    rp[0] = r0;

    return qh;
}

static mpn_limb_t mpn_div_qr_2n_pi2(mpn_limb_t *qp, mpn_limb_t *rp, const mpn_limb_t *np, size_t nn, mpn_limb_t d1,
                                    mpn_limb_t d0, mpn_limb_t di1, mpn_limb_t di0)
{
    mpn_limb_t qh;
    size_t i;
    mpn_limb_t r1, r0;

    MPN_ASSERT(nn >= 2);
    MPN_ASSERT(d1 & MPN_LIMB_HIGHBIT);

    r1 = np[nn - 1];
    r0 = np[nn - 2];

    qh = 0;
    if (r1 >= d1 && (r1 > d1 || r0 >= d0)) {
#if MPN_NAIL_BITS == 0
        USUB_AABB(r1, r0, r1, r0, d1, d0);
#else
        r0 = r0 - d0;
        r1 = r1 - d1 - (r0 >> MPN_LIMB_BITS - 1);
        r0 &= MPN_LIMB_MASK;
#endif
        qh = 1;
    }

    for (i = nn - 2; i >= 2; i -= 2) {
        mpn_limb_t n1, n0, q1, q0;
        n1 = np[i - 1];
        n0 = np[i - 2];
        {
            mpn_limb_t _q3, _q2a, _q2, _q1, _q2c, _q1c, _q1d, _q0;
            mpn_limb_t _t1, _t0;
            mpn_limb_t _mask;
            UMUL_AB(_q2, _q1, r0, di1);
            UMUL_AB(_q3, _q2a, r1, di1);
            ++_q2;
            UADD_AABB(_q3, _q2, _q3, _q2, r1, _q2a);
            UMUL_AB(_q2c, _q1c, r1, di0);
            UADD_AABB_C(_q3, _q2, _q1, _q2, _q1, r0, _q1c);
            UMUL_AB(_q1d, _q0, r0, di0);
            UADD_AABB_C(_q2c, _q1, _q0, _q1, _q0, n1, n0);
            UADD_AABB_C(_q3, _q2, _q1, _q2, _q1, _q2c, _q1d);
            UMUL_AB(_t1, _t0, _q2, d0);
            _t1 += _q2 * d1 + _q3 * d0;
            USUB_AABB(r1, r0, n1, n0, _t1, _t0);
            _mask = -(mpn_limb_t)((r1 >= _q1) & ((r1 > _q1) | (r0 >= _q0)));
            UADD_AABB(r1, r0, r1, r0, d1 & _mask, d0 & _mask);
            USUB_AABB(_q3, _q2, _q3, _q2, CNST_LIMB(0), -_mask);
            if (UNLIKELY(r1 >= d1)) {
                if (r1 > d1 || r0 >= d0) {
                    USUB_AABB(r1, r0, r1, r0, d1, d0);
                    UADD_AABB(_q3, _q2, _q3, _q2, CNST_LIMB(0), CNST_LIMB(1));
                }
            }
            (q1) = _q3;
            (q0) = _q2;
        }
        qp[i - 1] = q1;
        qp[i - 2] = q0;
    }

    if (i > 0) {
        mpn_limb_t q;
        UDIV_NNNDD(q, r1, r0, r1, r0, np[0], d1, d0, di1);
        qp[0] = q;
    }
    rp[1] = r1;
    rp[0] = r0;

    return qh;
}

/* 3/2 loop, for unnormalized divisor. Caller must pass shifted d1 and
   d0, while {np,nn} is shifted on the fly. */
static mpn_limb_t mpn_div_qr_2u_pi1(mpn_limb_t *qp, mpn_limb_t *rp, const mpn_limb_t *np, size_t nn, mpn_limb_t d1,
                                    mpn_limb_t d0, int shift, mpn_limb_t di)
{
    mpn_limb_t qh;
    mpn_limb_t r2, r1, r0;
    size_t i;

    MPN_ASSERT(nn >= 2);
    MPN_ASSERT(d1 & MPN_LIMB_HIGHBIT);
    MPN_ASSERT(shift > 0);

    r2 = np[nn - 1] >> (MPN_LIMB_BITS - shift);
    r1 = (np[nn - 1] << shift) | (np[nn - 2] >> (MPN_LIMB_BITS - shift));
    r0 = np[nn - 2] << shift;

    UDIV_NNNDD(qh, r2, r1, r2, r1, r0, d1, d0, di);

    for (i = nn - 2 - 1; i >= 0; i--) {
        mpn_limb_t q;
        r0 = np[i];
        r1 |= r0 >> (MPN_LIMB_BITS - shift);
        r0 <<= shift;
        UDIV_NNNDD(q, r2, r1, r2, r1, r0, d1, d0, di);
        qp[i] = q;
    }

    rp[0] = (r1 >> shift) | (r2 << (MPN_LIMB_BITS - shift));
    rp[1] = r2 >> shift;

    return qh;
}

static unsigned int mpn_div_by2(mpn_limb_t q[], const mpn_limb_t n[], unsigned int nn, mpn_limb_t r[2], mpn_limb_t d[2])
{
    mpn_limb_t d1 = d[1], d0 = d[0], dinv;
    MPN_ASSERT(nn >= 2);
    MPN_ASSERT(d1 > 0);

    if (UNLIKELY(d1 & MPN_LIMB_HIGHBIT)) {
        if (MPN_BELOW_THRESHOLD(nn, DIV_QR_2_PI2_THRESHOLD)) {
            INVERT_PI1(dinv, d1, d0);
            return mpn_div_qr_2n_pi1(q, r, n, nn, d1, d0, dinv);
        } else {
            mpn_limb_t di[2];

            // di[2] = invert 4(dividend) by 2(divisor)
            {
                mpn_limb_t v1, v0, p1, t1, t0, p0, mask;
                INVERT_LIMB(v1, d1);
                p1 = d1 * v1;
                /* <1, v1> * d1 = <B-1, p1> */
                p1 += d0;
                if (p1 < d0) {
                    v1--;
                    mask = -(mpn_limb_t)(p1 >= d1);
                    p1 -= d1;
                    v1 += mask;
                    p1 -= mask & d1;
                }
                /* <1, v1> * d1 + d0 = <B-1, p1> */
                UMUL_AB(t1, p0, d0, v1);
                p1 += t1;
                if (p1 < t1) {
                    if (UNLIKELY(p1 >= d1)) {
                        if (p1 > d1 || p0 >= d0) {
                            USUB_AABB(p1, p0, p1, p0, d1, d0);
                            v1--;
                        }
                    }
                    USUB_AABB(p1, p0, p1, p0, d1, d0);
                    v1--;
                }
                /**
                 * Now v1 is the 3/2 inverse, <1, v1> * <d1, d0> = <B-1, p1, p0>,
                 * with <p1, p0> + <d1, d0> >= B^2.
                 *
                 * The 4/2 inverse is (B^4 - 1) / <d1, d0> = <1, v1, v0>. The
                 * partial remainder after <1, v1> is
                 *
                 * B^4 - 1 - B <1, v1> <d1, d0> = <B-1, B-1, B-1, B-1> - <B-1, p1, p0, 0>
                 *                              = <~p1, ~p0, B-1>
                 */
                UDIV_NNNDD(v0, t1, t0, ~p1, ~p0, ~(mpn_limb_t)0, d1, d0, v1);
                di[0] = v0;
                di[1] = v1;

#ifdef MPN_SANITY_CHECK
                {
                    mpn_limb_t tp[4];
                    mpn_limb_t dp[2];
                    dp[0] = d0;
                    dp[1] = d1;
                    mpn_mul(tp, dp, 2, di, 2);
                    ASSERT_ALWAYS(mpn_add_vectorized(tp + 2, tp + 2, dp, 2) == 0);
                    ASSERT_ALWAYS(tp[2] == ~(mpn_limb_t)0);
                    ASSERT_ALWAYS(tp[3] == ~(mpn_limb_t)0);
                    ASSERT_ALWAYS(mpn_add_vectorized(tp, tp, dp, 2) == 1);
                }
#endif
            }

            return mpn_div_qr_2n_pi2(q, r, n, nn, d1, d0, di[1], di[0]);
        }
    } else {
        int shift;
        COUNT_TRAILING_ZEROS(shift, d1);
        d1 = (d1 << shift) | (d0 >> (MPN_LIMB_BITS - shift));
        d0 <<= shift;
        INVERT_PI1(dinv, d1, d0);

        return mpn_div_qr_2u_pi1(q, r, n, nn, d1, d0, shift, dinv);
    }
}

#ifndef DIVREM_1_NORM_THRESHOLD
#define DIVREM_1_NORM_THRESHOLD ~0
#endif

#ifndef DIVREM_1_UNNORM_THRESHOLD
#define DIVREM_1_UNNORM_THRESHOLD ~0
#endif

mpn_limb_t mpn_divrem_1(mpn_limb_t *qp, mpn_size_t qxn, const mpn_limb_t *up, mpn_size_t un, mpn_limb_t d)
{
    mpn_size_t n;
    mpn_size_t i;
    mpn_limb_t n1, n0;
    mpn_limb_t r = 0;

    MPN_ASSERT(qxn >= 0);
    MPN_ASSERT(un >= 0);
    MPN_ASSERT(d != 0);
    /* FIXME: What's the correct overlap rule when qxn!=0? */
    // MPN_ASSERT(MPN_SAME_OR_SEPARATE_P(qp + qxn, up, un));

    n = un + qxn;
    if (n == 0) return 0;

    d <<= MPN_NAIL_BITS;

    qp += (n - 1); /* Make qp point at most significant quotient limb */

    if ((d & MPN_LIMB_HIGHBIT) != 0) {
        if (un != 0) {
            /* High quotient limb is 0 or 1, skip a divide step. */
            mpn_limb_t q;
            r = up[un - 1] << MPN_NAIL_BITS;
            q = (r >= d);
            *qp-- = q;
            r -= (d & -q);
            r >>= MPN_NAIL_BITS;
            n--;
            un--;
        }

        if (MPN_BELOW_THRESHOLD(n, DIVREM_1_NORM_THRESHOLD)) {
        plain:
            for (i = un - 1; i >= 0; i--) {
                n0 = up[i] << MPN_NAIL_BITS;
                UDIV_NND(*qp, r, r, n0, d);
                r >>= MPN_NAIL_BITS;
                qp--;
            }
            for (i = qxn - 1; i >= 0; i--) {
                UDIV_NND(*qp, r, r, CNST_LIMB(0), d);
                r >>= MPN_NAIL_BITS;
                qp--;
            }
            return r;
        } else {
            /* Multiply-by-inverse, divisor already normalized. */
            mpn_limb_t dinv;
            INVERT_LIMB(dinv, d);

            for (i = un - 1; i >= 0; i--) {
                n0 = up[i] << MPN_NAIL_BITS;
                UDIV_NND_PREINV(*qp, r, r, n0, d, dinv);
                r >>= MPN_NAIL_BITS;
                qp--;
            }
            for (i = qxn - 1; i >= 0; i--) {
                UDIV_NND_PREINV(*qp, r, r, CNST_LIMB(0), d, dinv);
                r >>= MPN_NAIL_BITS;
                qp--;
            }
            return r;
        }
    } else {
        /* Most significant bit of divisor == 0.  */
        int cnt;

        /* Skip a division if high < divisor (high quotient 0).  Testing here
       before normalizing will still skip as often as possible.  */
        if (un != 0) {
            n1 = up[un - 1] << MPN_NAIL_BITS;
            if (n1 < d) {
                r = n1 >> MPN_NAIL_BITS;
                *qp-- = 0;
                n--;
                if (n == 0) return r;
                un--;
            }
        }

        if (!UDIV_NEEDS_NORMALIZATION && MPN_BELOW_THRESHOLD(n, DIVREM_1_UNNORM_THRESHOLD)) { goto plain; }

        COUNT_LEADING_ZEROS(cnt, d);
        d <<= cnt;
        r <<= cnt;

        if (UDIV_NEEDS_NORMALIZATION && MPN_BELOW_THRESHOLD(n, DIVREM_1_UNNORM_THRESHOLD)) {
            mpn_limb_t nshift;
            if (un != 0) {
                n1 = up[un - 1] << MPN_NAIL_BITS;
                r |= (n1 >> (MPN_LIMB_BITS - cnt));
                for (i = un - 2; i >= 0; i--) {
                    n0 = up[i] << MPN_NAIL_BITS;
                    nshift = (n1 << cnt) | (n0 >> (MPN_NUMB_BITS - cnt));
                    UDIV_NND(*qp, r, r, nshift, d);
                    r >>= MPN_NAIL_BITS;
                    qp--;
                    n1 = n0;
                }
                UDIV_NND(*qp, r, r, n1 << cnt, d);
                r >>= MPN_NAIL_BITS;
                qp--;
            }
            for (i = qxn - 1; i >= 0; i--) {
                UDIV_NND(*qp, r, r, CNST_LIMB(0), d);
                r >>= MPN_NAIL_BITS;
                qp--;
            }
            return r >> cnt;
        } else {
            mpn_limb_t dinv, nshift;
            INVERT_LIMB(dinv, d);
            if (un != 0) {
                n1 = up[un - 1] << MPN_NAIL_BITS;
                r |= (n1 >> (MPN_LIMB_BITS - cnt));
                for (i = un - 2; i >= 0; i--) {
                    n0 = up[i] << MPN_NAIL_BITS;
                    nshift = (n1 << cnt) | (n0 >> (MPN_NUMB_BITS - cnt));
                    UDIV_NND_PREINV(*qp, r, r, nshift, d, dinv);
                    r >>= MPN_NAIL_BITS;
                    qp--;
                    n1 = n0;
                }
                UDIV_NND_PREINV(*qp, r, r, n1 << cnt, d, dinv);
                r >>= MPN_NAIL_BITS;
                qp--;
            }
            for (i = qxn - 1; i >= 0; i--) {
                UDIV_NND_PREINV(*qp, r, r, CNST_LIMB(0), d, dinv);
                r >>= MPN_NAIL_BITS;
                qp--;
            }
            return r >> cnt;
        }
    }
}

mpn_limb_t mpn_divrem_2(mpn_limb_t *qp, mpn_size_t qxn, mpn_limb_t *np, mpn_size_t nn, const mpn_limb_t *dp)
{
    mpn_limb_t most_significant_q_limb;
    mpn_size_t i;
    mpn_limb_t r1, r0, d1, d0;
    mpn_limb_t di;

    MPN_ASSERT(nn >= 2);
    MPN_ASSERT(qxn >= 0);
    MPN_ASSERT(dp[1] & MPN_NUMB_HIGHBIT);
    MPN_ASSERT(!MPN_OVERLAP_P(qp, nn - 2 + qxn, np, nn) || qp >= np + 2);
    ASSERT_MPN(np, nn);
    ASSERT_MPN(dp, 2);

    np += nn - 2;
    d1 = dp[1];
    d0 = dp[0];
    r1 = np[1];
    r0 = np[0];

    most_significant_q_limb = 0;
    if (r1 >= d1 && (r1 > d1 || r0 >= d0)) {
#if MPN_NAIL_BITS == 0
        USUB_AABB(r1, r0, r1, r0, d1, d0);
#else
        r0 = r0 - d0;
        r1 = r1 - d1 - (r0 >> MPN_LIMB_BITS - 1);
        r0 &= MPN_NUMB_MASK;
#endif
        most_significant_q_limb = 1;
    }

    INVERT_PI1(di, d1, d0);

    qp += qxn;

    for (i = nn - 2 - 1; i >= 0; i--) {
        mpn_limb_t n0, q;
        n0 = np[-1];
        UDIV_NNNDD(q, r1, r0, r1, r0, n0, d1, d0, di);
        np--;
        qp[i] = q;
    }

    if (UNLIKELY(qxn != 0)) {
        qp -= qxn;
        for (i = qxn - 1; i >= 0; i--) {
            mpn_limb_t q;
            UDIV_NNNDD(q, r1, r0, r1, r0, CNST_LIMB(0), d1, d0, di);
            qp[i] = q;
        }
    }

    np[1] = r1;
    np[0] = r0;

    return most_significant_q_limb;
}

#if 0
void mpn_tdiv_qr(mpn_limb_t *qp, mpn_limb_t *rp, mpn_size_t qxn, const mpn_limb_t *np, mpn_size_t nn,
                 const mpn_limb_t *dp, mpn_size_t dn)
{
    ASSERT_ALWAYS(qxn == 0);

    ASSERT(nn >= 0);
    ASSERT(dn >= 0);
    ASSERT(dn == 0 || dp[dn - 1] != 0);
    ASSERT(!MPN_OVERLAP_P(qp, nn - dn + 1 + qxn, np, nn));
    ASSERT(!MPN_OVERLAP_P(qp, nn - dn + 1 + qxn, dp, dn));

    switch (dn) {
        case 0:
            MPI_RAISE_EXCEPTION();
            // DIVIDE_BY_ZERO;

        case 1: {
            rp[0] = mpn_divrem_1(qp, (mpn_size_t)0, np, nn, dp[0]);
            return;
        }

        case 2: {
            mpn_limb_t *n2p;
            mpn_limb_t qhl, cy;

            if ((dp[1] & MPN_NUMB_HIGHBIT) == 0) {
                int cnt;
                mpn_limb_t d2p[2];
                COUNT_LEADING_ZEROS(cnt, dp[1]);
                cnt -= MPN_NAIL_BITS;
                d2p[1] = (dp[1] << cnt) | (dp[0] >> (MPN_NUMB_BITS - cnt));
                d2p[0] = (dp[0] << cnt) & MPN_NUMB_MASK;
                n2p = malloc(MPN_LIMB_BYTES * (nn + 1));
                cy = mpn_lshift(n2p, np, nn, cnt);
                n2p[nn] = cy;
                qhl = mpn_divrem_2(qp, 0L, n2p, nn + (cy != 0), d2p);
                if (cy == 0) qp[nn - 2] = qhl; /* always store nn-2+1 quotient limbs */
                rp[0] = (n2p[0] >> cnt) | ((n2p[1] << (MPN_NUMB_BITS - cnt)) & MPN_NUMB_MASK);
                rp[1] = (n2p[1] >> cnt);
            } else {
                n2p = malloc(MPN_LIMB_BYTES * (nn));
                MPN_COPY(n2p, np, nn);
                qhl = mpn_divrem_2(qp, 0L, n2p, nn, dp);
                qp[nn - 2] = qhl; /* always store nn-2+1 quotient limbs */
                rp[0] = n2p[0];
                rp[1] = n2p[1];
            }
            free(n2p); // XXX
            return;
        }

        default: {
            int adjust;
            mpn_limb_t dinv;

            adjust = np[nn - 1] >= dp[dn - 1]; /* conservative tests for quotient size */
            if (nn + adjust >= 2 * dn) {
                mpn_limb_t *n2p, d2p;
                mpn_limb_t cy;
                int cnt;

                qp[nn - dn] = 0;                          /* zero high quotient limb */
                if ((dp[dn - 1] & MPN_NUMB_HIGHBIT) == 0) /* normalize divisor */
                {
                    COUNT_LEADING_ZEROS(cnt, dp[dn - 1]);
                    cnt -= MPN_NAIL_BITS;
                    d2p = malloc(MPN_LIMB_BYTES * dn);
                    mpn_lshift(d2p, dp, dn, cnt);
                    n2p = malloc(MPN_LIMB_BYTES * (nn + 1));
                    cy = mpn_lshift(n2p, np, nn, cnt);
                    n2p[nn] = cy;
                    nn += adjust;
                } else {
                    cnt = 0;
                    d2p = (mpn_limb_t *)dp;
                    n2p = malloc(MPN_LIMB_BYTES * (nn + 1));
                    MPN_COPY(n2p, np, nn);
                    n2p[nn] = 0;
                    nn += adjust;
                }

                INVERT_PI1(dinv, d2p[dn - 1], d2p[dn - 2]);
                if (MPN_BELOW_THRESHOLD(dn, DC_DIV_QR_THRESHOLD))
                    mpn_sbpi1_div_qr(qp, n2p, nn, d2p, dn, dinv.inv32);
                else if (BELOW_THRESHOLD(dn, MUPI_DIV_QR_THRESHOLD) ||                    /* fast condition */
                         BELOW_THRESHOLD(nn, 2 * MU_DIV_QR_THRESHOLD) ||                  /* fast condition */
                         (double)(2 * (MU_DIV_QR_THRESHOLD - MUPI_DIV_QR_THRESHOLD)) * dn /* slow... */
                                 + (double)MUPI_DIV_QR_THRESHOLD * nn
                             > (double)dn * nn) /* ...condition */
                    mpn_dcpi1_div_qr(qp, n2p, nn, d2p, dn, &dinv);
                else {
                    mpn_size_t itch = mpn_mu_div_qr_itch(nn, dn, 0);
                    mpn_limb_t *scratch = TMP_ALLOC_LIMBS(itch);
                    mpn_mu_div_qr(qp, rp, n2p, nn, d2p, dn, scratch);
                    n2p = rp;
                }

                if (cnt != 0) {
                    mpn_rshift(rp, n2p, dn, cnt);
                } else {
                    MPN_COPY(rp, n2p, dn);
                }
                // XXX: TMP_FREE;
                return;
            }

            /* When we come here, the numerator/partial remainder is less
               than twice the size of the denominator.  */

            {
                /* Problem:

                   Divide a numerator N with nn limbs by a denominator D with dn
                   limbs forming a quotient of qn=nn-dn+1 limbs.  When qn is small
                   compared to dn, conventional division algorithms perform poorly.
                   We want an algorithm that has an expected running time that is
                   dependent only on qn.

                   Algorithm (very informally stated):

                   1) Divide the 2 x qn most significant limbs from the numerator
                  by the qn most significant limbs from the denominator.  Call
                  the result qest.  This is either the correct quotient, but
                  might be 1 or 2 too large.  Compute the remainder from the
                  division.  (This step is implemented by an mpn_divrem call.)

                   2) Is the most significant limb from the remainder < p, where p
                  is the product of the most significant limb from the quotient
                  and the next(d)?  (Next(d) denotes the next ignored limb from
                  the denominator.)  If it is, decrement qest, and adjust the
                  remainder accordingly.

                   3) Is the remainder >= qest?  If it is, qest is the desired
                  quotient.  The algorithm terminates.

                   4) Subtract qest x next(d) from the remainder.  If there is
                  borrow out, decrement qest, and adjust the remainder
                  accordingly.

                   5) Skip one word from the denominator (i.e., let next(d) denote
                  the next less significant limb.  */

                mpn_size_t qn;
                mpn_limb_t *n2p, d2p;
                mpn_limb_t *tp;
                mpn_limb_t cy;
                mpn_size_t in, rn;
                mpn_limb_t quotient_too_large;
                unsigned int cnt;

                qn = nn - dn;
                qp[qn] = 0;   /* zero high quotient limb */
                qn += adjust; /* qn cannot become bigger */

                if (qn == 0) {
                    MPN_COPY(rp, np, dn);
                    // XXX: TMP_FREE;
                    return;
                }

                in = dn - qn; /* (at least partially) ignored # of limbs in ops */
                /* Normalize denominator by shifting it to the left such that its
                   most significant bit is set.  Then shift the numerator the same
                   amount, to mathematically preserve quotient.  */
                if ((dp[dn - 1] & MPN_NUMB_HIGHBIT) == 0) {
                    COUNT_LEADING_ZEROS(cnt, dp[dn - 1]);
                    cnt -= MPN_NAIL_BITS;

                    d2p = TMP_ALLOC_LIMBS(qn);
                    mpn_lshift(d2p, dp + in, qn, cnt);
                    d2p[0] |= dp[in - 1] >> (MPN_NUMB_BITS - cnt);

                    n2p = TMP_ALLOC_LIMBS(2 * qn + 1);
                    cy = mpn_lshift(n2p, np + nn - 2 * qn, 2 * qn, cnt);
                    if (adjust) {
                        n2p[2 * qn] = cy;
                        n2p++;
                    } else {
                        n2p[0] |= np[nn - 2 * qn - 1] >> (MPN_NUMB_BITS - cnt);
                    }
                } else {
                    cnt = 0;
                    d2p = (mpn_limb_t *)dp + in;

                    n2p = TMP_ALLOC_LIMBS(2 * qn + 1);
                    MPN_COPY(n2p, np + nn - 2 * qn, 2 * qn);
                    if (adjust) {
                        n2p[2 * qn] = 0;
                        n2p++;
                    }
                }

                /* Get an approximate quotient using the extracted operands.  */
                if (qn == 1) {
                    mpn_limb_t q0, r0;
                    udiv_qrnnd(q0, r0, n2p[1], n2p[0] << MPN_NAIL_BITS, d2p[0] << MPN_NAIL_BITS);
                    n2p[0] = r0 >> MPN_NAIL_BITS;
                    qp[0] = q0;
                } else if (qn == 2)
                    mpn_divrem_2(qp, 0L, n2p, 4L, d2p); /* FIXME: obsolete function */
                else {
                    invert_pi1(dinv, d2p[qn - 1], d2p[qn - 2]);
                    if (BELOW_THRESHOLD(qn, DC_DIV_QR_THRESHOLD))
                        mpn_sbpi1_div_qr(qp, n2p, 2 * qn, d2p, qn, dinv.inv32);
                    else if (BELOW_THRESHOLD(qn, MU_DIV_QR_THRESHOLD))
                        mpn_dcpi1_div_qr(qp, n2p, 2 * qn, d2p, qn, &dinv);
                    else {
                        mpn_size_t itch = mpn_mu_div_qr_itch(2 * qn, qn, 0);
                        mpn_limb_t *scratch = TMP_ALLOC_LIMBS(itch);
                        mpn_limb_t *r2p = rp;
                        if (np == r2p)      /* If N and R share space, put ... */
                            r2p += nn - qn; /* intermediate remainder at N's upper end. */
                        mpn_mu_div_qr(qp, r2p, n2p, 2 * qn, d2p, qn, scratch);
                        MPN_COPY(n2p, r2p, qn);
                    }
                }

                rn = qn;
                /* Multiply the first ignored divisor limb by the most significant
                   quotient limb.  If that product is > the partial remainder's
                   most significant limb, we know the quotient is too large.  This
                   test quickly catches most cases where the quotient is too large;
                   it catches all cases where the quotient is 2 too large.  */
                {
                    mpn_limb_t dl, x;
                    mpn_limb_t h, dummy;

                    if (in - 2 < 0)
                        dl = 0;
                    else
                        dl = dp[in - 2];

#if MPN_NAIL_BITS == 0
                    x = (dp[in - 1] << cnt) | ((dl >> 1) >> ((~cnt) % GMP_LIMB_BITS));
#else
                    x = (dp[in - 1] << cnt) & MPN_NUMB_MASK;
                    if (cnt != 0) x |= dl >> (MPN_NUMB_BITS - cnt);
#endif
                    umul_ppmm(h, dummy, x, qp[qn - 1] << MPN_NAIL_BITS);

                    if (n2p[qn - 1] < h) {
                        mpn_limb_t cy;

                        mpn_decr_u(qp, (mpn_limb_t)1);
                        cy = mpn_add_n(n2p, n2p, d2p, qn);
                        if (cy) {
                            /* The partial remainder is safely large.  */
                            n2p[qn] = cy;
                            ++rn;
                        }
                    }
                }

                quotient_too_large = 0;
                if (cnt != 0) {
                    mpn_limb_t cy1, cy2;

                    /* Append partially used numerator limb to partial remainder.  */
                    cy1 = mpn_lshift(n2p, n2p, rn, MPN_NUMB_BITS - cnt);
                    n2p[0] |= np[in - 1] & (MPN_NUMB_MASK >> cnt);

                    /* Update partial remainder with partially used divisor limb.  */
                    cy2 = mpn_submul_1(n2p, qp, qn, dp[in - 1] & (MPN_NUMB_MASK >> cnt));
                    if (qn != rn) {
                        ASSERT_ALWAYS(n2p[qn] >= cy2);
                        n2p[qn] -= cy2;
                    } else {
                        n2p[qn] = cy1 - cy2; /* & MPN_NUMB_MASK; */

                        quotient_too_large = (cy1 < cy2);
                        ++rn;
                    }
                    --in;
                }
                /* True: partial remainder now is neutral, i.e., it is not shifted up.  */

                tp = TMP_ALLOC_LIMBS(dn);

                if (in < qn) {
                    if (in == 0) {
                        MPN_COPY(rp, n2p, rn);
                        ASSERT_ALWAYS(rn == dn);
                        goto foo;
                    }
                    mpn_mul(tp, qp, qn, dp, in);
                } else
                    mpn_mul(tp, dp, in, qp, qn);

                cy = mpn_sub(n2p, n2p, rn, tp + in, qn);
                MPN_COPY(rp + in, n2p, dn - in);
                quotient_too_large |= cy;
                cy = mpn_sub_n(rp, np, tp, in);
                cy = mpn_sub_1(rp + in, rp + in, rn, cy);
                quotient_too_large |= cy;
            foo:
                if (quotient_too_large) {
                    mpn_decr_u(qp, (mpn_limb_t)1);
                    mpn_add_n(rp, rp, dp, dn);
                }
            }
            TMP_FREE;
            return;
        }
    }
}

mpn_limb_t mpn_divrem(mpn_limb_t *qp, mpn_size_t qxn, mpn_limb_t *np, mpn_size_t nn, const mpn_limb_t *dp,
                      mpn_size_t dn)
{
    MPN_ASSERT(qxn >= 0);
    MPN_ASSERT(nn >= dn);
    MPN_ASSERT(dn >= 1);
    MPN_ASSERT(dp[dn - 1] & MPN_NUMB_HIGHBIT);
    MPN_ASSERT(!MPN_OVERLAP_P(np, nn, dp, dn));
    MPN_ASSERT(!MPN_OVERLAP_P(qp, nn - dn + qxn, np, nn) || qp == np + dn + qxn);
    MPN_ASSERT(!MPN_OVERLAP_P(qp, nn - dn + qxn, dp, dn));
    ASSERT_MPN(np, nn);
    ASSERT_MPN(dp, dn);

    if (dn == 1) {
        mpn_limb_t ret;
        mpn_limb_t *q2p;
        mpn_size_t qn;

        // XXX
        q2p = malloc(MPN_LIMB_BYTES * (nn + qxn));

        np[0] = mpn_divrem_1(q2p, qxn, np, nn, dp[0]);
        qn = nn + qxn - 1;
        COPY(qp, q2p, qn);
        ret = q2p[qn];

        // XXX
        free(q2p);

        return ret;
    } else if (dn == 2) {
        return mpn_divrem_2(qp, qxn, np, nn, dp);
    } else {
        mpn_limb_t *q2p;
        mpn_limb_t qhl;
        mpn_size_t qn;

        if (UNLIKELY(qxn != 0)) {
            mpn_limb_t *n2p;
            n2p = malloc(MPN_LIMB_BYTES * (nn + qxn));
            q2p = malloc(MPN_LIMB_BYTES * (nn - dn + qxn + 1));

            ZEROIZE(n2p, 0, qxn);
            COPY(n2p + qxn, np, nn);
            mpn_tdiv_qr(q2p, np, 0L, n2p, nn + qxn, dp, dn);
            qn = nn - dn + qxn;
            COPY(qp, q2p, qn);
            qhl = q2p[qn];

            free(n2p);
            free(q2p);
        } else {
            q2p = malloc(MPN_LIMB_BYTES * (nn - dn + 1));
            mpn_tdiv_qr(q2p, np, 0L, np, nn, dp, dn);
            qn = nn - dn;
            COPY(qp, q2p, qn);
            qhl = q2p[qn];
            free(q2p);
        }

        return qhl;
    }
}
#endif


/**
 * mpn division: xsize, q, x(q = x / y, x = x % y)
 */
unsigned int mpn_div(mpn_limb_t *q, unsigned int *qsize, mpn_limb_t *x, unsigned int xsize, mpn_limb_t *y,
                     unsigned int ysize)
{
#if (defined MPN_UDIV_ASM) && (MPN_LIMB_BITS == 64)
    // FIXME
    /**
     * mpn(half) division: xsize, q, x(q = x / y, x = x % y)
     */
    unsigned int mpn_div_u32(uint32_t * q, unsigned int *qsize, uint32_t *x, unsigned int xsize, uint32_t *y,
                             unsigned int ysize);

    unsigned int size = mpn_div_u32((uint32_t *)q, qsize, (uint32_t *)x, xsize * 2, (uint32_t *)y, ysize * 2);
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
        unsigned int qsz = mpn_div_limb(q, x, xsize, &x[0], y[0]);
        if (q != NULL && qsize != NULL) { *qsize = qsz; }

        return 1;
    }

    if (ysize == 2) {
        unsigned int qsz = mpn_div_by2(q, x, xsize, 0, y);
        if (q != NULL && qsize != NULL) { *qsize = qsz; }

        return 2;
    }

    /* common case */
    {
        unsigned int qsz = xsize - ysize + 1;
        unsigned int shifts = mpn_limb_nlz_consttime(y[ysize - 1]);

        /* normalization */
        x[xsize] = 0;
        if (shifts != 0) {
            mpn_lshift(x, x, xsize, shifts);
            mpn_lshift(y, y, ysize, shifts);
        }

        /* zeroize */
        if (q != NULL && qsize != NULL) { ZEROIZE(q, 0, qsz); }

        /* division */
        {
            mpn_limb_t yy = y[ysize - 1];
            for (unsigned int i = xsize; i >= ysize; i--) {
                /* estimate digit of quotient */
                mpn_limb_t __q, __r = 0;

                MPN_ASSERT(x[i] <= yy); // WILL NEVER happen
                if (x[i] == yy) {
                    __q = MPN_LIMB_MASK;
                } else {
                    UDIV_NND(__q, __r, x[i], x[i - 1], yy);

                    /* tune estimation above */
                    {
                        mpn_limb_t rH, rL, xxx = x[xsize - 3], yyy = y[ysize - 2];
                        UMUL_AB(rH, rL, __q, y[ysize - 2]);
                        for (;;) {
                            /**
                             * break condition:
                             *   q[i - t -1] * (y[t] * b + y[t - 1]) > x[i] * b ^ 2 + x[i - 1] * b + x[i -
                             * 2]
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
                    mpn_limb_t extend = mpn_sub_mul(&x[i - ysize], y, ysize, __q);
                    extend = (x[i] -= extend);
                    if (extend > 0) { /* subtracted too much */
                        extend = mpn_add_vectorized(&x[i - ysize], &x[i - ysize], y, ysize);
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
            mpn_rshift(x, x, xsize, shifts);
            mpn_rshift(y, y, ysize, shifts);
        }
        xsize = mpn_limbs(x, xsize);

        if (q != NULL && qsize != NULL) { *qsize = mpn_limbs(q, qsz); }

        return xsize;
    }
#endif
}

/**
 * mpn modular: x[] = x[] % y[]
 */
unsigned int mpn_mod(mpn_limb_t *x, unsigned int xsize, mpn_limb_t *y, unsigned int ysize)
{
    return mpn_div(NULL, NULL, x, xsize, y, ysize);
}

/**
 * greatest common divisor(mpn_limb_t)
 */
mpn_limb_t mpn_limb_gcd(mpn_limb_t a, mpn_limb_t b)
{
    mpn_limb_t g = 1;

    if (a < b) {
        mpn_limb_t t = a;
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
static unsigned int umul_acc_bin(mpn_limb_t *r, unsigned int rsize, const mpn_limb_t *a, unsigned int asize,
                                 const mpn_limb_t *b, unsigned int bsize)
{
    unsigned int tsize = asize + bsize;
    if (tsize > rsize) {
        ZEROIZE(r, rsize, tsize - 1); /* cleanup the rest of destination buffer */
        rsize = tsize;
    }

    {
        mpn_limb_t expansion = 0;
        for (unsigned int i = 0; i < bsize && !expansion; i++) {
            expansion = mpn_mul_acc(r + i, a, asize, b[i]);
            if (expansion) {
                expansion = mpn_inc_vectorized(r + i + asize, r + i + asize, rsize - i - asize, expansion);
            }
        }

        if (expansion) {
            return 0; // no enought buffer size to write
        } else {
            return mpn_limbs(r, rsize);
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
unsigned int mpn_mod_invert(mpn_limb_t *r, const mpn_limb_t *a, unsigned int asize, const mpn_limb_t *m,
                            unsigned int msize, mpn_optimizer_t *optimizer)
{
    MPN_ASSERT(optimizer != NULL);
    MPN_ASSERT(asize == mpn_limbs(a, asize));
    MPN_ASSERT(msize == mpn_limbs(m, msize));

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
        mpn_limb_t *mtemp = mpn_optimizer_get_limbs(optimizer, msize);
        mpn_limb_t *abuff = mpn_optimizer_get_limbs(optimizer, asize);
        mpn_limb_t *mbuff = mpn_optimizer_get_limbs(optimizer, msize);
        mpn_limb_t *invbuf = mpn_optimizer_get_limbs(optimizer, msize);
        MPN_ASSERT(mtemp != NULL && abuff != NULL && mbuff != NULL && invbuf != NULL);

        COPY(mtemp, m, msize);

        unsigned int modsize = msize;
        mpn_limb_t *X1 = r, *X2 = mbuff, *Q = invbuf;
        unsigned int x1size = 1, x2size = 1, qsize;

        COPY(abuff, a, asize);
        ZEROIZE(X1, 0, modsize);
        ZEROIZE(X2, 0, modsize);
        X2[0] = 1;

        for (;;) {
            msize = mpn_div(Q, &qsize, mtemp, msize, abuff, asize);
            x1size = umul_acc_bin(X1, modsize, Q, qsize, X2, x2size);

            if (msize == 1 && m[0] == 1) {
                // ZEROIZE(X2, x2size, modsize);
                x2size = umul_acc_bin(X2, modsize, X1, x1size, abuff, asize);
                COPY(mtemp, X2, modsize);
                mpn_sub_vectorized(r, m, X1, modsize);

                size = mpn_limbs(r, modsize);

                goto operation_end;
            } else if (msize == 1 && m[0] == 0) {
                mpn_mul(mtemp, X1, x1size, abuff, asize);
                size = 0; /* gcd = a */

                goto operation_end;
            }

            asize = mpn_div(Q, &qsize, abuff, asize, mtemp, msize);
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
                mpn_mul(mtemp, X2, x2size, X1, msize);

                size = 0;

                goto operation_end;
            }
        }

    operation_end:
        mpn_optimizer_put_limbs(optimizer, msize);
        mpn_optimizer_put_limbs(optimizer, msize);
        mpn_optimizer_put_limbs(optimizer, asize);
        mpn_optimizer_put_limbs(optimizer, msize);

        return size;
    }
}

/**
 * mpn: ALL-ones if buff[::] is zero, otherwise ALL-zeros
 */
mpn_limb_t mpn_is_zero(const mpn_limb_t *buff, unsigned int bufflen)
{
    for (unsigned int i = 0; i < bufflen; i++) {
        if (buff[i] != 0) { return 0; }
    }
    return MPN_LIMB_MASK;
}

/**
 * mpn: ALL-ones if buff[::] is zero, otherwise ALL-zeros(constant-time version)
 */
mpn_limb_t mpn_is_zero_consttime(const mpn_limb_t *buff, unsigned int bufflen)
{
    mpn_limb_t diff = 0;
    for (unsigned int i = 0; i < bufflen; i++) { diff |= buff[i]; }
    return mpn_limb_is_zero_consttime(diff);
}

/**
 * mpn: generate in range (lo[] < r < hi[])
 *
 * @note:
 *   1. length of |r| >= hilen
 */
int mpn_random_range(mpn_limb_t *r, unsigned int maxtries, const mpn_limb_t *lo, unsigned int lolen,
                     const mpn_limb_t *hi, unsigned int hilen, int (*rand_bytes)(void *, unsigned char *, unsigned int),
                     void *rand_state)
{
    mpn_limb_t topmask = MPN_MASK_LIMB_HI(mpn_bits_consttime(hi, hilen));

    for (unsigned int i = 0; i < maxtries; i++) {
        int err;
        if ((err = rand_bytes(rand_state, (unsigned char *)r, hilen * MPN_LIMB_BYTES)) != 0) {
            MPI_RAISE_ERROR(err, "generation failed, iterations = %u, err = %d", i, err);
            return err;
        }
        r[hilen - 1] &= topmask;
        unsigned int randlen = mpn_limbs(r, hilen);

        if (mpn_cmp(r, randlen, lo, lolen) > 0 && mpn_cmp(r, randlen, hi, hilen) < 0) { return 0; }
    }

    return -EBUSY;
}

/**
 *  mpn: create mpn from big-endian octets
 */
unsigned int mpn_from_octets(mpn_limb_t *r, unsigned int size, const unsigned char *in, unsigned int inlen)
{
    unsigned int h = 0;
    unsigned int left = inlen;
    while (left > 0) {
        mpn_limb_t l = 0;
        unsigned m = (MPN_LIMB_BYTES <= left) ? MPN_LIMB_BYTES : left;
        for (unsigned i = m; i > 0; i--) { // big-endian
            l = (l << BITS_PER_BYTE) | in[left - i];
        }
        MPN_ASSERT(h < size);
        r[h++] = l;
        left -= m;
    }

    return mpn_limbs(r, h);
}

/**
 *  mpn: convert mpn to big-endian octets
 */
unsigned int mpn_to_octets(unsigned char *out, unsigned int outsize, const mpn_limb_t *a, unsigned int size)
{
    MPN_ASSERT(out != NULL);
    MPN_ASSERT(a != NULL && size != 0);

    unsigned int reqsize = MPN_BITS_TO_BYTES(mpn_bits_consttime(a, size));
    MPN_ASSERT(outsize >= reqsize);

    unsigned char *p = out;
    unsigned int left = reqsize;
    const mpn_limb_t *w = &a[size - 1];
    if ((left & (MPN_LIMB_BYTES - 1)) != 0) {
        unsigned char buff[MPN_LIMB_BYTES];
        MPN_LIMB_TO_OCTETS(buff, *w);
        unsigned int n = left & (MPN_LIMB_BYTES - 1);
        memcpy(p, &buff[MPN_LIMB_BYTES - n], n);
        w--;
        p += n;
        left -= n;
    }
    while (left >= MPN_LIMB_BYTES) {
        MPN_LIMB_TO_OCTETS(p, *w);
        left -= MPN_LIMB_BYTES;
        p += MPN_LIMB_BYTES;
        w--;
    }

    return reqsize;
}

/**
 *  mpn: create mpn from hex string
 */
unsigned int mpn_from_string(mpn_limb_t *r, unsigned int size, const char *in, unsigned int inlen)
{
    unsigned int h = 0;
    while (inlen > 0) {
        mpn_limb_t l = 0;
        unsigned int m = (MPN_LIMB_BYTES * 2 <= inlen) ? (MPN_LIMB_BYTES * 2) : inlen;
        for (unsigned int i = m; i > 0; i--) { // big-endian
            char c = in[inlen - i];
            l = (l << BITS_PER_CHAR)
                | ((c > '9' ? c + 9 : c) & 0x0F /* to hex-char if isxdigit(c) which checked before */);
        }
        MPN_ASSERT(h < size);
        r[h++] = l;
        inlen -= m;
    }
    return mpn_limbs(r, h);
}

/**
 *  mpn: convert mpn to hex string
 */
unsigned int mpn_to_string(char *out, unsigned int outsize, const mpn_limb_t *a, unsigned int size)
{
    if (size == 0) {
        MPN_ASSERT(outsize >= 2);
        strcpy(out, "0");
        return 1;
    }

    char *p = out;
    unsigned notfirst = 0; /* to strip leading zeros */
    static const char HEX_CHARS[] = "0123456789ABCDEF";

    for (unsigned int i = size; i > 0; i--) {
        mpn_limb_t vv = a[i - 1];
        for (int j = MPN_LIMB_BITS - BITS_PER_BYTE; j >= 0; j -= BITS_PER_BYTE) {
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
