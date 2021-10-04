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

#include <mpi/mpi.h>
#include <mpi/mpi-asm.h>

#define BITS_PER_BYTE        8                          /* @constant: bits per byte */
#define BITS_PER_CHAR        4                          /* @constant: bits per character */
#define MPI_MAX_BITS         (UINT_MAX / BITS_PER_BYTE) /* @note: mpi bits limitation */
#define MPI_BITS_TO_BYTES(n) (((n) + BITS_PER_BYTE - 1) / BITS_PER_BYTE)

/* swap variable */
#define SWAP(type, a, b) \
    do {                 \
        type __t = a;    \
        (a) = (b);       \
        (b) = __t;       \
    } while (0)

/* copy(increment) */
#define COPY(dst, src, to) \
    for (unsigned int __i = 0; __i < (to); __i++) { (dst)[__i] = (src)[__i]; }

/* expand by zeros */
#define ZEROIZE(dst, from, to) \
    for (unsigned int __i = (from); __i < (to); __i++) { (dst)[__i] = 0; }

/* copy and expand the left by zeros */
#define ZEXPAND(dst, dstlen, src, srclen)                                 \
    {                                                                     \
        unsigned int __i;                                                 \
        for (__i = 0; __i < (srclen); __i++) { (dst)[__i] = (src)[__i]; } \
        for (; __i < (dstlen); __i++) { (dst)[__i] = 0; }                 \
    }

/**
 * mpi alignment
 */
MPN_INLINE unsigned int mpi_aligned_diff(void *ptr, uintptr_t alignment)
{
    return (unsigned int)((~(((uintptr_t)ptr) & (alignment - 1)) + 1) & (alignment - 1));
}

MPN_INLINE unsigned int mpi_aligned_size(unsigned int size, unsigned int alignment)
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
MPN_INLINE void mpn_masked_copy_consttime(mpn_limb_t *dst, const mpn_limb_t *a, const mpn_limb_t *b, unsigned int len,
                                          mpn_limb_t mask)
{
    mpn_limb_t rmask = ~mask;
    for (unsigned int i = 0; i < len; i++) { dst[i] = (a[i] & mask) ^ (b[i] & rmask); }
}

/* conditional swap: a[], b[] = b[], a[] if cond; otherwise not changed */
MPN_INLINE void mpn_masked_swap_consttime(mpn_limb_t *a, mpn_limb_t *b, unsigned int n, unsigned cond)
{
    mpn_limb_t mask = cond;
    mask = ((~mask & ((mask - 1))) >> (sizeof(mpn_limb_t) * BITS_PER_BYTE - 1)) - 1;
    for (unsigned int i = 0; i < n; i++) {
        mpn_limb_t t = (a[i] ^ b[i]) & mask;
        a[i] ^= t;
        b[i] ^= t;
    }
}

/* conditional move: dst[] = cond ? src[] : dst[] */
MPN_INLINE void mpn_masked_move_consttime(mpn_limb_t *dst, const mpn_limb_t *src, unsigned int len, unsigned cond)
{
    mpn_masked_copy_consttime(dst, src, dst, len, (mpn_limb_t)0 - cond != 0);
}

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @addtogroup: mpi/low-level-api
 *
 * @berif: Low-Level-APIs for multiple-precision-computation
 */
/**
 * mpn: ALL-ones if buff[::] is zero, otherwise ALL-zeros
 */
mpn_limb_t mpn_is_zero(const mpn_limb_t *buff, unsigned int bufflen);

/**
 * mpn: ALL-ones if buff[::] is zero, otherwise ALL-zeros(constant-time version)
 */
mpn_limb_t mpn_is_zero_consttime(const mpn_limb_t *buff, unsigned int bufflen);

/**
 * mpn: get bit size of mpi
 */
unsigned int mpn_bits(const mpn_limb_t *data, unsigned int size);

/**
 * mpn: get bit size of mpi(constant-time version)
 */
unsigned int mpn_bits_consttime(const mpn_limb_t *data, unsigned int size);

/**
 * mpn: get actual size of mpi chunk
 */
unsigned int mpn_limbs(const mpn_limb_t *data, unsigned int size);

/**
 * mpn: get actual size of mpi chunk(constant-time version)
 */
unsigned int mpn_limbs_consttime(const mpn_limb_t *data, unsigned int size);

/**
 * mpn: unsigned comparison
 *
 * @note:
 *   1. return 1 if a[] > b[]; 0 if a[] = b[]; -1 if a[] < b[]
 */
int mpn_cmp(const mpn_limb_t *a, unsigned int asize, const mpn_limb_t *b, unsigned int bsize);


/**
 * mpn: left shift
 *
 * @note:
 *   1. required bit_size(r) >= bit_size(a) + nbits
 *   2. the return is number of |mpn_limb_t| of the result |r|
 *   3. r == a is acceptable
 */
unsigned int mpn_lshift(mpn_limb_t *r, const mpn_limb_t *a, unsigned int asize, unsigned int nbits);

/**
 * mpn: right shift
 *
 * @note:
 *   1. required bit_size(r) >= bit_size(a) - nbits
 *   2. the return is number of |mpn_limb_t| of the result |r|
 *   3. r == a is acceptable
 */
unsigned int mpn_rshift(mpn_limb_t *r, const mpn_limb_t *a, unsigned int asize, unsigned int nbits);

