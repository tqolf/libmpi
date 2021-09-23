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
/**
 * definitions for mixed size 32/64 bit arithmetic.
 *
 */

#ifndef MULTIPLE_PRECISION_INTEGER_ASM_H
#define MULTIPLE_PRECISION_INTEGER_ASM_H

#include <mpi/mpi.h>

#if (defined(__GNUC__) && (__GNUC__ >= 2)) || defined(__clang__)
#define LIKELY(x)           __builtin_expect(!!(x), 1)
#define UNLIKELY(x)         __builtin_expect(!!(x), 0)
#define BUILTIN_CONSTANT(c) __builtin_constant_p(c)
#else
#define LIKELY(x)           (x)
#define UNLIKELY(x)         (x)
#define BUILTIN_CONSTANT(c) (c)
#endif

/**
 * Define auxiliary asm macros.
 *   1. UADD_AB(carry, sum, addend_1, addend_2)
 *   2. UADD_ABC(carry, sum, addend_1, addend_2, addend_3)
 *   3. USUB_AB(borrow, difference, minuend, subtrahend)
 *   4. USUB_ABC(borrow, difference, minuend, subtrahend_1, subtrahend_2)
 *
 *   5. UADD_AABB(high_sum, low_sum, high_addend_1, low_addend_1,
 *   high_addend_2, low_addend_2) adds two mpi_limb_t integers, composed by
 *   HIGH_ADDEND_1 and LOW_ADDEND_1, and HIGH_ADDEND_2 and LOW_ADDEND_2
 *   respectively.  The result is placed in HIGH_SUM and LOW_SUM. Overflow
 *   (i.e. carry out) is not stored anywhere, and is lost.
 *
 *   6. USUB_AABB(high_difference, low_difference, high_minuend, low_minuend,
 *   high_subtrahend, low_subtrahend) subtracts two two-word mpi_limb_t integers,
 *   composed by HIGH_MINUEND_1 and LOW_MINUEND_1, and HIGH_SUBTRAHEND_2 and
 *   LOW_SUBTRAHEND_2 respectively.  The result is placed in HIGH_DIFFERENCE
 *   and LOW_DIFFERENCE.  Overflow (i.e. carry out) is not stored anywhere,
 *   and is lost.
 *
 *   7. UMUL_AB(high_prod, low_prod, multiplier, multiplicand) multiplies two
 *   mpi_limb_t integers MULTIPLIER and MULTIPLICAND, and generates a two mpi_limb_t
 *   word product in HIGH_PROD and LOW_PROD.
 *
 *   8. UDIV_NND(quotient, remainder, high_numerator, low_numerator,
 *   denominator) divides a (twice as large a mpi_limb_t), composed by the mpi_limb_t integers
 *   HIGH_NUMERATOR and LOW_NUMERATOR, by DENOMINATOR and places the quotient
 *   in QUOTIENT and the remainder in REMAINDER.  HIGH_NUMERATOR must be less
 *   than DENOMINATOR for correct operation. If, in addition, the most
 *   significant bit of DENOMINATOR must be 1, then the pre-processor symbol
 *   UDIV_NEEDS_NORMALIZATION is defined to 1.
 *
 *   9. COUNT_LEADING_ZEROS(count, x) counts the number of zero-bits from the
 *   msb to the first non-zero bit in the mpi_limb_t X.  This is the number of
 *   steps X needs to be shifted left to set the msb.  Undefined for X == 0,
 *   unless the symbol COUNT_LEADING_ZEROS_0 is defined to some value.
 *
 *   10. COUNT_TRAILING_ZEROS(count, x) like COUNT_LEADING_ZEROS, but counts
 *   from the least significant end.
 *
 *
 *   If any of these macros are left undefined for a particular CPU,
 *   C macros are used.
 *
 *
 *   Notes:
 *
 *   For UADD_AABB the two high and two low addends can both commute, but
 *   unfortunately gcc only supports one "%" commutative in each asm block.
 *   This has always been so but is only documented in recent versions
 *   (eg. pre-release 3.3).  Having two or more "%"s can cause an internal
 *   compiler error in certain rare circumstances.
 *
 *   Apparently it was only the last "%" that was ever actually respected, so
 *   the code has been updated to leave just that.  Clearly there's a free
 *   choice whether high or low should get it, if there's a reason to favour
 *   one over the other.  Also obviously when the constraints on the two
 *   operands are identical there's no benefit to the reloader in any "%" at
 *   all.
 *
 */

#define BITS_S4    (MPI_LIMB_BITS / 4)
#define UINT_H1    ((mpi_limb_t)1 << (MPI_LIMB_BITS / 2))
#define UINT_LO(t) ((mpi_limb_t)(t) & (UINT_H1 - 1))
#define UINT_HI(t) ((mpi_limb_t)(t) >> (MPI_LIMB_BITS / 2))

#ifdef MPI_LIMB_LONG_LONG
#define COUNT_LEADING_ZEROS_BUILTIN(count, x)       \
    do {                                            \
        MPI_ASSERT((x) != 0);                       \
        (count) = (unsigned int)__builtin_clzll(x); \
    } while (0)
#else
#define COUNT_LEADING_ZEROS_BUILTIN(count, x)      \
    do {                                           \
        MPI_ASSERT((x) != 0);                      \
        (count) = (unsigned int)__builtin_clzl(x); \
    } while (0)
#endif

#ifdef MPI_LIMB_LONG_LONG
#define COUNT_TRAILING_ZEROS_BUILTIN(count, x)      \
    do {                                            \
        MPI_ASSERT((x) != 0);                       \
        (count) = (unsigned int)__builtin_ctzll(x); \
    } while (0)
#else
#define COUNT_TRAILING_ZEROS_BUILTIN(count, x)     \
    do {                                           \
        MPI_ASSERT((x) != 0);                      \
        (count) = (unsigned int)__builtin_ctzl(x); \
    } while (0)
