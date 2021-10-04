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
#ifndef MULTIPLE_PRECISION_CONFIGURATION_H
#define MULTIPLE_PRECISION_CONFIGURATION_H

#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <inttypes.h>

/* memory allocator */
#ifdef MPI_DEBUG_MEMORY
#include <devkit/memleak.h>
#define MPI_ALLOCATE(size)                                                  \
    ({                                                                      \
        void *__ptr = malloc(size);                                         \
        MPI_DEBUG_ALLOCATOR_ON_ALLOCATE(__ptr, (size), __FILE__, __LINE__); \
        __ptr;                                                              \
    }) /* allocate memory chunk */
#define MPI_ZALLOCATE(n, size)                                                    \
    ({                                                                            \
        void *__ptr = calloc((n), (size));                                        \
        MPI_DEBUG_ALLOCATOR_ON_ZALLOCATE(__ptr, (n), (size), __FILE__, __LINE__); \
        __ptr;                                                                    \
    }) /* allocate memory chunk, then zeroize */
#define MPI_DEALLOCATE(ptr)                                           \
    {                                                                 \
        free((ptr));                                                  \
        MPI_DEBUG_ALLOCATOR_ON_DEALLOCATE((ptr), __FILE__, __LINE__); \
    } /* de-allocate memory chunk */
#define MPI_REALLOCATE(ptr, size)                                                                    \
    ({                                                                                               \
        void *__ptr = realloc((ptr), (size));                                                        \
        MPI_DEBUG_ALLOCATOR_ON_REALLOCATE(__ptr, (ptr), (size), __FILE__, __LINE__);                 \
        __ptr;                                                                                       \
    }) /* (OPTIONAL) re-allocate memory chunk. NOT use |MPI_REALLOCATE| to expand mpn room if not be \
          defined */
#else
#define MPI_ALLOCATE(size)        malloc(size)           /* allocate memory chunk */
#define MPI_ZALLOCATE(n, size)    calloc((n), (size))    /* allocate memory chunk, then zeroize */
#define MPI_DEALLOCATE(ptr)       free(ptr)              /* de-allocate memory chunk */
#define MPI_REALLOCATE(ptr, size) realloc((ptr), (size)) /* (OPTIONAL) re-allocate memory chunk */
#endif

// clang-format off
// #define MPN_LIMB_BITS                32 /* undefine to detect automatically */
// #define MPN_NO_INLINE_ASM            /* NOT use inline asm, @see https://gcc.gnu.org/wiki/DontUseInlineAsm */
// #define MPI_CACHE_LINE_BYTES         64 /* size of cache line (in bytes) */
// #define MPI_LOW_FOOTPRINT            /* optimize the static memory footprint of the library */
// #define MPI_USE_SLIDING_WINDOW_EXP   /* sliding-windows exponentiation */
// #define MPI_USE_C_MONTGOMERY_MUL_BIN /* use c implementation for mpn_montgomery_mul */
// #define MPI_USE_C_MONTGOMERY_RED_BIN /* use c implementation for mpn_montgomery_reduce */
// clang-format on

/** automatically detection for some known platforms */
#ifndef MPN_LIMB_BITS
#if defined(__x86_64) || defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64) || defined(__aarch64__) \
    || defined(__powerpc64__) || defined(_ARCH_PP64)
#define MPN_LIMB_BITS 64
#elif defined(__x86) || defined(__i386) || defined(__i386__) || defined(_M_IX86) || defined(__arm) || defined(__arm__) \
    || defined(_M_ARM)
#define MPN_LIMB_BITS 32
#endif
#endif

#ifndef MPN_NAIL_BITS
#define MPN_NAIL_BITS 0
#endif

/** inline */
#if defined(__cplusplus)
#define MPN_INLINE inline
#else
#define MPN_INLINE static inline
#endif