/**
 * mpn addition: carry, r = a[:n] + b[:n]
 */
mpn_limb_t mpn_add_school(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, unsigned int n);

/**
 * mpn: carry, r[] = a[] + b[]
 */
mpn_limb_t mpn_add(mpn_limb_t *r, unsigned int rroom, const mpn_limb_t *a, unsigned int asize, const mpn_limb_t *b,
                   unsigned int bsize);

/**
 * mpn: carry, r[:n] = a[:n] + w
 */
mpn_limb_t mpn_inc_school(mpn_limb_t *r, const mpn_limb_t *a, unsigned int size, mpn_limb_t w);

/**
 * mpn: carry, r[] = a[] + w
 */
mpn_limb_t mpn_inc(mpn_limb_t *r, unsigned int rroom, const mpn_limb_t *a, unsigned int asize, mpn_limb_t w);

/**
 * mpn subtraction: borrow, r[:n] = a[:n] - b[:n]
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: MAX(bit_size(a), bit_size(b)) + 1
 */
mpn_limb_t mpn_sub_school(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, unsigned int n);

/**
 * mpn subtraction: size, r[] = a[] - b[]
 */
unsigned int mpn_sub(mpn_limb_t *r, unsigned int rroom, const mpn_limb_t *a, unsigned int asize, const mpn_limb_t *b,
                     unsigned int bsize);

/**
 * mpn: borrow, r[:n] = a[:n] - w
 */
mpn_limb_t mpn_dec_school(mpn_limb_t *r, const mpn_limb_t *a, unsigned int asize, mpn_limb_t w);

/**
 * mpn: size, r[] = a[] - w
 */
unsigned int mpn_dec(mpn_limb_t *r, unsigned int rroom, const mpn_limb_t *a, unsigned int asize, mpn_limb_t w);

/**
 * mpn multiplication: extension, r[:asize+bsize] = a[:asize] * b[:bsize]
 * @note:
 *   1. (IMPORTANT)make sure size of |r| isn't less than |asize| + |bsize|
 *   2. the return is the highest unit |mpn_limb_t|
 */
mpn_limb_t mpn_mul(mpn_limb_t *r, const mpn_limb_t *a, unsigned int asize, const mpn_limb_t *b, unsigned int bsize);

/**
 * mpn multiply-and-add: extension, r[] += a[] * b
 * @note:
 *   1. (IMPORTANT)make sure size of |r| isn't less than |asize|
 *   2. the return is extension of result of multiply-and-add.
 */
mpn_limb_t mpn_mul_acc(mpn_limb_t *r, const mpn_limb_t *a, unsigned int asize, mpn_limb_t b);

/**
 * mpn square: r[] = a[] ^ 2
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: 2 * bit_size(a)
 */
mpn_limb_t mpn_sqr(mpn_limb_t *r, const mpn_limb_t *a, unsigned int anum);

/**
 * mpn division: xsize, q, x(q = x / y, x = x % y)
 */
unsigned int mpn_div(mpn_limb_t *q, unsigned int *qsize, mpn_limb_t *x, unsigned int xsize, mpn_limb_t *y,
                     unsigned int ysize);

/**
 * mpn modular: x[] = x[] % y[]
 */
unsigned int mpn_mod(mpn_limb_t *x, unsigned int xsize, mpn_limb_t *y, unsigned int ysize);

/**
 * mpn: division(n by 1)
 *
 * @note:
 *   1. required length of q should be not smaller than size
 */
unsigned int mpn_div_limb(mpn_limb_t q[], const mpn_limb_t x[], unsigned int size, mpn_limb_t *r, mpn_limb_t d);

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
unsigned int mpn_mod_invert(mpn_limb_t *r, const mpn_limb_t *a, unsigned int asize, const mpn_limb_t *m,
                            unsigned int msize, mpn_optimizer_t *optimizer);

/**
 *  mpn: create mpi from hex string
 */
unsigned int mpn_from_string(mpn_limb_t *r, unsigned int size, const char *in, unsigned int inlen);

/**
 *  mpn: convert mpi to hex string
 */
unsigned int mpn_to_string(char *out, unsigned int outsize, const mpn_limb_t *a, unsigned int size);

/**
 *  mpn: create mpi from big-endian octets
 */
unsigned int mpn_from_octets(mpn_limb_t *r, unsigned int size, const unsigned char *in, unsigned int inlen);

/**
 *  mpn: convert mpi to big-endian octets
 */
unsigned int mpn_to_octets(unsigned char *out, unsigned int outsize, const mpn_limb_t *a, unsigned int size);

/**
 * leading zeros counting(constant-time version)
 */
unsigned int mpn_limb_nlz_consttime(mpn_limb_t x);

/**
 * trailing zeros counting(constant-time version)
 */
unsigned int mpn_limb_ntz_consttime(mpn_limb_t x);

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
int mpn_random_range(mpn_limb_t *r, unsigned int maxtries, const mpn_limb_t *lo, unsigned int lolen,
                     const mpn_limb_t *hi, unsigned int hilen, int (*rand_bytes)(void *, unsigned char *, unsigned int),
                     void *rand_state);

/**
 * test if |a| and |b| are coprime
 */
int mpn_is_coprime(mpn_limb_t *a, unsigned int asize, mpn_limb_t *b, unsigned int bsize, mpn_optimizer_t *optimizer);

#if defined(__cplusplus)
}
#endif

#endif