#endif

#if !defined(MPI_NO_INLINE_ASM) && defined(__GNUC__) && (__GNUC__ >= 2)

#if defined(__arm__) && (defined(__thumb2__) || !defined(__thumb__)) && MPI_LIMB_BITS == 32
#define UADD_AABB(sh, sl, ah, al, bh, bl)                                \
    do {                                                                 \
        if (BUILTIN_CONSTANT(bl) && -(uint32_t)(bl) < (uint32_t)(bl)) {  \
            __asm__("subs\t%1, %4, %5\n\tadc\t%0, %2, %3"                \
                    : "=r"(sh), "=&r"(sl)                                \
                    : "r"(ah), "rI"(bh), "%r"(al), "rI"(-(uint32_t)(bl)) \
                    : "cc");                                             \
        } else {                                                         \
            __asm__("adds\t%1, %4, %5\n\tadc\t%0, %2, %3"                \
                    : "=r"(sh), "=&r"(sl)                                \
                    : "r"(ah), "rI"(bh), "%r"(al), "rI"(bl)              \
                    : "cc");                                             \
        }                                                                \
    } while (0)

/* FIXME: Extend the immediate range for the low word by using both ADDS and
   SUBS, since they set carry in the same way.  We need separate definitions
   for thumb and non-thumb since thumb lacks RSC.  */
#if defined(__thumb__)
#define USUB_AABB(sh, sl, ah, al, bh, bl)                                   \
    do {                                                                    \
        if (BUILTIN_CONSTANT(ah) && BUILTIN_CONSTANT(bh) && (ah) == (bh)) { \
            __asm__("subs\t%1, %2, %3\n\tsbc\t%0, %0, %0"                   \
                    : "=r"(sh), "=r"(sl)                                    \
                    : "r"(al), "rI"(bl)                                     \
                    : "cc");                                                \
        } else if (BUILTIN_CONSTANT(al)) {                                  \
            __asm__("rsbs\t%1, %5, %4\n\tsbc\t%0, %2, %3"                   \
                    : "=r"(sh), "=&r"(sl)                                   \
                    : "r"(ah), "rI"(bh), "rI"(al), "r"(bl)                  \
                    : "cc");                                                \
        } else if (BUILTIN_CONSTANT(bl)) {                                  \
            __asm__("subs\t%1, %4, %5\n\tsbc\t%0, %2, %3"                   \
                    : "=r"(sh), "=&r"(sl)                                   \
                    : "r"(ah), "rI"(bh), "r"(al), "rI"(bl)                  \
                    : "cc");                                                \
        } else {                                                            \
            __asm__("subs\t%1, %4, %5\n\tsbc\t%0, %2, %3"                   \
                    : "=r"(sh), "=&r"(sl)                                   \
                    : "r"(ah), "rI"(bh), "r"(al), "rI"(bl)                  \
                    : "cc");                                                \
        }                                                                   \
    } while (0)
#else
#define USUB_AABB(sh, sl, ah, al, bh, bl)                                   \
    do {                                                                    \
        if (BUILTIN_CONSTANT(ah) && BUILTIN_CONSTANT(bh) && (ah) == (bh)) { \
            __asm__("subs\t%1, %2, %3\n\tsbc\t%0, %0, %0"                   \
                    : "=r"(sh), "=r"(sl)                                    \
                    : "r"(al), "rI"(bl)                                     \
                    : "cc");                                                \
        } else if (BUILTIN_CONSTANT(al)) {                                  \
            if (BUILTIN_CONSTANT(ah)) {                                     \
                __asm__("rsbs\t%1, %5, %4\n\trsc\t%0, %3, %2"               \
                        : "=r"(sh), "=&r"(sl)                               \
                        : "rI"(ah), "r"(bh), "rI"(al), "r"(bl)              \
                        : "cc");                                            \
            } else {                                                        \
                __asm__("rsbs\t%1, %5, %4\n\tsbc\t%0, %2, %3"               \
                        : "=r"(sh), "=&r"(sl)                               \
                        : "r"(ah), "rI"(bh), "rI"(al), "r"(bl)              \
                        : "cc");                                            \
            }                                                               \
        } else if (BUILTIN_CONSTANT(ah)) {                                  \
            if (BUILTIN_CONSTANT(bl)) {                                     \
                __asm__("subs\t%1, %4, %5\n\trsc\t%0, %3, %2"               \
                        : "=r"(sh), "=&r"(sl)                               \
                        : "rI"(ah), "r"(bh), "r"(al), "rI"(bl)              \
                        : "cc");                                            \
            } else {                                                        \
                __asm__("rsbs\t%1, %5, %4\n\trsc\t%0, %3, %2"               \
                        : "=r"(sh), "=&r"(sl)                               \
                        : "rI"(ah), "r"(bh), "rI"(al), "r"(bl)              \
                        : "cc");                                            \
            }                                                               \
        } else if (BUILTIN_CONSTANT(bl)) {                                  \
            __asm__("subs\t%1, %4, %5\n\tsbc\t%0, %2, %3"                   \
                    : "=r"(sh), "=&r"(sl)                                   \
                    : "r"(ah), "rI"(bh), "r"(al), "rI"(bl)                  \
                    : "cc");                                                \
        } else {                                                            \
            __asm__("subs\t%1, %4, %5\n\tsbc\t%0, %2, %3"                   \
                    : "=r"(sh), "=&r"(sl)                                   \
                    : "r"(ah), "rI"(bh), "r"(al), "rI"(bl)                  \
                    : "cc");                                                \
        }                                                                   \
    } while (0)
#endif

