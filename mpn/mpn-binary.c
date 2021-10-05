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
mpn_size_t mpn_limb_nlz_consttime(mpn_limb_t x)
{
    mpn_size_t nlz = 0;
    mpn_limb_t mask;
#if (MPN_LIMB_BITS == 64)
    mask = mpn_limb_is_zero_consttime(x & 0xFFFFFFFF00000000);
    nlz += (mpn_size_t)(32 & mask);
    x = ((x << 32) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xFFFF000000000000);
    nlz += (mpn_size_t)(16 & mask);
    x = ((x << 16) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xFF00000000000000);
    nlz += (mpn_size_t)(8 & mask);
    x = ((x << 8) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xF000000000000000);
    nlz += (mpn_size_t)(4 & mask);
    x = ((x << 4) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xC000000000000000);
    nlz += (mpn_size_t)(2 & mask);
    x = ((x << 2) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x8000000000000000);
    nlz += (mpn_size_t)(1 & mask);
    x = ((x << 1) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x8000000000000000);
    nlz += (mpn_size_t)(1 & mask);
#elif MPN_LIMB_BITS == 32
    mask = mpn_limb_is_zero_consttime(x & 0xFFFF0000);
    nlz += (mpn_size_t)(16 & mask);
    x = ((x << 16) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xFF000000);
    nlz += (mpn_size_t)(8 & mask);
    x = ((x << 8) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xF0000000);
    nlz += (mpn_size_t)(4 & mask);
    x = ((x << 4) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xC0000000);
    nlz += (mpn_size_t)(2 & mask);
    x = ((x << 2) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x80000000);
    nlz += (mpn_size_t)(1 & mask);
    x = ((x << 1) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x80000000);
    nlz += (mpn_size_t)(1 & mask);
#elif MPN_LIMB_BITS == 16
    mask = mpn_limb_is_zero_consttime(x & 0xFF00);
    nlz += (mpn_size_t)(8 & mask);
    x = ((x << 8) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xF000);
    nlz += (mpn_size_t)(4 & mask);
    x = ((x << 4) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0xC000);
    nlz += (mpn_size_t)(2 & mask);
    x = ((x << 2) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x8000);
    nlz += (mpn_size_t)(1 & mask);
    x = ((x << 1) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x8000);
    nlz += (mpn_size_t)(1 & mask);
#endif
    return nlz;
}

/**
 * trailing zeros counting(constant-time version)
 */
mpn_size_t mpn_limb_ntz_consttime(mpn_limb_t x)
{
    mpn_size_t ntz = mpn_limb_is_zero_consttime(x) & 1;

    mpn_limb_t mask;
#if (MPN_LIMB_BITS == 64)
    mask = mpn_limb_is_zero_consttime(x & 0x00000000FFFFFFFF);
    ntz += (mpn_size_t)(32 & mask);
    x = ((x >> 32) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x000000000000FFFF);
    ntz += (mpn_size_t)(16 & mask);
    x = ((x >> 16) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x00000000000000FF);
    ntz += (mpn_size_t)(8 & mask);
    x = ((x >> 8) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x000000000000000F);
    ntz += (mpn_size_t)(4 & mask);
    x = ((x >> 4) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x0000000000000003);
    ntz += (mpn_size_t)(2 & mask);
    x = ((x >> 2) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x0000000000000001);
    ntz += (mpn_size_t)(1 & mask);
#elif (MPN_LIMB_BITS == 32)
    mask = mpn_limb_is_zero_consttime(x & 0x0000FFFF);
    ntz += (mpn_size_t)(16 & mask);
    x = ((x >> 16) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x000000FF);
    ntz += (mpn_size_t)(8 & mask);
    x = ((x >> 8) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x0000000F);
    ntz += (mpn_size_t)(4 & mask);
    x = ((x >> 4) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x00000003);
    ntz += (mpn_size_t)(2 & mask);
    x = ((x >> 2) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x00000001);
    ntz += (mpn_size_t)(1 & mask);
#elif (MPN_LIMB_BITS == 16)
    mask = mpn_limb_is_zero_consttime(x & 0x00FF);
    ntz += (mpn_size_t)(8 & mask);
    x = ((x >> 8) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x000F);
    ntz += (mpn_size_t)(4 & mask);
    x = ((x >> 4) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x0003);
    ntz += (mpn_size_t)(2 & mask);
    x = ((x >> 2) & mask) | (x & ~mask);

    mask = mpn_limb_is_zero_consttime(x & 0x0001);
    ntz += (mpn_size_t)(1 & mask);
#else
#error Unkown target cpu.
#endif

    return ntz;
}

