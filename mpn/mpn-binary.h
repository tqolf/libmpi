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
#ifndef MULTIPLE_PRECISION_BINARY_H
#define MULTIPLE_PRECISION_BINARY_H

#include <mpn/mpn-asm.h>
#include <mpn/mpn-optimizer.h>

#define BITS_PER_BYTE        8                          /* @constant: bits per byte */
#define BITS_PER_CHAR        4                          /* @constant: bits per character */
#define MPN_MAX_BITS         (UINT_MAX / BITS_PER_BYTE) /* @note: mpn width limitation */
#define MPN_BITS_TO_BYTES(n) (((n) + BITS_PER_BYTE - 1) / BITS_PER_BYTE)

/* swap variable */
#define SWAP(type, a, b) \
    do {                 \
        type __t = a;    \
        (a) = (b);       \
        (b) = __t;       \
    } while (0)

/* copy(increment) */
#define COPY(dst, src, to) \
    for (mpn_size_t __i = 0; __i < (to); __i++) { (dst)[__i] = (src)[__i]; }

/* expand by zeros */
#define ZEROIZE(dst, from, to) \
    for (mpn_size_t __i = (from); __i < (to); __i++) { (dst)[__i] = 0; }

/* copy and expand the left by zeros */
#define ZEXPAND(dst, dstlen, src, srclen)                                 \
    {                                                                     \
        mpn_size_t __i;                                                   \
        for (__i = 0; __i < (srclen); __i++) { (dst)[__i] = (src)[__i]; } \
        for (; __i < (dstlen); __i++) { (dst)[__i] = 0; }                 \
    }

/**
 * mpn alignment
 */
MPN_INLINE mpn_size_t mpi_aligned_diff(void *ptr, uintptr_t alignment)
{
    return (mpn_size_t)((~(((uintptr_t)ptr) & (alignment - 1)) + 1) & (alignment - 1));
}

MPN_INLINE mpn_size_t mpi_aligned_size(mpn_size_t size, mpn_size_t alignment)
{
    return (size + (alignment - 1)) & (-alignment);
}

MPN_INLINE mpn_limb_t *mpi_aligned_pointer(void *ptr, uintptr_t alignment)
{
    return (mpn_limb_t *)((uintptr_t)((unsigned char *)ptr + alignment - 1) & (-alignment));
}

/**
 * basic constant-time operation
 */
/* return all-ones if MSB(a) == 1; otherwise, all-zeros */
MPN_INLINE mpn_limb_t mpn_limb_test_msb_consttime(mpn_limb_t a)
{
    return (mpn_limb_t)0 - (a >> (sizeof(a) * BITS_PER_BYTE - 1));
}

/* return all-ones if |a| equals zero; otherwise, all-zeros */
MPN_INLINE mpn_limb_t mpn_limb_is_zero_consttime(mpn_limb_t a)
{
    mpn_limb_t t = ~a & (a - 1);
    return (mpn_limb_t)0 - (t >> (sizeof(t) * BITS_PER_BYTE - 1));
}

/* copy under mask: dst[] = (a[] & mask) ^ (b[] & ~mask) */
MPN_INLINE void mpn_masked_copy_consttime(mpn_limb_t *dst, const mpn_limb_t *a, const mpn_limb_t *b, mpn_size_t len,
                                          mpn_limb_t mask)
{
    mpn_limb_t rmask = ~mask;
    for (mpn_size_t i = 0; i < len; i++) { dst[i] = (a[i] & mask) ^ (b[i] & rmask); }
}

/* conditional swap: a[], b[] = b[], a[] if cond; otherwise not changed */
MPN_INLINE void mpn_masked_swap_consttime(mpn_limb_t *a, mpn_limb_t *b, mpn_size_t n, unsigned cond)
{
    mpn_limb_t mask = cond;
    mask = ((~mask & ((mask - 1))) >> (sizeof(mpn_limb_t) * BITS_PER_BYTE - 1)) - 1;
    for (mpn_size_t i = 0; i < n; i++) {
        mpn_limb_t t = (a[i] ^ b[i]) & mask;
        a[i] ^= t;
        b[i] ^= t;
    }
}