/** assertion and static assertion */
#ifndef MPN_ASSERT
#define MPN_ASSERT(cond)                                                      \
    if (!(cond)) {                                                            \
        printf("Assertion Failed: " #cond ", @ %s:%d\n", __FILE__, __LINE__); \
        MPI_RAISE_EXCEPTION();                                                \
    }
#endif

#ifndef MPN_ASSERT_ALWAYS
#define MPN_ASSERT_ALWAYS(cond)                                               \
    if (!(cond)) {                                                            \
        printf("Assertion Failed: " #cond ", @ %s:%d\n", __FILE__, __LINE__); \
        MPI_RAISE_EXCEPTION();                                                \
    }
#endif

/* Check that the nail parts are zero. */
#define ASSERT_ALWAYS_LIMB(limb)                  \
    do {                                          \
        mpn_limb_t __nail = (limb)&MPN_NAIL_MASK; \
        MPN_ASSERT_ALWAYS(__nail == 0);           \
    } while (0)

#define ASSERT_ALWAYS_MPN(ptr, size)                                                          \
    do {                                                                                      \
        /* let whole loop go dead when no nails */                                            \
        if (MPN_NAIL_BITS != 0) {                                                             \
            for (mpn_size_t __i = 0; __i < (size); __i++) { ASSERT_ALWAYS_LIMB((ptr)[__i]); } \
        }                                                                                     \
    } while (0)

#define MPN_OVERLAP_P(xp, xsize, yp, ysize) ((xp) + (xsize) > (yp) && (yp) + (ysize) > (xp))

#define ASSERT_MPN ASSERT_ALWAYS_MPN

#if defined(__cplusplus) || (defined(_MSC_VER) && !defined(__clang__))
#define MPN_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#else
#define MPN_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif

/* branch prediction */
#if (defined(__GNUC__) && (__GNUC__ >= 2)) || defined(__clang__)
#define LIKELY(x)           __builtin_expect(!!(x), 1)
#define UNLIKELY(x)         __builtin_expect(!!(x), 0)
#define BUILTIN_CONSTANT(c) __builtin_constant_p(c)
#else
#define LIKELY(x)           (x)
#define UNLIKELY(x)         (x)
#define BUILTIN_CONSTANT(c) (c)
#endif

#define MPN_ABOVE_THRESHOLD(size, thresh)        \
    ((BUILTIN_CONSTANT(thresh) && (thresh) == 0) \
     || (!(BUILTIN_CONSTANT(thresh) && (thresh) == ~((size_t)0)) && (size) >= (thresh)))
#define MPN_BELOW_THRESHOLD(size, thresh) (!MPN_ABOVE_THRESHOLD(size, thresh))


/** debug macros */
// clang-format off
#define MPI_NARG(...)  MPI_NARG_(, ##__VA_ARGS__, MPI_RSEQ_N())
#define MPI_NARG_(...) MPI_ARG_N(__VA_ARGS__)
#define MPI_ARG_N(                                                       \
        _0,  _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9, _10, _11, _12,  \
        _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, \
        _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, \
        _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, \
        _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63,   N, \
        ...) N
#define MPI_RSEQ_N()                                                     \
         63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, \
         47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, \
         31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, \
         15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0
// clang-format on

#if (defined(__GNUC__) && (__GNUC__ >= 2)) || defined(__clang__)
#define MPI_HAVE_ARGS(...) __builtin_constant_p(MPI_NARG(__VA_ARGS__))
#else
#define MPI_HAVE_ARGS(...) MPI_NARG(__VA_ARGS__)
#endif

MPN_INLINE void mpi_raise_exception(void)
{
#ifdef SIGFPE
    raise(SIGFPE);
#endif
    abort();
}
#define MPI_RAISE_WARN(...) printf("%s:%d: ", __FILE__, __LINE__), printf(__VA_ARGS__), printf("\n")
#define MPI_RAISE_ERROR(errno, ...)                                                             \
    if (MPI_HAVE_ARGS(__VA_ARGS__)) {                                                           \
        printf("%s:%d: %d", __FILE__, __LINE__, errno), printf(", " __VA_ARGS__), printf("\n"); \
    } else {                                                                                    \
        printf("%s:%d: %d\n", __FILE__, __LINE__, errno);                                       \
    }
#define MPI_RAISE_EXCEPTION() mpi_raise_exception()

#if (defined MPI_DEBUG_VERBOSE)
#define MPI_TRACE(...)                                                               \
    if (MPI_HAVE_ARGS(__VA_ARGS__)) {                                                \
        printf("%s:%d:", __FILE__, __LINE__), printf(" " __VA_ARGS__), printf("\n"); \
    } else {                                                                         \
        printf("%s:%d\n", __FILE__, __LINE__);                                       \
    }
#else
#define MPI_TRACE(...)
#endif

/** definations from configurations, NEVER modify it */
#if MPN_LIMB_BITS == 64
typedef int64_t mpn_slimb_t;
typedef uint64_t mpn_limb_t;
#define MPN_LIMB_BYTES   8 // sizeof(mpn_limb_t)
#define MPN_LIMB_FORMAT  PRIu64
#define MPN_LIMB_MASK    0xFFFFFFFFFFFFFFFF
#define MPN_LIMB_MASK_LO 0x00000000FFFFFFFF
#define MPN_LIMB_MASK_HI 0xFFFFFFFF00000000

MPN_INLINE void MPN_LIMB_TO_OCTETS(unsigned char *p, mpn_limb_t v)
{
    p[0] = (unsigned char)(v >> 56);
    p[1] = (unsigned char)(v >> 48);
    p[2] = (unsigned char)(v >> 40);
    p[3] = (unsigned char)(v >> 32);
    p[4] = (unsigned char)(v >> 24);
    p[5] = (unsigned char)(v >> 16);
    p[6] = (unsigned char)(v >> 8);
    p[7] = (unsigned char)(v >> 0);
}
#elif MPN_LIMB_BITS == 32
typedef int32_t mpn_slimb_t;
typedef uint32_t mpn_limb_t;
#define MPN_LIMB_BYTES   4 // sizeof(mpn_limb_t)
#define MPN_LIMB_FORMAT  PRIu32
#define MPN_LIMB_MASK    0xFFFFFFFF
#define MPN_LIMB_MASK_LO 0x0000FFFF
#define MPN_LIMB_MASK_HI 0xFFFF0000

MPN_INLINE void MPN_LIMB_TO_OCTETS(unsigned char *p, mpn_limb_t v)
{
    p[0] = (unsigned char)(v >> 24);
    p[1] = (unsigned char)(v >> 16);
    p[2] = (unsigned char)(v >> 8);
    p[3] = (unsigned char)(v >> 0);
}
#else
#error MPN_LIMB_BITS MUST bed defined first.
#endif

typedef unsigned int mpn_size_t;

#define MPN_NUMB_BITS    (MPN_LIMB_BITS - MPN_NAIL_BITS)
#define MPN_NUMB_MASK    ((~CNST_LIMB(0)) >> MPN_NAIL_BITS)
#define MPN_NAIL_MASK    (~MPN_NUMB_MASK)
#define MPN_LIMB_HIGHBIT (CNST_LIMB(1) << (MPN_LIMB_BITS - 1))
#define MPN_NUMB_HIGHBIT (CNST_LIMB(1) << (MPN_NUMB_BITS - 1))

/**
 * Configuration checks: NEVER modify this
 */
MPN_STATIC_ASSERT(sizeof(mpn_limb_t) == MPN_LIMB_BYTES, "mpn_limb_t MUST be MPN_LIMB_BYTES bytes");
MPN_STATIC_ASSERT(~(mpn_limb_t)0 == MPN_LIMB_MASK, "~(mpn_limb_t)0 MUST equals to MPN_LIMB_MASK");


MPN_INLINE unsigned int MPN_BITS_TO_LIMBS(unsigned int bits)
{
    return (bits + MPN_LIMB_BITS - 1) / MPN_LIMB_BITS;
}

/* mask for [lo, hi), (hi - lo) bits */
MPN_INLINE mpn_limb_t MPN_MASK_LIMB(unsigned int lo, unsigned int hi)
{
    MPN_ASSERT(lo <= hi);
    MPN_ASSERT(lo < MPN_LIMB_BITS && hi <= MPN_LIMB_BITS);

    return ~(MPN_LIMB_MASK << hi) & (MPN_LIMB_MASK << lo);
}

/* mask HI nbits */
MPN_INLINE mpn_limb_t MPN_MASK_LIMB_HI(unsigned int nbits)
{
    return ((mpn_limb_t)(-1) >> ((MPN_LIMB_BITS - (nbits & (MPN_LIMB_BITS - 1))) & (MPN_LIMB_BITS - 1)));
}

MPN_INLINE mpn_limb_t MPN_MASK_LIMB_LO(unsigned int nbits)
{
    return MPN_LIMB_MASK >> (MPN_LIMB_BITS - (nbits & (MPN_LIMB_BITS - 1)));
}

#endif