#if defined(__ARM_ARCH_2__) || defined(__ARM_ARCH_2A__) || defined(__ARM_ARCH_3__)
#define UMUL_AB(xh, xl, a, b)                                                                       \
    do {                                                                                            \
        register uint32_t __t0, __t1, __t2;                                                         \
        __asm__("%@ Inlined UMUL_AB\n"                                                              \
                "	mov	%2, %5, lsr #16\n"                                                            \
                "	mov	%0, %6, lsr #16\n"                                                            \
                "	bic	%3, %5, %2, lsl #16\n"                                                        \
                "	bic	%4, %6, %0, lsl #16\n"                                                        \
                "	mul	%1, %3, %4\n"                                                                 \
                "	mul	%4, %2, %4\n"                                                                 \
                "	mul	%3, %0, %3\n"                                                                 \
                "	mul	%0, %2, %0\n"                                                                 \
                "	adds	%3, %4, %3\n"                                                                \
                "	addcs	%0, %0, #65536\n"                                                           \
                "	adds	%1, %1, %3, lsl #16\n"                                                       \
                "	adc	%0, %0, %3, lsr #16"                                                          \
                : "=&r"((uint32_t)(xh)), "=r"((uint32_t)(xl)), "=&r"(__t0), "=&r"(__t1), "=r"(__t2) \
                : "r"((uint32_t)(a)), "r"((uint32_t)(b))                                            \
                : "cc");                                                                            \
    } while (0)

#else /* ARMv4 or newer */
#define UMUL_AB(xh, xl, a, b) __asm__("umull %0,%1,%2,%3" : "=&r"(xl), "=&r"(xh) : "r"(a), "r"(b))
#define SMUL_AB(xh, xl, a, b) __asm__("smull %0,%1,%2,%3" : "=&r"(xl), "=&r"(xh) : "r"(a), "r"(b))
#endif /* defined(__ARM_ARCH_2__) ... */

#define COUNT_LEADING_ZEROS(count, x)  COUNT_LEADING_ZEROS_BUILTIN(count, x)
#define COUNT_TRAILING_ZEROS(count, x) COUNT_TRAILING_ZEROS_BUILTIN(count, x)
#endif /* __arm__ */

#if defined(__aarch64__) && MPI_LIMB_BITS == 64
#define UADD_AABB(sh, sl, ah, al, bh, bl)                                               \
    do {                                                                                \
        if (BUILTIN_CONSTANT(bl) && ~(uint64_t)(bl) <= (uint64_t)(bl)) {                \
            __asm__("subs\t%1, %x4, %5\n\tadc\t%0, %x2, %x3"                            \
                    : "=r"(sh), "=&r"(sl)                                               \
                    : "rZ"((uint64_t)(ah)), "rZ"((uint64_t)(bh)), "%r"((uint64_t)(al)), \
                      "rI"(-(uint64_t)(bl))                                             \
                    : "cc");                                                            \
        } else {                                                                        \
            __asm__("adds\t%1, %x4, %5\n\tadc\t%0, %x2, %x3"                            \
                    : "=r"(sh), "=&r"(sl)                                               \
                    : "rZ"((uint64_t)(ah)), "rZ"((uint64_t)(bh)), "%r"((uint64_t)(al)), \
                      "rI"((uint64_t)(bl))                                              \
                    : "cc");                                                            \
        }                                                                               \
    } while (0)

#define USUB_AABB(sh, sl, ah, al, bh, bl)                                                      \
    do {                                                                                       \
        if (BUILTIN_CONSTANT(bl) && ~(uint64_t)(bl) <= (uint64_t)(bl)) {                       \
            __asm__("adds\t%1, %x4, %5\n\tsbc\t%0, %x2, %x3"                                   \
                    : "=r,r"(sh), "=&r,&r"(sl)                                                 \
                    : "rZ,rZ"((uint64_t)(ah)), "rZ,rZ"((uint64_t)(bh)), "r,Z"((uint64_t)(al)), \
                      "rI,r"(-(uint64_t)(bl))                                                  \
                    : "cc");                                                                   \
        } else {                                                                               \
            __asm__("subs\t%1, %x4, %5\n\tsbc\t%0, %x2, %x3"                                   \
                    : "=r,r"(sh), "=&r,&r"(sl)                                                 \
                    : "rZ,rZ"((uint64_t)(ah)), "rZ,rZ"((uint64_t)(bh)), "r,Z"((uint64_t)(al)), \
                      "rI,r"((uint64_t)(bl))                                                   \
                    : "cc");                                                                   \
        }                                                                                      \
    } while (0)

#define UMUL_AB(ph, pl, m0, m1)                                         \
    do {                                                                \
        uint64_t __m0 = (m0), __m1 = (m1);                              \
        __asm__("umulh\t%0, %1, %2" : "=r"(ph) : "r"(__m0), "r"(__m1)); \
        (pl) = __m0 * __m1;                                             \
    } while (0)

#define COUNT_LEADING_ZEROS(count, x)  COUNT_LEADING_ZEROS_BUILTIN(count, x)
#define COUNT_TRAILING_ZEROS(count, x) COUNT_TRAILING_ZEROS_BUILTIN(count, x)
#endif /* __aarch64__ */

/* On x86 and x86_64, every asm implicitly clobbers "flags" and "fpsr",
   so we don't need : "cc".  */
#if (defined(__i386__) || defined(__i486__)) && MPI_LIMB_BITS == 32
#define UADD_AABB(sh, sl, ah, al, bh, bl) \
    __asm__("addl %5,%k1\n\tadcl %3,%k0"  \
            : "=r"(sh), "=&r"(sl)         \
            : "0"((uint32_t)(ah)), "g"((uint32_t)(bh)), "%1"((uint32_t)(al)), "g"((uint32_t)(bl)))
#define USUB_AABB(sh, sl, ah, al, bh, bl) \
    __asm__("subl %5,%k1\n\tsbbl %3,%k0"  \
            : "=r"(sh), "=&r"(sl)         \
            : "0"((uint32_t)(ah)), "g"((uint32_t)(bh)), "1"((uint32_t)(al)), "g"((uint32_t)(bl)))
