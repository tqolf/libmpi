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
#ifndef MULTIPLE_PRECISION_INTEGER_BINARY_H
#define MULTIPLE_PRECISION_INTEGER_BINARY_H

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
MPI_INLINE unsigned int mpi_aligned_diff(void *ptr, uintptr_t alignment)
{
    return (unsigned int)((~(((uintptr_t)ptr) & (alignment - 1)) + 1) & (alignment - 1));
}

MPI_INLINE unsigned int mpi_aligned_size(unsigned int size, unsigned int alignment)
{
    return (size + (alignment - 1)) & (-alignment);
}

MPI_INLINE mpi_limb_t *mpi_aligned_pointer(void *ptr, uintptr_t alignment)
{
    return (mpi_limb_t *)((uintptr_t)((unsigned char *)ptr + alignment - 1) & (-alignment));
}

/**
 * basic constant-time operation
 */
/* return all-ones if MSB(a) == 1; otherwise, all-zeros */
MPI_INLINE mpi_limb_t mpi_test_msb_limb_consttime(mpi_limb_t a)
{
    return (mpi_limb_t)0 - (a >> (sizeof(a) * BITS_PER_BYTE - 1));
}

/* return all-ones if |a| equals zero; otherwise, all-zeros */
MPI_INLINE mpi_limb_t mpi_is_zero_limb_consttime(mpi_limb_t a)
{
    mpi_limb_t t = ~a & (a - 1);
    return (mpi_limb_t)0 - (t >> (sizeof(t) * BITS_PER_BYTE - 1));
}

/* copy under mask: dst[] = (a[] & mask) ^ (b[] & ~mask) */
MPI_INLINE void mpi_masked_copy_consttime(mpi_limb_t *dst, const mpi_limb_t *a, const mpi_limb_t *b,
                                          unsigned int len, mpi_limb_t mask)
{
    mpi_limb_t rmask = ~mask;
    for (unsigned int i = 0; i < len; i++) { dst[i] = (a[i] & mask) ^ (b[i] & rmask); }
}

/* conditional swap: a[], b[] = b[], a[] if cond; otherwise not changed */
MPI_INLINE void mpi_masked_swap_consttime(mpi_limb_t *a, mpi_limb_t *b, unsigned int n, unsigned cond)
{
    mpi_limb_t mask = cond;
    mask = ((~mask & ((mask - 1))) >> (sizeof(mpi_limb_t) * BITS_PER_BYTE - 1)) - 1;
    for (unsigned int i = 0; i < n; i++) {
        mpi_limb_t t = (a[i] ^ b[i]) & mask;
        a[i] ^= t;
        b[i] ^= t;
    }
}

/* conditional move: dst[] = cond ? src[] : dst[] */
MPI_INLINE void mpi_masked_move_consttime(mpi_limb_t *dst, const mpi_limb_t *src, unsigned int len,
                                          unsigned cond)
{
    mpi_masked_copy_consttime(dst, src, dst, len, (mpi_limb_t)0 - cond != 0);
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
 * mpi(binary): ALL-ones if buff[::] is zero, otherwise ALL-zeros
 */
mpi_limb_t mpi_is_zero_bin(const mpi_limb_t *buff, unsigned int bufflen);

/**
 * mpi(binary): ALL-ones if buff[::] is zero, otherwise ALL-zeros(constant-time version)
 */
mpi_limb_t mpi_is_zero_consttime_bin(const mpi_limb_t *buff, unsigned int bufflen);

/**
 * mpi(binary): get bit size of mpi
 */
unsigned int mpi_bits_bin(const mpi_limb_t *data, unsigned int size);

/**
 * mpi(binary): get bit size of mpi(constant-time version)
 */
unsigned int mpi_bits_consttime_bin(const mpi_limb_t *data, unsigned int size);

/**
 * mpi(binary): get actual size of mpi chunk
 */
unsigned int mpi_fix_size_bin(const mpi_limb_t *data, unsigned int size);

/**
 * mpi(binary): get actual size of mpi chunk(constant-time version)
 */
unsigned int mpi_fix_size_consttime_bin(const mpi_limb_t *data, unsigned int size);

/**
 * mpi(binary): unsigned comparison
 *
 * @note:
 *   1. return 1 if a[] > b[]; 0 if a[] = b[]; -1 if a[] < b[]
 */
int mpi_ucmp_bin(const mpi_limb_t *a, unsigned int asize, const mpi_limb_t *b, unsigned int bsize);


/**
 * mpi(binary): left shift
 *
 * @note:
 *   1. required bit_size(r) >= bit_size(a) + nbits
 *   2. the return is number of |mpi_limb_t| of the result |r|
 *   3. r == a is acceptable
 */
unsigned int mpi_lshift_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int asize, unsigned int nbits);

/**
 * mpi(binary): right shift
 *
 * @note:
 *   1. required bit_size(r) >= bit_size(a) - nbits
 *   2. the return is number of |mpi_limb_t| of the result |r|
 *   3. r == a is acceptable
 */
unsigned int mpi_rshift_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int asize, unsigned int nbits);

/**
 * mpi(binary) addition: carry, r = a[:n] + b[:n]
 */
mpi_limb_t mpi_uadd_school_bin(mpi_limb_t *r, const mpi_limb_t *a, const mpi_limb_t *b, unsigned int n);

/**
 * mpi(binary): carry, r[] = a[] + b[]
 */