/**
 * mpn: get most significant bit
 */
mpn_size_t mpn_bits(const mpn_limb_t *data, mpn_size_t size)
{
    if (data == NULL || size == 0) { return 0; }
    mpn_size_t nlz;
    COUNT_LEADING_ZEROS_BUILTIN(nlz, data[size - 1]);

    return (size * MPN_LIMB_BITS - nlz);
}

/**
 * mpn: get most significant bit(constant-time version)
 */
mpn_size_t mpn_bits_consttime(const mpn_limb_t *data, mpn_size_t size)
{
    if (data == NULL || size == 0) { return 0; }
    return (size * MPN_LIMB_BITS - mpn_limb_nlz_consttime(data[size - 1]));
}

/**
 * mpn: get most significant limb
 */
mpn_size_t mpn_limbs(const mpn_limb_t *data, mpn_size_t size)
{
    MPN_ASSERT(data != NULL);
    mpn_size_t fixedsz = size;
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
mpn_size_t mpn_limbs_consttime(const mpn_limb_t *data, mpn_size_t size)
{
    MPN_ASSERT(data != NULL);
    mpn_size_t fixedsz = size;
    mpn_limb_t zscan = (mpn_limb_t)(-1);
    for (; size > 0; size--) {
        zscan &= mpn_limb_is_zero_consttime(data[size - 1]);
        fixedsz -= 1 & zscan;
    }
    return (mpn_size_t)((1 & zscan) | ((mpn_limb_t)fixedsz & ~zscan));
}

/**
 * mpn: unsigned comparison
 *
 * @note:
 *   1. return 1 if a[] > b[]; 0 if a[] = b[]; -1 if a[] < b[]
 */
int mpn_cmp(const mpn_limb_t *a, mpn_size_t asize, const mpn_limb_t *b, mpn_size_t bsize)
{
    MPN_ASSERT(a != NULL && b != NULL);
    if (asize == bsize) {
        mpn_size_t len = asize;
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
mpn_limb_t mpn_add_vectorized(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, mpn_size_t n)
{
    mpn_limb_t carry = 0;
    for (mpn_size_t i = 0; i < n; i++) { UADD_ABC(carry, r[i], a[i], b[i], carry); }

    return carry;
}
#endif

/**
 * mpn: carry, r[] = a[] + b[]
 */
mpn_limb_t mpn_add(mpn_limb_t *r, mpn_size_t rroom, const mpn_limb_t *a, mpn_size_t asize, const mpn_limb_t *b,
                   mpn_size_t bsize)
{
    MPN_ASSERT(r != NULL && a != NULL && b != NULL);
    MPN_ASSERT(asize >= bsize && rroom >= asize);

    mpn_limb_t *rr = r;
    const mpn_limb_t *aa = a, *bb = b;

    mpn_limb_t carry = mpn_add_vectorized(rr, aa, bb, bsize);
    rr += bsize;
    aa += bsize;

    mpn_size_t diff = asize - bsize;
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
mpn_limb_t mpn_inc_vectorized(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t size, mpn_limb_t w)
{
    mpn_limb_t carry = w;

    mpn_size_t i;
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
mpn_limb_t mpn_inc(mpn_limb_t *r, mpn_size_t rroom, const mpn_limb_t *a, mpn_size_t asize, mpn_limb_t w)
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
mpn_limb_t mpn_sub_vectorized(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, mpn_size_t n)
{
    mpn_limb_t borrow = 0;
    for (mpn_size_t i = 0; i < n; i++) { USUB_ABC(borrow, r[i], a[i], b[i], borrow); }

    return borrow;
}
#endif

/**
 * mpn subtraction: size, r[] = a[] - b[]
 */
mpn_size_t mpn_sub(mpn_limb_t *r, mpn_size_t rroom, const mpn_limb_t *a, mpn_size_t asize, const mpn_limb_t *b,
                   mpn_size_t bsize)
{
    MPN_ASSERT(r != NULL && a != NULL && b != NULL);
    MPN_ASSERT(asize >= bsize && asize <= rroom);

    mpn_limb_t *rr = r;
    const mpn_limb_t *aa = a, *bb = b;

    mpn_limb_t borrow = mpn_sub_vectorized(rr, aa, bb, bsize);
    rr += bsize;
    aa += bsize;
    bb += bsize;

    mpn_size_t diff = asize - bsize;
    while (diff > 0) {
        mpn_limb_t t = *aa++;
        *rr++ = t - (borrow);
        borrow = t < borrow;
        diff--;
    }

    mpn_size_t maxsz = asize;
    while (maxsz != 0 && *--rr == 0) { maxsz--; }
    return mpn_limbs(r, maxsz);
}

#ifndef MPN_UDEC_VECTORIZED_ASM
/**
 * mpn: borrow, r[:n] = a[:n] - w
 */
mpn_limb_t mpn_dec_vectorized(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t asize, mpn_limb_t w)
{
    mpn_size_t i;
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
mpn_size_t mpn_dec(mpn_limb_t *r, mpn_size_t rroom, const mpn_limb_t *a, mpn_size_t asize, mpn_limb_t w)
{
    MPN_ASSERT(r != NULL && a != NULL);
    MPN_ASSERT(rroom >= asize);
    MPN_ASSERT(asize > 1 || a[0] >= w);

    mpn_size_t size = asize;
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
mpn_limb_t mpn_mul(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t asize, const mpn_limb_t *b, mpn_size_t bsize)
{
    mpn_limb_t extension = 0;
    ZEROIZE(r, 0, asize + bsize);

    for (mpn_size_t i = 0; i < bsize; i++) {
        mpn_limb_t t = b[i];

        mpn_size_t j;
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
mpn_limb_t mpn_mul_acc(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t asize, mpn_limb_t b)
{
    mpn_limb_t extension = 0;
    for (mpn_size_t i = 0; i < asize; i++) {
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
mpn_limb_t mpn_sqr(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t asize)
{
    mpn_size_t i;
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
        mpn_size_t j;
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
mpn_size_t mpn_lshift(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t asize, mpn_size_t nbits)
{
    MPN_ASSERT(r != NULL && a != NULL);

    mpn_size_t nw = nbits / MPN_LIMB_BITS; // shift words
    mpn_size_t nb = nbits % MPN_LIMB_BITS; // shift bits

    mpn_size_t rsize = asize + nw;
    if (LIKELY(nb > 0)) {
        mpn_limb_t hi = a[asize - 1], lo;
        lo = hi >> (MPN_LIMB_BITS - nb);
        if (lo != 0) { r[rsize++] = lo; }

        for (mpn_size_t i = asize - 1; i > 0; i--) {
            lo = a[i - 1];
            r[i + nw] = (hi << nb) | (lo >> (MPN_LIMB_BITS - nb));
            hi = lo;
        }
        r[nw] = a[0] << nb;
    } else {
        mpn_limb_t *rr = &r[nw];
        for (mpn_size_t i = asize; i > 0; i--) { rr[i - 1] = a[i - 1]; }
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
mpn_size_t mpn_rshift(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t asize, mpn_size_t nbits)
{
    MPN_ASSERT(r != NULL && a != NULL);
    MPN_ASSERT(asize > nbits / MPN_LIMB_BITS);

    mpn_size_t nw = nbits / MPN_LIMB_BITS; // shift words
    mpn_size_t nb = nbits % MPN_LIMB_BITS; // shift bits

    mpn_size_t rsize = asize - nw;
    if (LIKELY(nb > 0)) {
        mpn_limb_t hi, lo = a[nw];
        for (mpn_size_t n = nw; n < asize - 1; n++) {
            hi = a[n + 1];
            r[n - nw] = (lo >> nb) | (hi << (MPN_LIMB_BITS - nb));
            lo = hi;
        }
        lo >>= nb;
        if (lo != 0) {
            r[rsize - 1] = lo;
        } else {
            r[--rsize] = 0;
        }
    } else {
        COPY(r, &a[nw], rsize);
    }

    return rsize;
}

/**
 * mpn: division(n by 1)
 *
 * @note:
 *   1. required length of q should be not smaller than size
 */
mpn_size_t mpn_div_limb(mpn_limb_t q[], const mpn_limb_t x[], mpn_size_t size, mpn_limb_t *r, mpn_limb_t d)
{
    mpn_limb_t __r = 0, __q;
    for (mpn_size_t i = size; i > 0; i--) {
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

mpn_size_t mpn_div_double_limbs(mpn_limb_t q[], mpn_limb_t r[2], const mpn_limb_t n[], mpn_size_t nn,
                                const mpn_limb_t d[2])
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

#if (defined MPN_UDIV_ASM) && (MPN_LIMB_BITS == 64)
MPN_INLINE mpn_size_t mpn_div_asm(mpn_limb_t *q, mpn_size_t *qsize, mpn_limb_t *x, mpn_size_t xsize, mpn_limb_t *y,
                                  mpn_size_t ysize)
{
    unsigned int mpn_div_u32(uint32_t * q, unsigned int *qsize, uint32_t *x, unsigned int xsize, uint32_t *y,
                             unsigned int ysize);

    unsigned int size = mpn_div_u32((uint32_t *)q, qsize, (uint32_t *)x, xsize * 2, (uint32_t *)y, ysize * 2);
    if (size & 1) ((uint32_t *)x)[size] = 0;
    size = (unsigned int)((size + 2 - 1) / 2);
    if (q != NULL && qsize != NULL) {
        if (*qsize & 1) ((uint32_t *)q)[*qsize] = 0;
        *qsize = (mpn_size_t)((*qsize + 2 - 1) / 2);
    }

    return size;
}
#else
#undef MPN_UDIV_ASM
#endif

#ifndef MPN_UDIV_ASM
/**
 * borrow, r[] = r[] - a[] * m
 *
 * rsize >= asize
 */
static mpn_limb_t mpn_sub_mul(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t asize, mpn_limb_t m)
{
    mpn_limb_t borrow = 0, aH, aL;

    for (mpn_size_t i = 0; i < asize; i++) {
        UMUL_AB(aH, aL, a[i], m);                 // aH, aL = a[i] * m
        USUB_ABC(borrow, r[i], r[i], aL, borrow); // borrow, r[i] = r[i] - aL  - borrow
        borrow += aH;                             // WILL NEVER overflow
    }

    return borrow;
}
#endif

/**
 * mpn division: xsize, q, x(q = x / y, x = x % y)
 */
mpn_size_t mpn_div(mpn_limb_t *q, mpn_size_t *qsize, mpn_limb_t *x, mpn_size_t xsize, mpn_limb_t *y, mpn_size_t ysize)
{
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

#if (defined MPN_UDIV_ASM)
    return mpn_div_asm(q, qsize, x, xsize, y, ysize);
#else
    if (ysize == 1) { /* special case */
        mpn_size_t qsz = mpn_div_limb(q, x, xsize, &x[0], y[0]);
        if (q != NULL && qsize != NULL) { *qsize = qsz; }

        return 1;
    } else { /* common case */
        mpn_size_t qsz = xsize - ysize + 1;
        mpn_size_t shifts = mpn_limb_nlz_consttime(y[ysize - 1]);

        /* normalization */
        x[xsize] = 0; /* @note: one additional limb required for normalization */
        if (shifts != 0) {
            mpn_lshift(x, x, xsize, shifts);
            mpn_lshift(y, y, ysize, shifts);
        }

        /* zeroize */
        if (q != NULL && qsize != NULL) { ZEROIZE(q, 0, qsz); }

        /* division */
        {
            mpn_limb_t yy = y[ysize - 1];
            for (mpn_size_t i = xsize; i >= ysize; i--) {
                /* estimate digit of quotient */
                mpn_limb_t __q, __r = 0;

                MPN_ASSERT(x[i] <= yy); // WILL NEVER happen
                if (x[i] == yy) {
                    __q = MPN_LIMB_MASK;
                } else {
                    UDIV_NND(__q, __r, x[i], x[i - 1], yy);

                    /* tune estimation above */
                    {
                        mpn_limb_t rH, rL;
                        mpn_limb_t xxx = xsize >= 3 ? x[xsize - 3] : 0;
                        mpn_limb_t yyy = ysize >= 2 ? y[ysize - 2] : 0;

                        UMUL_AB(rH, rL, __q, yyy);
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
                        // x[i] += extend; // XXX: EXPECT_EQ(0, x[i] += extend)
                        x[i] = 0;
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

        if (q != NULL && qsize != NULL) { *qsize = mpn_limbs(q, qsz); }

        return mpn_limbs(x, xsize);
    }
#endif
}

/**
 * mpn modular: x[] = x[] % y[]
 */
mpn_size_t mpn_mod(mpn_limb_t *x, mpn_size_t xsize, mpn_limb_t *y, mpn_size_t ysize)
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
static mpn_size_t umul_acc_bin(mpn_limb_t *r, mpn_size_t rsize, const mpn_limb_t *a, mpn_size_t asize,
                               const mpn_limb_t *b, mpn_size_t bsize)
{
    mpn_size_t tsize = asize + bsize;
    if (tsize > rsize) {
        ZEROIZE(r, rsize, tsize - 1); /* cleanup the rest of destination buffer */
        rsize = tsize;
    }

    {
        mpn_limb_t expansion = 0;
        for (mpn_size_t i = 0; i < bsize && !expansion; i++) {
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
mpn_size_t mpn_mod_invert(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t asize, const mpn_limb_t *m, mpn_size_t msize,
                          mpn_optimizer_t *optimizer)
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
        mpn_size_t size = 0;
        mpn_limb_t *mtemp = mpn_optimizer_get_limbs(optimizer, msize);
        mpn_limb_t *abuff = mpn_optimizer_get_limbs(optimizer, asize);
        mpn_limb_t *mbuff = mpn_optimizer_get_limbs(optimizer, msize);
        mpn_limb_t *invbuf = mpn_optimizer_get_limbs(optimizer, msize);
        MPN_ASSERT(mtemp != NULL && abuff != NULL && mbuff != NULL && invbuf != NULL);

        COPY(mtemp, m, msize);

        mpn_size_t modsize = msize;
        mpn_limb_t *X1 = r, *X2 = mbuff, *Q = invbuf;
        mpn_size_t x1size = 1, x2size = 1, qsize;

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
mpn_limb_t mpn_is_zero(const mpn_limb_t *buff, mpn_size_t bufflen)
{
    for (mpn_size_t i = 0; i < bufflen; i++) {
        if (buff[i] != 0) { return 0; }
    }
    return MPN_LIMB_MASK;
}

/**
 * mpn: ALL-ones if buff[::] is zero, otherwise ALL-zeros(constant-time version)
 */
mpn_limb_t mpn_is_zero_consttime(const mpn_limb_t *buff, mpn_size_t bufflen)
{
    mpn_limb_t diff = 0;
    for (mpn_size_t i = 0; i < bufflen; i++) { diff |= buff[i]; }
    return mpn_limb_is_zero_consttime(diff);
}

/**
 * mpn: generate in range (lo[] < r < hi[])
 *
 * @note:
 *   1. length of |r| >= hilen
 */
int mpn_random_range(mpn_limb_t *r, mpn_size_t maxtries, const mpn_limb_t *lo, mpn_size_t lolen, const mpn_limb_t *hi,
                     mpn_size_t hilen, int (*rand_bytes)(void *, unsigned char *, mpn_size_t), void *rand_state)
{
    mpn_limb_t topmask = MPN_MASK_LIMB_HI(mpn_bits_consttime(hi, hilen));

    for (mpn_size_t i = 0; i < maxtries; i++) {
        int err;
        if ((err = rand_bytes(rand_state, (unsigned char *)r, hilen * MPN_LIMB_BYTES)) != 0) {
            MPI_RAISE_ERROR(err, "generation failed, iterations = %u, err = %d", i, err);
            return err;
        }
        r[hilen - 1] &= topmask;
        mpn_size_t randlen = mpn_limbs(r, hilen);

        if (mpn_cmp(r, randlen, lo, lolen) > 0 && mpn_cmp(r, randlen, hi, hilen) < 0) { return 0; }
    }

    return -EBUSY;
}

/**
 *  mpn: create mpn from big-endian octets
 */
mpn_size_t mpn_from_octets(mpn_limb_t *r, mpn_size_t size, const unsigned char *in, mpn_size_t inlen)
{
    mpn_size_t h = 0;
    mpn_size_t left = inlen;
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
mpn_size_t mpn_to_octets(unsigned char *out, mpn_size_t outsize, const mpn_limb_t *a, mpn_size_t size)
{
    MPN_ASSERT(out != NULL);
    MPN_ASSERT(a != NULL && size != 0);

    mpn_size_t reqsize = MPN_BITS_TO_BYTES(mpn_bits_consttime(a, size));
    MPN_ASSERT(outsize >= reqsize);

    unsigned char *p = out;
    mpn_size_t left = reqsize;
    const mpn_limb_t *w = &a[size - 1];
    if ((left & (MPN_LIMB_BYTES - 1)) != 0) {
        unsigned char buff[MPN_LIMB_BYTES];
        MPN_LIMB_TO_OCTETS(buff, *w);
        mpn_size_t n = left & (MPN_LIMB_BYTES - 1);
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
mpn_size_t mpn_from_string(mpn_limb_t *r, mpn_size_t size, const char *in, mpn_size_t inlen)
{
    mpn_size_t h = 0;
    while (inlen > 0) {
        mpn_limb_t l = 0;
        mpn_size_t m = (MPN_LIMB_BYTES * 2 <= inlen) ? (MPN_LIMB_BYTES * 2) : inlen;
        for (mpn_size_t i = m; i > 0; i--) { // big-endian
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
mpn_size_t mpn_to_string(char *out, mpn_size_t outsize, const mpn_limb_t *a, mpn_size_t size)
{
    if (size == 0) {
        MPN_ASSERT(outsize >= 2);
        strcpy(out, "0");
        return 1;
    }

    char *p = out;
    unsigned notfirst = 0; /* to strip leading zeros */
    static const char HEX_CHARS[] = "0123456789ABCDEF";

    for (mpn_size_t i = size; i > 0; i--) {
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

    return (mpn_size_t)(p - out);
}