#define UMUL_AB(w1, w0, u, v) \
    __asm__("mull %3" : "=a"(w0), "=d"(w1) : "%0"((uint32_t)(u)), "rm"((uint32_t)(v)))
#define UDIV_NND(q, r, n1, n0, dx) /* d renamed to dx avoiding "=d" */ \
    __asm__("divl %4"              /* stringification in K&R C */      \
            : "=a"(q), "=d"(r)                                         \
            : "0"((uint32_t)(n0)), "1"((uint32_t)(n1)), "rm"((uint32_t)(dx)))

#if defined(HAVE_HOST_CPU_i586) || defined(HAVE_HOST_CPU_pentium) || defined(HAVE_HOST_CPU_pentiummmx)
/* Pentium bsrl takes between 10 and 72 cycles depending where the most
   significant 1 bit is, hence the use of the following alternatives.  bsfl
   is slow too, between 18 and 42 depending where the least significant 1
   bit is, so let the generic COUNT_TRAILING_ZEROS below make use of the
   COUNT_LEADING_ZEROS here too.  */

#if defined(HAVE_HOST_CPU_pentiummmx) && !defined(MPI_ASM_STANDALONE)
/* The following should be a fixed 14 or 15 cycles, but possibly plus an L1
   cache miss reading from __mpi_clz_tab.  For P55 it's favoured over the float
   below so as to avoid mixing MMX and x87, since the penalty for switching
   between the two is about 100 cycles.

   The asm block sets __shift to -3 if the high 24 bits are clear, -2 for
   16, -1 for 8, or 0 otherwise.  This could be written equivalently as
   follows, but as of gcc 2.95.2 it results in conditional jumps.

       __shift = -(__n < 0x1000000);
       __shift -= (__n < 0x10000);
       __shift -= (__n < 0x100);

   The middle two sbbl and cmpl's pair, and with luck something gcc
   generates might pair with the first cmpl and the last sbbl.  The "32+1"
   constant could be folded into __mpi_clz_tab[], but it doesn't seem worth
   making a different table just for that.  */

#define COUNT_LEADING_ZEROS(c, n)                               \
    do {                                                        \
        uint32_t __n = (n);                                     \
        uint32_t __shift;                                       \
        __asm__("cmpl  $0x1000000, %1\n"                        \
                "sbbl  %0, %0\n"                                \
                "cmpl  $0x10000, %1\n"                          \
                "sbbl  $0, %0\n"                                \
                "cmpl  $0x100, %1\n"                            \
                "sbbl  $0, %0\n"                                \
                : "=&r"(__shift)                                \
                : "r"(__n));                                    \
        __shift = __shift * 8 + 24 + 1;                         \
        (c) = 32 + 1 - __shift - __mpi_clz_tab[__n >> __shift]; \
    } while (0)
#define COUNT_LEADING_ZEROS_NEED_CLZ_TAB
#define COUNT_LEADING_ZEROS_0 31 /* n==0 indistinguishable from n==1 */

#else /* ! pentiummmx || MPI_ASM_STANDALONE */
/* The following should be a fixed 14 cycles or so.  Some scheduling
   opportunities should be available between the float load/store too.  This
   sort of code is used in gcc 3 for __builtin_ffs (with "n&-n") and is
   apparently suggested by the Intel optimizing manual (don't know exactly
   where).  gcc 2.95 or up will be best for this, so the "double" is
   correctly aligned on the stack.  */
#define COUNT_LEADING_ZEROS(c, n)            \
    do {                                     \
        union {                              \
            double d;                        \
            unsigned a[2];                   \
        } __u;                               \
        __u.d = (mpi_limb_t)(n);             \
        (c) = 0x3FF + 31 - (__u.a[1] >> 20); \
    } while (0)
#define COUNT_LEADING_ZEROS_0 (0x3FF + 31)
#endif /* pentiummx */

#else /* ! pentium */

/* On P6, gcc prior to 3.0 generates a partial register stall for
   __cbtmp^31, due to using "xorb $31" instead of "xorl $31", the former
   being 1 code byte smaller.  "31-__cbtmp" is a workaround, probably at the
   cost of one extra instruction.  Do this for "i386" too, since that means
   generic x86.  */
#if !defined(COUNT_LEADING_ZEROS) && __GNUC__ < 3                                                       \
    && (defined(HAVE_HOST_CPU_i386) || defined(HAVE_HOST_CPU_i686) || defined(HAVE_HOST_CPU_pentiumpro) \
        || defined(HAVE_HOST_CPU_pentium2) || defined(HAVE_HOST_CPU_pentium3))
#define COUNT_LEADING_ZEROS(count, x)                                \
    do {                                                             \
        uint32_t __cbtmp;                                            \
        MPI_ASSERT((x) != 0);                                        \
        __asm__("bsrl %1,%0" : "=r"(__cbtmp) : "rm"((uint32_t)(x))); \
        (count) = 31 - __cbtmp;                                      \
    } while (0)
#endif /* gcc<3 asm bsrl */

#ifndef COUNT_LEADING_ZEROS
#define COUNT_LEADING_ZEROS(count, x)                                \
    do {                                                             \
        uint32_t __cbtmp;                                            \
        MPI_ASSERT((x) != 0);                                        \
        __asm__("bsrl %1,%0" : "=r"(__cbtmp) : "rm"((uint32_t)(x))); \
        (count) = __cbtmp ^ 31;                                      \
    } while (0)
#endif /* asm bsrl */

#ifndef COUNT_TRAILING_ZEROS
#define COUNT_TRAILING_ZEROS(count, x)                              \
    do {                                                            \
        MPI_ASSERT((x) != 0);                                       \
        __asm__("bsfl %1,%k0" : "=r"(count) : "rm"((uint32_t)(x))); \
    } while (0)
#endif /* asm bsfl */

#endif /* ! pentium */

#endif /* 80x86 */