mpi_limb_t mpi_uadd_bin(mpi_limb_t *r, unsigned int rroom, const mpi_limb_t *a, unsigned int asize,
                        const mpi_limb_t *b, unsigned int bsize);

/**
 * mpi(binary): carry, r[:n] = a[:n] + w
 */
mpi_limb_t mpi_uinc_school_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int size, mpi_limb_t w);

/**
 * mpi(binary): carry, r[] = a[] + w
 */
mpi_limb_t mpi_uinc_bin(mpi_limb_t *r, unsigned int rroom, const mpi_limb_t *a, unsigned int asize,
                        mpi_limb_t w);

/**
 * mpi(binary) subtraction: borrow, r[:n] = a[:n] - b[:n]
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: MAX(bit_size(a), bit_size(b)) + 1
 */
mpi_limb_t mpi_usub_school_bin(mpi_limb_t *r, const mpi_limb_t *a, const mpi_limb_t *b, unsigned int n);

/**
 * mpi(binary) subtraction: size, r[] = a[] - b[]
 */
unsigned int mpi_usub_bin(mpi_limb_t *r, unsigned int rroom, const mpi_limb_t *a, unsigned int asize,
                          const mpi_limb_t *b, unsigned int bsize);

/**
 * mpi(binary): borrow, r[:n] = a[:n] - w
 */
mpi_limb_t mpi_udec_school_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int asize, mpi_limb_t w);

/**
 * mpi(binary): size, r[] = a[] - w
 */
unsigned int mpi_udec_bin(mpi_limb_t *r, unsigned int rroom, const mpi_limb_t *a, unsigned int asize,
                          mpi_limb_t w);

/**
 * mpi(binary) multiplication: extension, r[:asize+bsize] = a[:asize] * b[:bsize]
 * @note:
 *   1. (IMPORTANT)make sure size of |r| isn't less than |asize| + |bsize|
 *   2. the return is the highest unit |mpi_limb_t|
 */
mpi_limb_t mpi_umul_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int asize, const mpi_limb_t *b,
                        unsigned int bsize);

/**
 * mpi(binary) multiply-and-add: extension, r[] += a[] * b
 * @note:
 *   1. (IMPORTANT)make sure size of |r| isn't less than |asize|
 *   2. the return is extension of result of multiply-and-add.
 */
mpi_limb_t mpi_umul_acc_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int asize, mpi_limb_t b);

/**
 * mpi(binary) square: r[] = a[] ^ 2
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: 2 * bit_size(a)
 */
mpi_limb_t mpi_usqr_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int anum);

/**
 * mpi(binary) division: xsize, q, x(q = x / y, x = x % y)
 */
unsigned int mpi_udiv_bin(mpi_limb_t *q, unsigned int *qsize, mpi_limb_t *x, unsigned int xsize,
                          mpi_limb_t *y, unsigned int ysize);

/**
 * mpi(binary) modular: x[] = x[] % y[]
 */
unsigned int mpi_umod_bin(mpi_limb_t *x, unsigned int xsize, mpi_limb_t *y, unsigned int ysize);

/**
 * mpi(binary): division(n by 1)
 *
 * @note:
 *   1. required length of q should be not smaller than size
 */
unsigned int mpi_udiv_limb_bin(mpi_limb_t q[], const mpi_limb_t x[], unsigned int size, mpi_limb_t *r,
                               mpi_limb_t d);

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
unsigned int mpi_umod_inv_bin(mpi_limb_t *r, const mpi_limb_t *a, unsigned int asize, const mpi_limb_t *m,
                              unsigned int msize, mpi_optimizer_t *optimizer);

/**
 *  mpi(binary): create mpi from hex string
 */
unsigned int mpi_from_string_bin(mpi_limb_t *r, unsigned int size, const char *in, unsigned int inlen);

/**
 *  mpi(binary): convert mpi to hex string
 */
unsigned int mpi_to_string_bin(char *out, unsigned int outsize, const mpi_limb_t *a, unsigned int size);

/**
 *  mpi(binary): create mpi from big-endian octets
 */
unsigned int mpi_from_octets_bin(mpi_limb_t *r, unsigned int size, const unsigned char *in,
                                 unsigned int inlen);

/**
 *  mpi(binary): convert mpi to big-endian octets
 */
unsigned int mpi_to_octets_bin(unsigned char *out, unsigned int outsize, const mpi_limb_t *a,
                               unsigned int size);

/**
 * leading zeros counting(constant-time version)
 */
unsigned int mpi_nlz_limb_consttime(mpi_limb_t x);

/**
 * trailing zeros counting(constant-time version)
 */
unsigned int mpi_ntz_limb_consttime(mpi_limb_t x);

/**
 * greatest common divisor(mpi_limb_t)
 */
mpi_limb_t mpi_gcd_limb(mpi_limb_t a, mpi_limb_t b);

/**
 * mpi(binary): generate in range
 *
 * @note:
 *   1. length of |r| >= hilen
 */
int mpi_random_range_bin(mpi_limb_t *r, unsigned int maxtries, const mpi_limb_t *lo, unsigned int lolen,
                         const mpi_limb_t *hi, unsigned int hilen,
                         int (*rand_bytes)(void *, unsigned char *, unsigned int), void *rand_state);

/**
 * test if |a| and |b| are coprime
 */
int mpi_is_coprime_bin(mpi_limb_t *a, unsigned int asize, mpi_limb_t *b, unsigned int bsize,
                       mpi_optimizer_t *optimizer);

#if defined(__cplusplus)
}
#endif

#endif