/* conditional move: dst[] = cond ? src[] : dst[] */
MPN_INLINE void mpn_masked_move_consttime(mpn_limb_t *dst, const mpn_limb_t *src, mpn_size_t len, unsigned cond)
{
    mpn_masked_copy_consttime(dst, src, dst, len, (mpn_limb_t)0 - cond != 0);
}

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * mpn: ALL-ones if buff[::] is zero, otherwise ALL-zeros
 */
mpn_limb_t mpn_is_zero(const mpn_limb_t *buff, mpn_size_t bufflen);

/**
 * mpn: ALL-ones if buff[::] is zero, otherwise ALL-zeros(constant-time version)
 */
mpn_limb_t mpn_is_zero_consttime(const mpn_limb_t *buff, mpn_size_t bufflen);

/**
 * mpn: get most significant bit
 */
mpn_size_t mpn_bits(const mpn_limb_t *data, mpn_size_t size);

/**
 * mpn: get most significant bit(constant-time version)
 */
mpn_size_t mpn_bits_consttime(const mpn_limb_t *data, mpn_size_t size);

/**
 * mpn: get most significant limb
 */
mpn_size_t mpn_limbs(const mpn_limb_t *data, mpn_size_t size);

/**
 * mpn: get most significant limb(constant-time version)
 */
mpn_size_t mpn_limbs_consttime(const mpn_limb_t *data, mpn_size_t size);

/**
 * mpn: unsigned comparison
 *
 * @note:
 *   1. return 1 if a[] > b[]; 0 if a[] = b[]; -1 if a[] < b[]
 */
int mpn_cmp(const mpn_limb_t *a, mpn_size_t asize, const mpn_limb_t *b, mpn_size_t bsize);


/**
 * mpn: left shift
 *
 * @note:
 *   1. required bit_size(r) >= bit_size(a) + nbits
 *   2. the return is number of |mpn_limb_t| of the result |r|
 *   3. r == a is acceptable
 */
mpn_size_t mpn_lshift(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t asize, mpn_size_t nbits);

/**
 * mpn: right shift
 *
 * @note:
 *   1. required bit_size(r) >= bit_size(a) - nbits
 *   2. the return is number of |mpn_limb_t| of the result |r|
 *   3. r == a is acceptable
 */
mpn_size_t mpn_rshift(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t asize, mpn_size_t nbits);

/**
 * mpn addition: carry, r = a[:n] + b[:n]
 */
mpn_limb_t mpn_add_vectorized(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, mpn_size_t n);

/**
 * mpn: carry, r[] = a[] + b[]
 */
mpn_limb_t mpn_add(mpn_limb_t *r, mpn_size_t rroom, const mpn_limb_t *a, mpn_size_t asize, const mpn_limb_t *b,
                   mpn_size_t bsize);

/**
 * mpn: carry, r[:n] = a[:n] + w
 */
mpn_limb_t mpn_inc_vectorized(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t size, mpn_limb_t w);

/**
 * mpn: carry, r[] = a[] + w
 */
mpn_limb_t mpn_inc(mpn_limb_t *r, mpn_size_t rroom, const mpn_limb_t *a, mpn_size_t asize, mpn_limb_t w);

/**
 * mpn subtraction: borrow, r[:n] = a[:n] - b[:n]
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: MAX(bit_size(a), bit_size(b)) + 1
 */
mpn_limb_t mpn_sub_vectorized(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, mpn_size_t n);

/**
 * mpn subtraction: size, r[] = a[] - b[]
 */
mpn_size_t mpn_sub(mpn_limb_t *r, mpn_size_t rroom, const mpn_limb_t *a, mpn_size_t asize, const mpn_limb_t *b,
                   mpn_size_t bsize);

/**
 * mpn: borrow, r[:n] = a[:n] - w
 */
mpn_limb_t mpn_dec_vectorized(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t asize, mpn_limb_t w);

/**
 * mpn: size, r[] = a[] - w
 */
mpn_size_t mpn_dec(mpn_limb_t *r, mpn_size_t rroom, const mpn_limb_t *a, mpn_size_t asize, mpn_limb_t w);

/**
 * mpn multiplication: extension, r[:asize+bsize] = a[:asize] * b[:bsize]
 * @note:
 *   1. (IMPORTANT)make sure size of |r| isn't less than |asize| + |bsize|
 *   2. the return is the highest unit |mpn_limb_t|
 */