#if defined(__amd64__) && MPI_LIMB_BITS == 64
#define UADD_AABB(sh, sl, ah, al, bh, bl) \
    __asm__("addq %5,%q1\n\tadcq %3,%q0"  \
            : "=r"(sh), "=&r"(sl)         \
            : "0"((uint64_t)(ah)), "rme"((uint64_t)(bh)), "%1"((uint64_t)(al)), "rme"((uint64_t)(bl)))
#define USUB_AABB(sh, sl, ah, al, bh, bl) \
    __asm__("subq %5,%q1\n\tsbbq %3,%q0"  \
            : "=r"(sh), "=&r"(sl)         \
            : "0"((uint64_t)(ah)), "rme"((uint64_t)(bh)), "1"((uint64_t)(al)), "rme"((uint64_t)(bl)))
#if (defined MPI_ASM_X86_MULX)                                             \
    && (defined(HAVE_HOST_CPU_haswell) || defined(HAVE_HOST_CPU_broadwell) \
        || defined(HAVE_HOST_CPU_skylake) || defined(HAVE_HOST_CPU_bd4) || defined(HAVE_HOST_CPU_zen))
#define UMUL_AB(w1, w0, u, v) \
    __asm__("mulx\t%3, %q0, %q1" : "=r"(w0), "=r"(w1) : "%d"((uint64_t)(u)), "rm"((uint64_t)(v)))
#else
#define UMUL_AB(w1, w0, u, v) \
    __asm__("mulq\t%3" : "=a"(w0), "=d"(w1) : "%0"((uint64_t)(u)), "rm"((uint64_t)(v)))
#endif
#define UDIV_NND(q, r, n1, n0, dx) /* d renamed to dx avoiding "=d" */ \
    __asm__("divq %4"              /* stringification in K&R C */      \
            : "=a"(q), "=d"(r)                                         \
            : "0"((uint64_t)(n0)), "1"((uint64_t)(n1)), "rm"((uint64_t)(dx)))

#if defined(HAVE_HOST_CPU_haswell) || defined(HAVE_HOST_CPU_broadwell) || defined(HAVE_HOST_CPU_skylake) \
    || defined(HAVE_HOST_CPU_k10) || defined(HAVE_HOST_CPU_bd1) || defined(HAVE_HOST_CPU_bd2)            \
    || defined(HAVE_HOST_CPU_bd3) || defined(HAVE_HOST_CPU_bd4) || defined(HAVE_HOST_CPU_zen)            \
    || defined(HAVE_HOST_CPU_bobcat) || defined(HAVE_HOST_CPU_jaguar)
#define COUNT_LEADING_ZEROS(count, x)                                       \
    do {                                                                    \
        /* This is lzcnt, spelled for older assemblers.  Destination and */ \
        /* source must be a 64-bit registers, hence cast and %q.         */ \
        __asm__("rep;bsr\t%1, %q0" : "=r"(count) : "rm"((uint64_t)(x)));    \
    } while (0)
#define COUNT_LEADING_ZEROS_0 64
#else
#define COUNT_LEADING_ZEROS(count, x)                                \
    do {                                                             \
        uint64_t __cbtmp;                                            \
        MPI_ASSERT((x) != 0);                                        \
        __asm__("bsr\t%1,%0" : "=r"(__cbtmp) : "rm"((uint64_t)(x))); \
        (count) = __cbtmp ^ 63;                                      \
    } while (0)
#endif

#if defined(HAVE_HOST_CPU_bd2) || defined(HAVE_HOST_CPU_bd3) || defined(HAVE_HOST_CPU_bd4) \
    || defined(HAVE_HOST_CPU_zen) || defined(HAVE_HOST_CPU_jaguar)
#define COUNT_TRAILING_ZEROS(count, x)                                      \
    do {                                                                    \
        /* This is tzcnt, spelled for older assemblers.  Destination and */ \
        /* source must be a 64-bit registers, hence cast and %q.         */ \
        __asm__("rep;bsf\t%1, %q0" : "=r"(count) : "rm"((uint64_t)(x)));    \
    } while (0)
#define COUNT_TRAILING_ZEROS_0 64
#else
#define COUNT_TRAILING_ZEROS(count, x)                               \
    do {                                                             \
        MPI_ASSERT((x) != 0);                                        \
        __asm__("bsf\t%1, %q0" : "=r"(count) : "rm"((uint64_t)(x))); \
    } while (0)
#endif
#endif /* __amd64__ */

#endif

/* If this machine has no inline assembler, use C macros.  */
/* carry, R = A + B */
#ifndef UADD_AB
#define UADD_AB(CARRY, R, A, B)     \
    do {                            \
        mpi_limb_t __t = (A) + (B); \
        (CARRY) = __t < (A);        \
        (R) = __t;                  \
    } while (0)
#endif

/* carry, R = A + B + C */
#if !defined(UADD_ABC)
#define UADD_ABC(CARRY, R, A, B, C)  \
    do {                             \
        mpi_limb_t __s = (A) + (B);  \
        mpi_limb_t __t1 = __s < (A); \
        mpi_limb_t __r = __s + (C);  \
        mpi_limb_t __t2 = __r < __s; \
        (CARRY) = __t1 + __t2;       \
        (R) = __r;                   \
    } while (0)
#endif

/* borrow, R = A - B */
#if !defined(USUB_AB)
#define USUB_AB(BORROW, R, A, B) \
    do {                         \
        (BORROW) = (A) < (B);    \
        (R) = (A) - (B);         \
    } while (0)
#endif

/* borrow, R = A - B - C */
#if !defined(USUB_ABC)
#define USUB_ABC(BORROW, R, A, B, C) \
    do {                             \
        mpi_limb_t __s = (A) - (B);  \
        mpi_limb_t __t1 = __s > (A); \
        mpi_limb_t __r = __s - (C);  \
        mpi_limb_t __t2 = __r > __s; \
        (BORROW) = __t1 + __t2;      \
        (R) = __r;                   \
    } while (0)
#endif

#if !defined(UADD_AABB)
#define UADD_AABB(sh, sl, ah, al, bh, bl)  \
    do {                                   \
        mpi_limb_t __x;                    \
        mpi_limb_t __al = (al);            \
        mpi_limb_t __bl = (bl);            \
        __x = __al + __bl;                 \
        (sh) = (ah) + (bh) + (__x < __al); \
        (sl) = __x;                        \
    } while (0)
#endif

#if !defined(USUB_AABB)
#define USUB_AABB(sh, sl, ah, al, bh, bl)   \
    do {                                    \
        mpi_limb_t __x;                     \
        mpi_limb_t __al = (al);             \
        mpi_limb_t __bl = (bl);             \
        __x = __al - __bl;                  \
        (sh) = (ah) - (bh) - (__al < __bl); \
        (sl) = __x;                         \
    } while (0)
#endif

/* If we lack UMUL_AB but have SMUL_AB, define UMUL_AB in terms of SMUL_AB. */
#if !defined(UMUL_AB) && defined(SMUL_AB)
#define UMUL_AB(w1, w0, u, v)                                                                             \
    do {                                                                                                  \
        mpi_limb_t __w1;                                                                                  \
        mpi_limb_t __xm0 = (u), __xm1 = (v);                                                              \
        SMUL_AB(__w1, w0, __xm0, __xm1);                                                                  \
        (w1) =                                                                                            \
            __w1 + (-(__xm0 >> (MPI_LIMB_BITS - 1)) & __xm1) + (-(__xm1 >> (MPI_LIMB_BITS - 1)) & __xm0); \
    } while (0)
#endif

/* If we still don't have UMUL_AB, define it using plain C. */
#if !defined(UMUL_AB)
#define UMUL_AB(w1, w0, u, v)                                       \
    do {                                                            \
        mpi_limb_t __u = (u), __v = (v);                            \
                                                                    \
        mpi_limb_t __ul = UINT_LO(__u);                             \
        mpi_limb_t __uh = UINT_HI(__u);                             \
        mpi_limb_t __vl = UINT_LO(__v);                             \
        mpi_limb_t __vh = UINT_HI(__v);                             \
                                                                    \
        mpi_limb_t __x0 = (mpi_limb_t)__ul * __vl;                  \
        mpi_limb_t __x1 = (mpi_limb_t)__ul * __vh;                  \
        mpi_limb_t __x2 = (mpi_limb_t)__uh * __vl;                  \
        mpi_limb_t __x3 = (mpi_limb_t)__uh * __vh;                  \
                                                                    \
        __x1 += UINT_HI(__x0); /* this can't give carry */          \
        __x1 += __x2;          /* but this indeed can */            \
        if (__x1 < __x2) {     /* did we get it? */                 \
            __x3 += UINT_H1;   /* yes, add it in the proper pos. */ \
        }                                                           \
                                                                    \
        (w1) = __x3 + UINT_HI(__x1);                                \
        (w0) = (__x1 << (MPI_LIMB_BITS / 2)) + UINT_LO(__x0);       \
    } while (0)
#endif

/* If we don't have SMUL_AB, define it using UMUL_AB */
#if !defined(SMUL_AB)
#define SMUL_AB(w1, w0, u, v)                                                                             \
    do {                                                                                                  \
        mpi_limb_t __w1;                                                                                  \
        mpi_limb_t __xm0 = (u), __xm1 = (v);                                                              \
        UMUL_AB(__w1, w0, __xm0, __xm1);                                                                  \
        (w1) =                                                                                            \
            __w1 - (-(__xm0 >> (MPI_LIMB_BITS - 1)) & __xm1) - (-(__xm1 >> (MPI_LIMB_BITS - 1)) & __xm0); \
    } while (0)
#endif

/* If UDIV_NND was not defined for this processor. */
#if !defined(UDIV_NND)
#define UDIV_NND(q, r, n1, n0, d)                                                 \
    do {                                                                          \
        mpi_limb_t __d1, __d0, __q1, __q0, __r1, __r0, __m;                       \
                                                                                  \
        MPI_ASSERT((d) != 0);                                                     \
        MPI_ASSERT((n1) < (d));                                                   \
                                                                                  \
        __d1 = UINT_HI(d);                                                        \
        __d0 = UINT_LO(d);                                                        \
                                                                                  \
        __q1 = (n1) / __d1;                                                       \
        __r1 = (n1)-__q1 * __d1;                                                  \
        __m = __q1 * __d0;                                                        \
        __r1 = __r1 * UINT_H1 | UINT_HI(n0);                                      \
        if (__r1 < __m) {                                                         \
            __q1--, __r1 += (d);                                                  \
            if (__r1 >= (d)) { /* i.e. we didn't get carry when adding to __r1 */ \
                if (__r1 < __m) {                                                 \
                    __q1--;                                                       \
                    __r1 += (d);                                                  \
                }                                                                 \
            }                                                                     \
        }                                                                         \
        __r1 -= __m;                                                              \
        __q0 = __r1 / __d1;                                                       \
        __r0 = __r1 - __q0 * __d1;                                                \
        __m = __q0 * __d0;                                                        \
        __r0 = __r0 * UINT_H1 | UINT_LO(n0);                                      \
        if (__r0 < __m) {                                                         \
            __q0--, __r0 += (d);                                                  \
            if (__r0 >= (d)) {                                                    \
                if (__r0 < __m) {                                                 \
                    __q0--;                                                       \
                    __r0 += (d);                                                  \
                }                                                                 \
            }                                                                     \
        }                                                                         \
        __r0 -= __m;                                                              \
                                                                                  \
        (q) = __q1 * UINT_H1 | __q0;                                              \
        (r) = __r0;                                                               \
    } while (0)
#define UDIV_NEEDS_NORMALIZATION 1
#endif