mpn_limb_t mpn_mul(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t asize, const mpn_limb_t *b, mpn_size_t bsize);

/**
 * mpn multiply-and-add: extension, r[] += a[] * b
 * @note:
 *   1. (IMPORTANT)make sure size of |r| isn't less than |asize|
 *   2. the return is extension of result of multiply-and-add.
 */
mpn_limb_t mpn_mul_acc(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t asize, mpn_limb_t b);

/**
 * mpn square: r[] = a[] ^ 2
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: 2 * bit_size(a)
 */
mpn_limb_t mpn_sqr(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t anum);

/**
 * mpn division: xsize, q, x(q = x / y, x = x % y)
 */
mpn_size_t mpn_div(mpn_limb_t *q, mpn_size_t *qsize, mpn_limb_t *x, mpn_size_t xsize, mpn_limb_t *y, mpn_size_t ysize);

/**
 * mpn modular: x[] = x[] % y[]
 */
mpn_size_t mpn_mod(mpn_limb_t *x, mpn_size_t xsize, mpn_limb_t *y, mpn_size_t ysize);

/**
 * mpn: division(n by 1)
 *
 * @note:
 *   1. required length of q should be not smaller than size
 */
mpn_size_t mpn_div_limb(mpn_limb_t q[], const mpn_limb_t x[], mpn_size_t size, mpn_limb_t *r, mpn_limb_t d);

/**
 * mpn: division(n by 2)
 *
 * @note:
 *   1. required length of q should be not smaller than size
 */
mpn_size_t mpn_div_double_limbs(mpn_limb_t q[], mpn_limb_t r[2], const mpn_limb_t n[], mpn_size_t nn,
                                const mpn_limb_t d[2]);

/**
 * @brief: multiplicative inversion
 *
 * @params:
 *   a/asize: source (value) BigNum A whose size is asize
 *   m/msize: source (modulus) BigNum M whose size is msize
 *   invbuf: buffer of inv
 *   abuf  : buffer of A
 *   mbuf  : buffer of M
 *   r  : result BigNum
 */
mpn_size_t mpn_mod_invert(mpn_limb_t *r, const mpn_limb_t *a, mpn_size_t asize, const mpn_limb_t *m, mpn_size_t msize,
                          mpn_optimizer_t *optimizer);

/**
 *  mpn: create mpn from hex string
 */
mpn_size_t mpn_from_string(mpn_limb_t *r, mpn_size_t size, const char *in, mpn_size_t inlen);

/**
 *  mpn: convert mpn to hex string
 */
mpn_size_t mpn_to_string(char *out, mpn_size_t outsize, const mpn_limb_t *a, mpn_size_t size);

/**
 *  mpn: create mpn from big-endian octets
 */
mpn_size_t mpn_from_octets(mpn_limb_t *r, mpn_size_t size, const unsigned char *in, mpn_size_t inlen);

/**
 *  mpn: convert mpn to big-endian octets
 */
mpn_size_t mpn_to_octets(unsigned char *out, mpn_size_t outsize, const mpn_limb_t *a, mpn_size_t size);

/**
 * leading zeros counting(constant-time version)
 */
mpn_size_t mpn_limb_nlz_consttime(mpn_limb_t x);

/**
 * trailing zeros counting(constant-time version)
 */
mpn_size_t mpn_limb_ntz_consttime(mpn_limb_t x);

/**
 * greatest common divisor(mpn_limb_t)
 */
mpn_limb_t mpn_limb_gcd(mpn_limb_t a, mpn_limb_t b);

/**
 * mpn: generate in range
 *
 * @note:
 *   1. length of |r| >= hilen
 */
int mpn_random_range(mpn_limb_t *r, mpn_size_t maxtries, const mpn_limb_t *lo, mpn_size_t lolen, const mpn_limb_t *hi,
                     mpn_size_t hilen, int (*rand_bytes)(void *, unsigned char *, mpn_size_t), void *rand_state);

/**
 * test if |a| and |b| are coprime
 */
int mpn_is_coprime(mpn_limb_t *a, mpn_size_t asize, mpn_limb_t *b, mpn_size_t bsize, mpn_optimizer_t *optimizer);

#if defined(__cplusplus)
}
#endif

#endif