#if !defined(COUNT_LEADING_ZEROS)
#define COUNT_LEADING_ZEROS(count, x)                                                                \
    do {                                                                                             \
        mpi_limb_t __xr = (x);                                                                       \
        mpi_limb_t __a;                                                                              \
                                                                                                     \
        if (MPI_LIMB_BITS == 32) {                                                                   \
            __a = __xr < ((mpi_limb_t)1 << 2 * BITS_S4)                                              \
                      ? (__xr < ((mpi_limb_t)1 << BITS_S4) ? 1 : BITS_S4 + 1)                        \
                      : (__xr < ((mpi_limb_t)1 << 3 * BITS_S4) ? 2 * BITS_S4 + 1 : 3 * BITS_S4 + 1); \
        } else {                                                                                     \
            for (__a = MPI_LIMB_BITS - 8; __a > 0; __a -= 8) {                                       \
                if (((__xr >> __a) & 0xff) != 0) { break; }                                          \
            }                                                                                        \
            ++__a;                                                                                   \
        }                                                                                            \
                                                                                                     \
        (count) = MPI_LIMB_BITS + 1 - __a - __mpi_clz_tab[__xr >> __a];                              \
    } while (0)
/* This version gives a well-defined value for zero. */
#define COUNT_LEADING_ZEROS_0 (MPI_LIMB_BITS - 1)
#define COUNT_LEADING_ZEROS_NEED_CLZ_TAB
#define COUNT_LEADING_ZEROS_SLOW
#endif

/* clz_tab needed by mpn/x86/pentium/mod_1.asm in a fat binary */
#if defined(HAVE_HOST_CPU_FAMILY_x86) && WANT_FAT_BINARY
#define COUNT_LEADING_ZEROS_NEED_CLZ_TAB
#endif

#ifdef COUNT_LEADING_ZEROS_NEED_CLZ_TAB
extern const unsigned char __mpi_clz_tab[129];
#endif

#if !defined(COUNT_TRAILING_ZEROS)
#if !defined(COUNT_LEADING_ZEROS_SLOW)
/* Define COUNT_TRAILING_ZEROS using an asm COUNT_LEADING_ZEROS.  */
#define COUNT_TRAILING_ZEROS(count, x)                    \
    do {                                                  \
        mpi_limb_t __ctz_x = (x);                         \
        mpi_limb_t __ctz_c;                               \
        MPI_ASSERT(__ctz_x != 0);                         \
        COUNT_LEADING_ZEROS(__ctz_c, __ctz_x & -__ctz_x); \
        (count) = MPI_LIMB_BITS - 1 - __ctz_c;            \
    } while (0)
#else
/* Define COUNT_TRAILING_ZEROS in plain C, assuming small counts are common.
   We use clz_tab without ado, since the C COUNT_LEADING_ZEROS above will have
   pulled it in.  */
#define COUNT_TRAILING_ZEROS(count, x)                                         \
    do {                                                                       \
        mpi_limb_t __ctz_x = (x);                                              \
        int __ctz_c;                                                           \
                                                                               \
        if (LIKELY((__ctz_x & 0xff) != 0))                                     \
            (count) = __mpi_clz_tab[__ctz_x & -__ctz_x] - 2;                   \
        else {                                                                 \
            for (__ctz_c = 8 - 2; __ctz_c < MPI_LIMB_BITS - 2; __ctz_c += 8) { \
                __ctz_x >>= 8;                                                 \
                if (LIKELY((__ctz_x & 0xff) != 0)) break;                      \
            }                                                                  \
                                                                               \
            (count) = __ctz_c + __mpi_clz_tab[__ctz_x & -__ctz_x];             \
        }                                                                      \
    } while (0)
#endif
#endif

#ifndef UDIV_NEEDS_NORMALIZATION
#define UDIV_NEEDS_NORMALIZATION 0
#endif

/* Whether UDIV_NND is actually implemented with UDIV_NND_PREINV, and
   that hence the latter should always be used.  */
#ifndef UDIV_PREINV_ALWAYS
#define UDIV_PREINV_ALWAYS 0
#endif

/**
 * UDIV_NND_PREINV
 *
 * Based on work by Niels Möller and Torbjörn Granlund.
 *
 * We write things strangely below, a more straightforward version:
 *  _r = (nl) - _qh * (d);
 *  _t = _r + (d);
 *  if (_r >= _ql) {
 *      _qh--;
 *      _r = _t;
 *  }
 *
 * For one operation shorter critical path, one may want to use this form:
 *  _p = _qh * (d)
 *  _s = (nl) + (d);
 *  _r = (nl) - _p;
 *  _t = _s - _p;
 *  if (_r >= _ql) {
 *      _qh--;
 *      _r = _t;
 *  }
 */
#define UDIV_NND_PREINV(q, r, nh, nl, d, di)                            \
    do {                                                                \
        mpi_limb_t _qh, _ql, _r, _mask;                                 \
        UMUL_AB(_qh, _ql, (nh), (di));                                  \
        if (BUILTIN_CONSTANT(nl) && (nl) == 0) {                        \
            _qh += (nh) + 1;                                            \
            _r = -_qh * (d);                                            \
            _mask = -(mpi_limb_t)(_r > _ql); /* both > and >= are OK */ \
            _qh += _mask;                                               \
            _r += _mask & (d);                                          \
        } else {                                                        \
            UADD_AB(_qh, _ql, _qh, _ql, (nh) + 1, (nl));                \
            _r = (nl)-_qh * (d);                                        \
            _mask = -(mpi_limb_t)(_r > _ql); /* both > and >= are OK */ \
            _qh += _mask;                                               \
            _r += _mask & (d);                                          \
            if (UNLIKELY(_r >= (d))) {                                  \
                _r -= (d);                                              \
                _qh++;                                                  \
            }                                                           \
        }                                                               \
        (r) = _r;                                                       \
        (q) = _qh;                                                      \
    } while (0)

/**
 * Dividing (NH, NL) by D, returning the remainder only.
 *
 * Unlike UDIV_NND_PREINV, works also for the case NH == D, where the
 * quotient doesn't quite fit in a single limb.
 */
#define UMOD_NND_PREINV(r, nh, nl, d, di)                               \
    do {                                                                \
        mpi_limb_t _qh, _ql, _r, _mask;                                 \
        UMUL_AB(_qh, _ql, (nh), (di));                                  \
        if (BUILTIN_CONSTANT(nl) && (nl) == 0) {                        \
            _r = ~(_qh + (nh)) * (d);                                   \
            _mask = -(mpi_limb_t)(_r > _ql); /* both > and >= are OK */ \
            _r += _mask & (d);                                          \
        } else {                                                        \
            UADD_AB(_qh, _ql, _qh, _ql, (nh) + 1, (nl));                \
            _r = (nl)-_qh * (d);                                        \
            _mask = -(mpi_limb_t)(_r > _ql); /* both > and >= are OK */ \
            _r += _mask & (d);                                          \
            if (UNLIKELY(_r >= (d))) { _r -= (d); }                     \
        }                                                               \
        (r) = _r;                                                       \
    } while (0)

/**
 * Compute quotient the quotient and remainder for n / d.
 * Requires d>= B^2 / 2 and n < d B.
 * di is the inverse floor ((B^3 - 1) / (d0 + d1 B)) - B.
 *
 * @note:
 * 1. Output variables are updated multiple times. Only some inputs
 * and outputs may overlap.
 */
#define UDIV_NNNDD(q, r1, r0, n2, n1, n0, d1, d0, dinv)              \
    do {                                                             \
        mpi_limb_t _q0, _t1, _t0, _mask;                             \
        UMUL_AB((q), _q0, (n2), (dinv));                             \
        UADD_AABB((q), _q0, (q), _q0, (n2), (n1));                   \
                                                                     \
        /* Compute the two most significant limbs of n - q'd */      \
        (r1) = (n1) - (d1) * (q);                                    \
        USUB_AABB((r1), (r0), (r1), (n0), (d1), (d0));               \
        UMUL_AB(_t1, _t0, (d0), (q));                                \
        USUB_AABB((r1), (r0), (r1), (r0), _t1, _t0);                 \
        (q)++;                                                       \
                                                                     \
        /* Conditionally adjust q and the remainders */              \
        _mask = -(mpi_limb_t)((r1) >= _q0);                          \
        (q) += _mask;                                                \
        UADD_AABB((r1), (r0), (r1), (r0), _mask &(d1), _mask &(d0)); \
        if (UNLIKELY((r1) >= (d1))) {                                \
            if ((r1) > (d1) || (r0) >= (d0)) {                       \
                (q)++;                                               \
                USUB_AABB((r1), (r0), (r1), (r0), (d1), (d0));       \
            }                                                        \
        }                                                            \
    } while (0)

#ifndef INVERT_UINT
#define INVERT_UINT(invxl, xl)                                \
    do {                                                      \
        mpi_limb_t _dummy;                                    \
        MPI_ASSERT((xl) != 0);                                \
        UDIV_NND(invxl, _dummy, ~(xl), ~((mpi_limb_t)0), xl); \
        (void)_dummy;                                         \
    } while (0)
#endif

#define INVERT_PI1(inv32, d1, d0)                   \
    do {                                            \
        mpi_limb_t _v, _p, _t1, _t0, _mask;         \
        INVERT_UINT(_v, d1);                        \
        _p = (d1)*_v;                               \
        _p += (d0);                                 \
        if (_p < (d0)) {                            \
            _v--;                                   \
            _mask = -(mpi_limb_t)(_p >= (d1));      \
            _p -= (d1);                             \
            _v += _mask;                            \
            _p -= _mask & (d1);                     \
        }                                           \
        UMUL_AB(_t1, _t0, d0, _v);                  \
        _p += _t1;                                  \
        if (_p < _t1) {                             \
            _v--;                                   \
            if (UNLIKELY(_p >= (d1))) {             \
                if (_p > (d1) || _t0 >= (d0)) _v--; \
            }                                       \
        }                                           \
        inv32 = _v;                                 \
    } while (0)

#if 0
// FIXME: not working
#define UDIV_NNDD(q, r1, r0, n1, n0, d1, d0)        \
    {                                               \
        mpi_limb_t quo = n1 / d1 + 1;               \
        for (;;) {                                  \
            mpi_limb_t w2, w1, w0, t1, t0;          \
            UMUL_AB(t0, w0, quo, d0);               \
            UMUL_AB(w1, t1, quo, d1);               \
            UADD_ABC(w2, w1, w1, t0, t1);           \
            MPI_ASSERT(w2 == 0 && w1 >= n1);        \
                                                    \
            if (w1 > n1 || (w1 == n1 && w0 > n0)) { \
                quo--;                              \
            } else {                                \
                (q) = quo;                          \
                USUB_AABB(r1, r0, w1, w0, d1, d0);  \
                break;                              \
            }                                       \
        }                                           \
    }
#else
#define UDIV_NNDD(q, r1, r0, n1, n0, d1, d0)                                   \
    {                                                                          \
        __uint128_t nn = ((__uint128_t)n1 << MPI_LIMB_BITS) + (__uint128_t)n0; \
        __uint128_t dd = ((__uint128_t)d1 << MPI_LIMB_BITS) + (__uint128_t)d0; \
        __uint128_t qq = nn / dd;                                              \
        __uint128_t rr = nn - dd * qq;                                         \
        (q) = (mpi_limb_t)qq;                                                  \
        (r1) = (mpi_limb_t)(rr >> MPI_LIMB_BITS);                              \
        (r0) = (mpi_limb_t)(rr & MPI_LIMB_MASK);                               \
    }
// XXX: implement with INVERT_PI1, UDIV_NNNDD
#endif

#endif
