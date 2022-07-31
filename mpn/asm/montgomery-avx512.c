/**
 * Copyright 2022 Kiran Nowak(kiran.nowak@gmail.com)
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

#include <assert.h>

#include "mpn-binary.h"
#include "mpn-montgomery.h"

#if (__ARCH32E >= __ARCH32E_K0)

#define RSA_AVX512_MIN_BITSIZE (1024)
#define RSA_AVX512_MAX_BITSIZE (13 * 1024)

#define NORM_DIGSIZE_AVX512 64 // uint64_t
#define NORM_BASE_AVX512    ((uint64_t)1 << NORM_DIGSIZE_AVX512)
#define NORM_MASK_AVX512    (NORM_BASE_AVX512 - 1)

#define EXP_DIGIT_SIZE_AVX512 (52)
#define EXP_DIGIT_BASE_AVX512 (1 << EXP_DIGIT_SIZE_AVX512)
#define EXP_DIGIT_MASK_AVX512 ((uint64_t)0xFFFFFFFFFFFFF)

/* number of digits */
MPN_INLINE int __digit_num_avx512(int bitSize, int digSize)
{
    return (bitSize + digSize - 1) / digSize;
}

/* number of "EXP_DIGIT_SIZE_AVX512" chunks in "bitSize" bit string matched for AMM */
MPN_INLINE unsigned int num_of_variable_avx512(int modulusBits)
{
    unsigned int ammBitSize = 2 + __digit_num_avx512(modulusBits, MPN_LIMB_BITS) * MPN_LIMB_BITS;
    unsigned int redNum = __digit_num_avx512(ammBitSize, EXP_DIGIT_SIZE_AVX512);
    return redNum;
}

/* buffer corresponding to num_of_variable_avx2() */
/* cpMontExp_avx512_BufferSize() */
MPN_INLINE int num_of_variable_buff_avx512(int len)
{
    int tail = len % 8;
    if (0 == tail) tail = 8;
    return len + (8 - tail);
}

/*
   converts regular (base = 2^64) representation
   into "redundant" (base = 2^DIGIT_SIZE) represenrartion
*/

/* pair of 52-bit digits occupys 13 bytes (the fact is using in implementation beloow) */
MPN_INLINE uint64_t getDig52(const uint8_t *pStr, int strLen)
{
    uint64_t digit = 0;
    for (; strLen > 0; strLen--) {
        digit <<= 8;
        digit += (uint64_t)(pStr[strLen - 1]);
    }
    return digit;
}

/* regular => redundant conversion */
static void regular_dig52(uint64_t *out, const uint64_t *in, int inBitSize)
{
    uint8_t *inStr = (uint8_t *)in;
    /* expected out length */
    int outLen = num_of_variable_buff_avx512(__digit_num_avx512(inBitSize, EXP_DIGIT_SIZE_AVX512));

    for (; inBitSize >= (2 * EXP_DIGIT_SIZE_AVX512); inBitSize -= (2 * EXP_DIGIT_SIZE_AVX512), out += 2) {
        out[0] = (*(uint64_t *)inStr) & EXP_DIGIT_MASK_AVX512;
        inStr += 6;
        out[1] = ((*(uint64_t *)inStr) >> 4) & EXP_DIGIT_MASK_AVX512;
        inStr += 7;
        outLen -= 2;
    }
    if (inBitSize > EXP_DIGIT_SIZE_AVX512) {
        uint64_t digit = getDig52(inStr, 7);
        out[0] = digit & EXP_DIGIT_MASK_AVX512;
        inStr += 6;
        // #yuriynat: inBitSize -= EXP_DIGIT_MASK_AVX512;
        inBitSize -= EXP_DIGIT_SIZE_AVX512;
        digit = getDig52(inStr, BITS2WORD8_SIZE(inBitSize));
        out[1] = digit >> 4;
        out += 2;
        outLen -= 2;
    } else if (inBitSize > 0) {
        out[0] = getDig52(inStr, BITS2WORD8_SIZE(inBitSize));
        out++;
        outLen--;
    }
    for (; outLen > 0; outLen--, out++) out[0] = 0;
}

/*
   converts "redundant" (base = 2^DIGIT_SIZE) representation
   into regular (base = 2^64)
*/
MPN_INLINE void putDig52(uint8_t *pStr, int strLen, uint64_t digit)
{
    for (; strLen > 0; strLen--) {
        *pStr++ = (uint8_t)(digit & 0xFF);
        digit >>= 8;
    }
}

static void dig52_regular(uint64_t *out, const uint64_t *in, int outBitSize)
{
    int i;
    int outLen = BITS2WORD64_SIZE(outBitSize);
    for (i = 0; i < outLen; i++) out[i] = 0;

    {
        uint8_t *outStr = (uint8_t *)out;
        for (; outBitSize >= (2 * EXP_DIGIT_SIZE_AVX512); outBitSize -= (2 * EXP_DIGIT_SIZE_AVX512), in += 2) {
            (*(uint64_t *)outStr) = in[0];
            outStr += 6;
            (*(uint64_t *)outStr) ^= in[1] << 4;
            outStr += 7;
        }
        if (outBitSize > EXP_DIGIT_SIZE_AVX512) {
            putDig52(outStr, 7, in[0]);
            outStr += 6;
            outBitSize -= EXP_DIGIT_SIZE_AVX512;
            putDig52(outStr, BITS2WORD8_SIZE(outBitSize), (in[1] << 4 | in[0] >> 48));
        } else if (outBitSize) {
            putDig52(outStr, BITS2WORD8_SIZE(outBitSize), in[0]);
        }
    }
}

/* ams functions */
typedef void (*cpAMM52)(uint64_t *out, const uint64_t *a, const uint64_t *b, const uint64_t *m, uint64_t k0, int len,
                        uint64_t *res);

static void AMM52(uint64_t *out, const uint64_t *a, const uint64_t *b, const uint64_t *m, uint64_t k0, int len,
                  uint64_t *res)
{
#define NUM64 ((int32_t)(sizeof(__m512i) / sizeof(uint64_t)))

    __mmask8 k1 = (__mmask8)_mm512_kmov(0x02); /* mask of the 2-nd elment */

    __m512i zero = _mm512_setzero_si512(); /* zeros */

    int n;
    int tail = len & (NUM64 - 1);
    int expLen = len;
    if (tail) expLen += (NUM64 - tail);

    /* make sure not inplace operation */
    // tbcd: temporary excluded: MPN_ASSERT(res!=a);
    // tbcd: temporary excluded: MPN_ASSERT(res!=b);

    /* set result to zero */
    for (n = 0; n < expLen; n += NUM64) _mm512_storeu_si512(res + n, zero);

    /*
    // do Almost Montgomery Multiplication
    */
    for (n = 0; n < len; n++) {
        /* compute and broadcast y = (r[0]+a[0]*b[0])*k0 */
        uint64_t y = ((res[0] + a[0] * b[n]) * k0) & EXP_DIGIT_MASK_AVX512;
        __m512i yn = _mm512_set1_epi64((Ipp64s)y);

        /* broadcast b[n] digit */
        __m512i bn = _mm512_set1_epi64((Ipp64s)b[n]);

        int i;
        __m512i rp, ap, mp, d;

        /* r[0] += a[0]*b + m[0]*y */
        __m512i ri = _mm512_loadu_si512(res); /* r[0] */
        __m512i ai = _mm512_loadu_si512(a);   /* a[0] */
        __m512i mi = _mm512_loadu_si512(m);   /* m[0] */
        ri = _mm512_madd52lo_epu64(ri, ai, bn);
        ri = _mm512_madd52lo_epu64(ri, mi, yn);

        /* shift r[0] by 1 digit */
        d = _mm512_srli_epi64(ri, EXP_DIGIT_SIZE_AVX512);
        d = _mm512_shuffle_epi32(d, 0x44);
        d = _mm512_mask_add_epi64(ri, k1, ri, d);

        for (i = 8; i < expLen; i += 8) {
            // rp = ri;
            ri = _mm512_loadu_si512(res + i);
            ap = ai;
            ai = _mm512_loadu_si512(a + i);
            mp = mi;
            mi = _mm512_loadu_si512(m + i);

            /* r[] += lo(a[]*b + m[]*y) */
            ri = _mm512_madd52lo_epu64(ri, ai, bn);
            ri = _mm512_madd52lo_epu64(ri, mi, yn);

            /* shift r[] by 1 digit */
            rp = _mm512_alignr_epi64(ri, d, 1);
            d = ri;

            /* r[] += hi(a[]*b + m[]*y) */
            rp = _mm512_madd52hi_epu64(rp, ap, bn);
            rp = _mm512_madd52hi_epu64(rp, mp, yn);
            _mm512_storeu_si512(res + i - NUM64, rp);
        }
        ri = _mm512_alignr_epi64(zero, d, 1);
        ri = _mm512_madd52hi_epu64(ri, ai, bn);
        ri = _mm512_madd52hi_epu64(ri, mi, yn);
        _mm512_storeu_si512(res + i - NUM64, ri);
    }

    /* normalization */
    {
        uint64_t acc = 0;
        for (n = 0; n < len; n++) {
            acc += res[n];
            out[n] = acc & EXP_DIGIT_MASK_AVX512;
            acc >>= EXP_DIGIT_SIZE_AVX512;
        }
    }
}

static void AMM52x20(uint64_t *out, const uint64_t *a, const uint64_t *b, const uint64_t *m, uint64_t k0, int len,
                     uint64_t *res)
{
    __mmask8 k2 = (__mmask8)_mm512_kmov(0x0f); /* mask of the 0-3 elments */

    /* load a */
    __m512i A0 = _mm512_loadu_si512(a);
    __m512i A1 = _mm512_loadu_si512(a + NUM64);
    __m512i A2 = _mm512_maskz_loadu_epi64(k2, a + 2 * NUM64);

    /* load m */
    __m512i M0 = _mm512_loadu_si512(m);
    __m512i M1 = _mm512_loadu_si512(m + NUM64);
    __m512i M2 = _mm512_maskz_loadu_epi64(k2, m + 2 * NUM64);

    /* R0, R1, R2 holds temporary result */
    __m512i R0 = _mm512_setzero_si512();
    __m512i R1 = _mm512_setzero_si512();
    __m512i R2 = _mm512_setzero_si512();

    __m512i ZERO = _mm512_setzero_si512(); /* zeros */
    __m512i K = _mm512_set1_epi64((Ipp64s)k0);

    UNUSED_PARAM(len);
    UNUSED_PARAM(res);

    __mmask8 k1 = (__mmask8)_mm512_kmov(0x01); /* mask of the 0 elment */
    int i;
    for (i = 0; i < 20; i++) {
        __m512i Bi = _mm512_set1_epi64((Ipp64s)b[i]); /* bloadcast(b[i]) */
        __m512i Yi = _mm512_setzero_si512();          /* Yi = 0 */
        __m512i tmp;

        R0 = _mm512_madd52lo_epu64(R0, A0, Bi); /* R += A*Bi (lo) */
        R1 = _mm512_madd52lo_epu64(R1, A1, Bi);
        R2 = _mm512_madd52lo_epu64(R2, A2, Bi);

        Yi = _mm512_madd52lo_epu64(ZERO, K, R0); /* Yi = R0*K */
        Yi = _mm512_permutexvar_epi64(ZERO, Yi); /* broadcast Yi */

        R0 = _mm512_madd52lo_epu64(R0, M0, Yi); /* R += M*Yi (lo) */
        R1 = _mm512_madd52lo_epu64(R1, M1, Yi);
        R2 = _mm512_madd52lo_epu64(R2, M2, Yi);

        /* shift R */
        tmp = _mm512_maskz_srli_epi64(k1, R0, EXP_DIGIT_SIZE_AVX512);
        R0 = _mm512_alignr_epi64(R1, R0, 1);
        R1 = _mm512_alignr_epi64(R2, R1, 1);
        R2 = _mm512_alignr_epi64(ZERO, R2, 1);
        R0 = _mm512_add_epi64(R0, tmp);

        R0 = _mm512_madd52hi_epu64(R0, A0, Bi); /* R += A*Bi (hi) */
        R1 = _mm512_madd52hi_epu64(R1, A1, Bi);
        R2 = _mm512_madd52hi_epu64(R2, A2, Bi);

        R0 = _mm512_madd52hi_epu64(R0, M0, Yi); /* R += M*Yi (hi) */
        R1 = _mm512_madd52hi_epu64(R1, M1, Yi);
        R2 = _mm512_madd52hi_epu64(R2, M2, Yi);
    }

    /* store de-normilized result */
    _mm512_storeu_si512(out, R0);
    _mm512_storeu_si512(out + NUM64, R1);
    _mm512_mask_storeu_epi64(out + 2 * NUM64, k2, R2);

    /* normalize result */
    {
        uint64_t acc = 0;
#if !defined(_MSC_VER) || defined(__INTEL_COMPZLER) // unkonwn for msvc
#pragma nounroll
#endif
        for (i = 0; i < 20; i++) {
            acc += out[i];
            out[i] = acc & EXP_DIGIT_MASK_AVX512;
            acc >>= EXP_DIGIT_SIZE_AVX512;
        }
    }
}

static void AMM52x40(uint64_t *out, const uint64_t *a, const uint64_t *b, const uint64_t *m, uint64_t k0, int len,
                     uint64_t *res)
{
    /* load a */
    __m512i A0 = _mm512_loadu_si512(a);
    __m512i A1 = _mm512_loadu_si512(a + NUM64);
    __m512i A2 = _mm512_loadu_si512(a + 2 * NUM64);
    __m512i A3 = _mm512_loadu_si512(a + 3 * NUM64);
    __m512i A4 = _mm512_loadu_si512(a + 4 * NUM64);

    /* load m */
    __m512i M0 = _mm512_loadu_si512(m);
    __m512i M1 = _mm512_loadu_si512(m + NUM64);
    __m512i M2 = _mm512_loadu_si512(m + 2 * NUM64);
    __m512i M3 = _mm512_loadu_si512(m + 3 * NUM64);
    __m512i M4 = _mm512_loadu_si512(m + 4 * NUM64);

    /* R0, R1, R2, R3, R4 holds temporary result */
    __m512i R0 = _mm512_setzero_si512();
    __m512i R1 = _mm512_setzero_si512();
    __m512i R2 = _mm512_setzero_si512();
    __m512i R3 = _mm512_setzero_si512();
    __m512i R4 = _mm512_setzero_si512();

    __m512i ZERO = _mm512_setzero_si512(); /* zeros */
    __m512i K = _mm512_set1_epi64((Ipp64s)k0);

    UNUSED_PARAM(len);
    UNUSED_PARAM(res);

    __mmask8 k1 = (__mmask8)_mm512_kmov(0x01); /* mask of the 0 elment */
    int i;
    for (i = 0; i < 40; i++) {
        __m512i Bi = _mm512_set1_epi64((Ipp64s)b[i]); /* bloadcast(b[i]) */
        __m512i Yi = _mm512_setzero_si512();          /* Yi = 0 */
        __m512i tmp;

        R0 = _mm512_madd52lo_epu64(R0, A0, Bi); /* R += A*Bi (lo) */
        R1 = _mm512_madd52lo_epu64(R1, A1, Bi);
        R2 = _mm512_madd52lo_epu64(R2, A2, Bi);
        R3 = _mm512_madd52lo_epu64(R3, A3, Bi);
        R4 = _mm512_madd52lo_epu64(R4, A4, Bi);

        Yi = _mm512_madd52lo_epu64(ZERO, K, R0); /* Yi = R0*K */
        Yi = _mm512_permutexvar_epi64(ZERO, Yi); /* broadcast Yi */

        R0 = _mm512_madd52lo_epu64(R0, M0, Yi); /* R += M*Yi (lo) */
        R1 = _mm512_madd52lo_epu64(R1, M1, Yi);
        R2 = _mm512_madd52lo_epu64(R2, M2, Yi);
        R3 = _mm512_madd52lo_epu64(R3, M3, Yi);
        R4 = _mm512_madd52lo_epu64(R4, M4, Yi);

        /* shift R */
        tmp = _mm512_maskz_srli_epi64(k1, R0, EXP_DIGIT_SIZE_AVX512);
        R0 = _mm512_alignr_epi64(R1, R0, 1);
        R1 = _mm512_alignr_epi64(R2, R1, 1);
        R2 = _mm512_alignr_epi64(R3, R2, 1);
        R3 = _mm512_alignr_epi64(R4, R3, 1);
        R4 = _mm512_alignr_epi64(ZERO, R4, 1);
        R0 = _mm512_add_epi64(R0, tmp);

        R0 = _mm512_madd52hi_epu64(R0, A0, Bi); /* R += A*Bi (hi) */
        R1 = _mm512_madd52hi_epu64(R1, A1, Bi);
        R2 = _mm512_madd52hi_epu64(R2, A2, Bi);
        R3 = _mm512_madd52hi_epu64(R3, A3, Bi);
        R4 = _mm512_madd52hi_epu64(R4, A4, Bi);

        R0 = _mm512_madd52hi_epu64(R0, M0, Yi); /* R += M*Yi (hi) */
        R1 = _mm512_madd52hi_epu64(R1, M1, Yi);
        R2 = _mm512_madd52hi_epu64(R2, M2, Yi);
        R3 = _mm512_madd52hi_epu64(R3, M3, Yi);
        R4 = _mm512_madd52hi_epu64(R4, M4, Yi);
    }

    /* store de-normilized result */
    _mm512_storeu_si512(out, R0);
    _mm512_storeu_si512(out + NUM64, R1);
    _mm512_storeu_si512(out + 2 * NUM64, R2);
    _mm512_storeu_si512(out + 3 * NUM64, R3);
    _mm512_storeu_si512(out + 4 * NUM64, R4);

    /* normalize result */
    {
        uint64_t acc = 0;
#if !defined(_MSC_VER) || defined(__INTEL_COMPZLER) // unkonwn for msvc
#pragma nounroll
#endif
        for (i = 0; i < 40; i++) {
            acc += out[i];
            out[i] = acc & EXP_DIGIT_MASK_AVX512;
            acc >>= EXP_DIGIT_SIZE_AVX512;
        }
    }
}

static void AMM52x60(uint64_t *out, const uint64_t *a, const uint64_t *b, const uint64_t *m, uint64_t k0, int len,
                     uint64_t *res)
{
    __mmask8 k2 = (__mmask8)_mm512_kmov(0x0f); /* mask of the 0-3 elments */

    /* load a */
    __m512i A0 = _mm512_loadu_si512(a);
    __m512i A1 = _mm512_loadu_si512(a + NUM64);
    __m512i A2 = _mm512_loadu_si512(a + 2 * NUM64);
    __m512i A3 = _mm512_loadu_si512(a + 3 * NUM64);
    __m512i A4 = _mm512_loadu_si512(a + 4 * NUM64);
    __m512i A5 = _mm512_loadu_si512(a + 5 * NUM64);
    __m512i A6 = _mm512_loadu_si512(a + 6 * NUM64);
    __m512i A7 = _mm512_maskz_loadu_epi64(k2, a + 7 * NUM64);

    /* load m */
    __m512i M0 = _mm512_loadu_si512(m);
    __m512i M1 = _mm512_loadu_si512(m + NUM64);
    __m512i M2 = _mm512_loadu_si512(m + 2 * NUM64);
    __m512i M3 = _mm512_loadu_si512(m + 3 * NUM64);
    __m512i M4 = _mm512_loadu_si512(m + 4 * NUM64);
    __m512i M5 = _mm512_loadu_si512(m + 5 * NUM64);
    __m512i M6 = _mm512_loadu_si512(m + 6 * NUM64);
    __m512i M7 = _mm512_maskz_loadu_epi64(k2, m + 7 * NUM64);

    /* R0, R1, R2, R3, R4, R5, R6, R7 holds temporary result */
    __m512i R0 = _mm512_setzero_si512();
    __m512i R1 = _mm512_setzero_si512();
    __m512i R2 = _mm512_setzero_si512();
    __m512i R3 = _mm512_setzero_si512();
    __m512i R4 = _mm512_setzero_si512();
    __m512i R5 = _mm512_setzero_si512();
    __m512i R6 = _mm512_setzero_si512();
    __m512i R7 = _mm512_setzero_si512();

    __m512i ZERO = _mm512_setzero_si512(); /* zeros */
    __m512i K = _mm512_set1_epi64((Ipp64s)k0);

    UNUSED_PARAM(len);
    UNUSED_PARAM(res);

    __mmask8 k1 = (__mmask8)_mm512_kmov(0x01); /* mask of the 0 elment */
    int i;
    for (i = 0; i < 60; i++) {
        __m512i Bi = _mm512_set1_epi64((Ipp64s)b[i]); /* bloadcast(b[i]) */
        __m512i Yi = _mm512_setzero_si512();          /* Yi = 0 */
        __m512i tmp;

        R0 = _mm512_madd52lo_epu64(R0, A0, Bi); /* R += A*Bi (lo) */
        R1 = _mm512_madd52lo_epu64(R1, A1, Bi);
        R2 = _mm512_madd52lo_epu64(R2, A2, Bi);
        R3 = _mm512_madd52lo_epu64(R3, A3, Bi);
        R4 = _mm512_madd52lo_epu64(R4, A4, Bi);
        R5 = _mm512_madd52lo_epu64(R5, A5, Bi);
        R6 = _mm512_madd52lo_epu64(R6, A6, Bi);
        R7 = _mm512_madd52lo_epu64(R7, A7, Bi);

        Yi = _mm512_madd52lo_epu64(ZERO, K, R0); /* Yi = R0*K */
        Yi = _mm512_permutexvar_epi64(ZERO, Yi); /* broadcast Yi */

        R0 = _mm512_madd52lo_epu64(R0, M0, Yi); /* R += M*Yi (lo) */
        R1 = _mm512_madd52lo_epu64(R1, M1, Yi);
        R2 = _mm512_madd52lo_epu64(R2, M2, Yi);
        R3 = _mm512_madd52lo_epu64(R3, M3, Yi);
        R4 = _mm512_madd52lo_epu64(R4, M4, Yi);
        R5 = _mm512_madd52lo_epu64(R5, M5, Yi);
        R6 = _mm512_madd52lo_epu64(R6, M6, Yi);
        R7 = _mm512_madd52lo_epu64(R7, M7, Yi);

        /* shift R */
        tmp = _mm512_maskz_srli_epi64(k1, R0, EXP_DIGIT_SIZE_AVX512);
        R0 = _mm512_alignr_epi64(R1, R0, 1);
        R1 = _mm512_alignr_epi64(R2, R1, 1);
        R2 = _mm512_alignr_epi64(R3, R2, 1);
        R3 = _mm512_alignr_epi64(R4, R3, 1);
        R4 = _mm512_alignr_epi64(R5, R4, 1);
        R5 = _mm512_alignr_epi64(R6, R5, 1);
        R6 = _mm512_alignr_epi64(R7, R6, 1);
        R7 = _mm512_alignr_epi64(ZERO, R7, 1);
        R0 = _mm512_add_epi64(R0, tmp);

        R0 = _mm512_madd52hi_epu64(R0, A0, Bi); /* R += A*Bi (hi) */
        R1 = _mm512_madd52hi_epu64(R1, A1, Bi);
        R2 = _mm512_madd52hi_epu64(R2, A2, Bi);
        R3 = _mm512_madd52hi_epu64(R3, A3, Bi);
        R4 = _mm512_madd52hi_epu64(R4, A4, Bi);
        R5 = _mm512_madd52hi_epu64(R5, A5, Bi);
        R6 = _mm512_madd52hi_epu64(R6, A6, Bi);
        R7 = _mm512_madd52hi_epu64(R7, A7, Bi);

        R0 = _mm512_madd52hi_epu64(R0, M0, Yi); /* R += M*Yi (hi) */
        R1 = _mm512_madd52hi_epu64(R1, M1, Yi);
        R2 = _mm512_madd52hi_epu64(R2, M2, Yi);
        R3 = _mm512_madd52hi_epu64(R3, M3, Yi);
        R4 = _mm512_madd52hi_epu64(R4, M4, Yi);
        R5 = _mm512_madd52hi_epu64(R5, M5, Yi);
        R6 = _mm512_madd52hi_epu64(R6, M6, Yi);
        R7 = _mm512_madd52hi_epu64(R7, M7, Yi);
    }

    /* store de-normilized result */
    _mm512_storeu_si512(out, R0);
    _mm512_storeu_si512(out + NUM64, R1);
    _mm512_storeu_si512(out + 2 * NUM64, R2);
    _mm512_storeu_si512(out + 3 * NUM64, R3);
    _mm512_storeu_si512(out + 4 * NUM64, R4);
    _mm512_storeu_si512(out + 5 * NUM64, R5);
    _mm512_storeu_si512(out + 6 * NUM64, R6);
    _mm512_mask_storeu_epi64(out + 7 * NUM64, k2, R7);

    /* normalize result */
    {
        uint64_t acc = 0;
#if !defined(_MSC_VER) || defined(__INTEL_COMPZLER) // unkonwn for msvc
#pragma nounroll
#endif
        for (i = 0; i < 60; i++) {
            acc += out[i];
            out[i] = acc & EXP_DIGIT_MASK_AVX512;
            acc >>= EXP_DIGIT_SIZE_AVX512;
        }
    }
}

static void AMM52x79(uint64_t *out, const uint64_t *a, const uint64_t *b, const uint64_t *m, uint64_t k0, int len,
                     uint64_t *res)
{
    __mmask8 k2 = (__mmask8)_mm512_kmov(0x7f); /* mask of the 0-7 elments */

    /* load a */
    __m512i A0 = _mm512_loadu_si512(a);
    __m512i A1 = _mm512_loadu_si512(a + NUM64);
    __m512i A2 = _mm512_loadu_si512(a + 2 * NUM64);
    __m512i A3 = _mm512_loadu_si512(a + 3 * NUM64);
    __m512i A4 = _mm512_loadu_si512(a + 4 * NUM64);
    __m512i A5 = _mm512_loadu_si512(a + 5 * NUM64);
    __m512i A6 = _mm512_loadu_si512(a + 6 * NUM64);
    __m512i A7 = _mm512_loadu_si512(a + 7 * NUM64);
    __m512i A8 = _mm512_loadu_si512(a + 8 * NUM64);
    __m512i A9 = _mm512_maskz_loadu_epi64(k2, a + 9 * NUM64);

    /* load m */
    __m512i M0 = _mm512_loadu_si512(m);
    __m512i M1 = _mm512_loadu_si512(m + NUM64);
    __m512i M2 = _mm512_loadu_si512(m + 2 * NUM64);
    __m512i M3 = _mm512_loadu_si512(m + 3 * NUM64);
    __m512i M4 = _mm512_loadu_si512(m + 4 * NUM64);
    __m512i M5 = _mm512_loadu_si512(m + 5 * NUM64);
    __m512i M6 = _mm512_loadu_si512(m + 6 * NUM64);
    __m512i M7 = _mm512_loadu_si512(m + 7 * NUM64);
    __m512i M8 = _mm512_loadu_si512(m + 8 * NUM64);
    __m512i M9 = _mm512_maskz_loadu_epi64(k2, m + 9 * NUM64);

    /* R0, R1, R2, R3, R4, R5, R6, R7, R8, R9 holds temporary result */
    __m512i R0 = _mm512_setzero_si512();
    __m512i R1 = _mm512_setzero_si512();
    __m512i R2 = _mm512_setzero_si512();
    __m512i R3 = _mm512_setzero_si512();
    __m512i R4 = _mm512_setzero_si512();
    __m512i R5 = _mm512_setzero_si512();
    __m512i R6 = _mm512_setzero_si512();
    __m512i R7 = _mm512_setzero_si512();
    __m512i R8 = _mm512_setzero_si512();
    __m512i R9 = _mm512_setzero_si512();

    __m512i ZERO = _mm512_setzero_si512(); /* zeros */
    __m512i K = _mm512_set1_epi64((Ipp64s)k0);

    UNUSED_PARAM(len);
    UNUSED_PARAM(res);

    __mmask8 k1 = (__mmask8)_mm512_kmov(0x01); /* mask of the 0 elment */
    int i;
    for (i = 0; i < 79; i++) {
        __m512i Bi = _mm512_set1_epi64((Ipp64s)b[i]); /* bloadcast(b[i]) */
        __m512i Yi = _mm512_setzero_si512();          /* Yi = 0 */
        __m512i tmp;

        R0 = _mm512_madd52lo_epu64(R0, A0, Bi); /* R += A*Bi (lo) */
        R1 = _mm512_madd52lo_epu64(R1, A1, Bi);
        R2 = _mm512_madd52lo_epu64(R2, A2, Bi);
        R3 = _mm512_madd52lo_epu64(R3, A3, Bi);
        R4 = _mm512_madd52lo_epu64(R4, A4, Bi);
        R5 = _mm512_madd52lo_epu64(R5, A5, Bi);
        R6 = _mm512_madd52lo_epu64(R6, A6, Bi);
        R7 = _mm512_madd52lo_epu64(R7, A7, Bi);
        R8 = _mm512_madd52lo_epu64(R8, A8, Bi);
        R9 = _mm512_madd52lo_epu64(R9, A9, Bi);

        Yi = _mm512_madd52lo_epu64(ZERO, K, R0); /* Yi = R0*K */
        Yi = _mm512_permutexvar_epi64(ZERO, Yi); /* broadcast Yi */

        R0 = _mm512_madd52lo_epu64(R0, M0, Yi); /* R += M*Yi (lo) */
        R1 = _mm512_madd52lo_epu64(R1, M1, Yi);
        R2 = _mm512_madd52lo_epu64(R2, M2, Yi);
        R3 = _mm512_madd52lo_epu64(R3, M3, Yi);
        R4 = _mm512_madd52lo_epu64(R4, M4, Yi);
        R5 = _mm512_madd52lo_epu64(R5, M5, Yi);
        R6 = _mm512_madd52lo_epu64(R6, M6, Yi);
        R7 = _mm512_madd52lo_epu64(R7, M7, Yi);
        R8 = _mm512_madd52lo_epu64(R8, M8, Yi);
        R9 = _mm512_madd52lo_epu64(R9, M9, Yi);

        /* shift R */
        tmp = _mm512_maskz_srli_epi64(k1, R0, EXP_DIGIT_SIZE_AVX512);
        R0 = _mm512_alignr_epi64(R1, R0, 1);
        R1 = _mm512_alignr_epi64(R2, R1, 1);
        R2 = _mm512_alignr_epi64(R3, R2, 1);
        R3 = _mm512_alignr_epi64(R4, R3, 1);
        R4 = _mm512_alignr_epi64(R5, R4, 1);
        R5 = _mm512_alignr_epi64(R6, R5, 1);
        R6 = _mm512_alignr_epi64(R7, R6, 1);
        R7 = _mm512_alignr_epi64(R8, R7, 1);
        R8 = _mm512_alignr_epi64(R9, R8, 1);
        R9 = _mm512_alignr_epi64(ZERO, R9, 1);
        R0 = _mm512_add_epi64(R0, tmp);

        R0 = _mm512_madd52hi_epu64(R0, A0, Bi); /* R += A*Bi (hi) */
        R1 = _mm512_madd52hi_epu64(R1, A1, Bi);
        R2 = _mm512_madd52hi_epu64(R2, A2, Bi);
        R3 = _mm512_madd52hi_epu64(R3, A3, Bi);
        R4 = _mm512_madd52hi_epu64(R4, A4, Bi);
        R5 = _mm512_madd52hi_epu64(R5, A5, Bi);
        R6 = _mm512_madd52hi_epu64(R6, A6, Bi);
        R7 = _mm512_madd52hi_epu64(R7, A7, Bi);
        R8 = _mm512_madd52hi_epu64(R8, A8, Bi);
        R9 = _mm512_madd52hi_epu64(R9, A9, Bi);

        R0 = _mm512_madd52hi_epu64(R0, M0, Yi); /* R += M*Yi (hi) */
        R1 = _mm512_madd52hi_epu64(R1, M1, Yi);
        R2 = _mm512_madd52hi_epu64(R2, M2, Yi);
        R3 = _mm512_madd52hi_epu64(R3, M3, Yi);
        R4 = _mm512_madd52hi_epu64(R4, M4, Yi);
        R5 = _mm512_madd52hi_epu64(R5, M5, Yi);
        R6 = _mm512_madd52hi_epu64(R6, M6, Yi);
        R7 = _mm512_madd52hi_epu64(R7, M7, Yi);
        R8 = _mm512_madd52hi_epu64(R8, M8, Yi);
        R9 = _mm512_madd52hi_epu64(R9, M9, Yi);
    }

    /* store de-normilized result */
    _mm512_storeu_si512(out, R0);
    _mm512_storeu_si512(out + NUM64, R1);
    _mm512_storeu_si512(out + 2 * NUM64, R2);
    _mm512_storeu_si512(out + 3 * NUM64, R3);
    _mm512_storeu_si512(out + 4 * NUM64, R4);
    _mm512_storeu_si512(out + 5 * NUM64, R5);
    _mm512_storeu_si512(out + 6 * NUM64, R6);
    _mm512_storeu_si512(out + 7 * NUM64, R7);
    _mm512_storeu_si512(out + 8 * NUM64, R8);
    _mm512_mask_storeu_epi64(out + 9 * NUM64, k2, R9);

    /* normalize result */
    {
        uint64_t acc = 0;
#if !defined(_MSC_VER) || defined(__INTEL_COMPZLER) // unkonwn for msvc
#pragma nounroll
#endif
        for (i = 0; i < 79; i++) {
            acc += out[i];
            out[i] = acc & EXP_DIGIT_MASK_AVX512;
            acc >>= EXP_DIGIT_SIZE_AVX512;
        }
    }
}

/* ======= degugging section =========================================*/
//#define _EXP_AVX512_DEBUG_
#ifdef _EXP_AVX512_DEBUG_
#include "pcpmontred.h"
void debugToConvMontDomain(mpn_limb_t *pR, const mpn_limb_t *redInp, const mpn_limb_t *redM, int almMM_bitsize,
                           const mpn_limb_t *pM, const mpn_limb_t *pRR, int nsM, mpn_limb_t k0, mpn_limb_t *pBuffer)
{
    uint64_t one[32] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint64_t redT[32];
    int redLen = __digit_num_avx512(almMM_bitsize, EXP_DIGIT_SIZE_AVX512);
    AMM52(redT, redInp, one, redM, k0, redLen, pBuffer);
    dig52_regular(pR, redT, almMM_bitsize);

    // cpMontMul_BNU(pR,              should be changed
    //              redT, nsM,
    //              pRR,  nsM,
    //              pM,   nsM, k0,
    //              pBuffer, 0);
    cpMul_BNU(pBuffer, pR, nsM, pRR, nsM, 0);
    cpMontRed_BNU_opt(pR, pBuffer, pM, nsM, k0);
}
#endif
/* ===================================================================*/

unsigned int mont_exp_bin_buffer_avx512(int modulusBits)
{
    unsigned int redNum = num_of_variable_avx512(modulusBits);       /* "sizeof" variable */
    unsigned int redBufferNum = num_of_variable_buff_avx512(redNum); /* "sizeof" variable  buffer */
    return redBufferNum * 8;
}

#if defined(MPI_USE_SLIDING_WINDOW_EXP)
unsigned int gsMontExpWinBuffer_avx512(int modulusBits)
{
    unsigned int w = mont_exp_win_size(modulusBits);

    unsigned int redNum = num_of_variable_avx512(modulusBits);       /* "sizeof" variable */
    unsigned int redBufferNum = num_of_variable_buff_avx512(redNum); /* "sizeof" variable  buffer */

    unsigned int bufferNum = CACHE_LINE_SIZE / (int32_t)sizeof(mpn_limb_t)
                             + mont_scramble_buffer_size(redNum, w) /* pre-computed table */
                             + redBufferNum * 7;                    /* addition 7 variables */
    return bufferNum;
}
#endif /* MPI_USE_SLIDING_WINDOW_EXP */


#define SET_BIT(ptr, nbit) (((uint8_t *)(ptr))[(nbit) / 8] |= ((1 << ((nbit) % 8)) & 0xFF))

/*
// "fast" binary montgomery exponentiation
//
// scratch buffer structure:
//    redX[redBufferLen]
//    redT[redBufferLen]
//    redY[redBufferLen]
//    redM[redBufferLen]
//    redBuffer[redBufferLen*3]
*/
unsigned int gsMontExpBin_BNU_avx512(mpn_limb_t *dataY, const mpn_limb_t *dataX, unsigned int nsX,
                                     const mpn_limb_t *dataE, unsigned int bitsizeE, MPZ_MOD_ENGINE *pMont,
                                     mpn_limb_t *pBuffer)
{
    const mpn_limb_t *dataM = MOD_MODULUS(pMont);
    const mpn_limb_t *dataRR = MOD_MNT_R2(pMont);
    unsigned int nsM = MOD_LEN(pMont);
    mpn_limb_t k0 = MOD_MNT_FACTOR(pMont);

    unsigned int nsE = MPN_BITS_TO_LIMBS(bitsizeE);

    int modulusBitSize = BITSIZE_BNU(dataM, nsM);
    int cnvMM_bitsize = __digit_num_avx512(modulusBitSize, MPN_LIMB_BITS) * MPN_LIMB_BITS;
    int almMM_bitsize = cnvMM_bitsize + 2;
    int redLen = __digit_num_avx512(almMM_bitsize, EXP_DIGIT_SIZE_AVX512);
    int redBufferLen = num_of_variable_buff_avx512(redLen);

    /* allocate buffers */
    mpn_limb_t *redX = pBuffer;
    mpn_limb_t *redT = redX + redBufferLen;
    mpn_limb_t *redY = redT + redBufferLen;
    mpn_limb_t *redM = redY + redBufferLen;
    mpn_limb_t *redBuffer = redM + redBufferLen;

    cpAMM52 ammFunc;
    switch (modulusBitSize) {
        case 1024:
            ammFunc = AMM52x20;
            break;
        case 2048:
            ammFunc = AMM52x40;
            break;
        case 3072:
            ammFunc = AMM52x60;
            break;
        case 4096:
            ammFunc = AMM52x79;
            break;
        default:
            ammFunc = AMM52;
            break;
    }

    /* convert modulus into reduced domain */
    ZEXPAND(redBuffer, redBufferLen, dataM, nsM);
    regular_dig52(redM, redBuffer, almMM_bitsize);

    /* compute taget domain Montgomery converter RR' */
    ZEROIZE(redBuffer, 0, redBufferLen);
    SET_BIT(redBuffer, (4 * redLen * EXP_DIGIT_SIZE_AVX512 - 4 * cnvMM_bitsize));
    regular_dig52(redY, redBuffer, almMM_bitsize);

    ZEXPAND(redBuffer, redBufferLen, dataRR, nsM);
    regular_dig52(redT, redBuffer, almMM_bitsize);
    ammFunc(redT, redT, redT, redM, k0, redLen, redBuffer);
    ammFunc(redT, redT, redY, redM, k0, redLen, redBuffer);

    /* convert base to Montgomery domain */
    ZEXPAND(redY, redBufferLen /*nsX+1*/, dataX, nsX);
    regular_dig52(redX, redY, almMM_bitsize);
    ammFunc(redX, redX, redT, redM, k0, redLen, redBuffer);

    /* init result */
    COPY_BNU(redY, redX, redLen);

    FIX_BNU(dataE, nsE);
    {
        /* execute most significant part pE */
        mpn_limb_t eValue = dataE[nsE - 1];
        int n = mpz_nlz(eValue) + 1;

        eValue <<= n;
        for (; n < MPN_LIMB_BITS; n++, eValue <<= 1) {
            /* squaring/multiplication: Y = Y*Y */
            ammFunc(redY, redY, redY, redM, k0, redLen, redBuffer);

            /* and multiply Y = Y*X */
            if (eValue & ((mpn_limb_t)1 << (MPN_LIMB_BITS - 1))) ammFunc(redY, redY, redX, redM, k0, redLen, redBuffer);
        }

        /* execute rest bits of E */
        for (--nsE; nsE > 0; nsE--) {
            eValue = dataE[nsE - 1];

            for (n = 0; n < MPN_LIMB_BITS; n++, eValue <<= 1) {
                /* squaring: Y = Y*Y */
                ammFunc(redY, redY, redY, redM, k0, redLen, redBuffer);

                /* and multiply: Y = Y*X */
                if (eValue & ((mpn_limb_t)1 << (MPN_LIMB_BITS - 1)))
                    ammFunc(redY, redY, redX, redM, k0, redLen, redBuffer);
            }
        }
    }

    /* convert result back to regular domain */
    ZEROIZE(redT, 0, redBufferLen);
    redT[0] = 1;
    ammFunc(redY, redY, redT, redM, k0, redLen, redBuffer);
    dig52_regular(dataY, redY, cnvMM_bitsize);

    return nsM;
}

#if !defined(MPI_USE_SLIDING_WINDOW_EXP)
/*
// "safe" binary montgomery exponentiation
//
// scratch buffer structure:
//    redX[redBufferLen]
//    redT[redBufferLen]
//    redY[redBufferLen]
//    redM[redBufferLen]
//    redBuffer[redBufferLen*3]
*/
unsigned int gsMontExpBin_BNU_sscm_avx512(mpn_limb_t *dataY, const mpn_limb_t *dataX, unsigned int nsX,
                                          const mpn_limb_t *dataE, unsigned int bitsizeE, MPZ_MOD_ENGINE *pMont,
                                          mpn_limb_t *pBuffer)
{
    const mpn_limb_t *dataM = MOD_MODULUS(pMont);
    const mpn_limb_t *dataRR = MOD_MNT_R2(pMont);
    unsigned int nsM = MOD_LEN(pMont);
    unsigned int nsE = MPN_BITS_TO_LIMBS(bitsizeE);
    mpn_limb_t k0 = MOD_MNT_FACTOR(pMont);

    int modulusBitSize = BITSIZE_BNU(dataM, nsM);
    int cnvMM_bitsize = __digit_num_avx512(modulusBitSize, MPN_LIMB_BITS) * MPN_LIMB_BITS;
    int almMM_bitsize = cnvMM_bitsize + 2;
    int redLen = __digit_num_avx512(almMM_bitsize, EXP_DIGIT_SIZE_AVX512);
    int redBufferLen = num_of_variable_buff_avx512(redLen);

    /* allocate buffers */
    mpn_limb_t *redX = pBuffer;
    mpn_limb_t *redM = redX + redBufferLen;
    mpn_limb_t *redR = redM + redBufferLen;
    mpn_limb_t *redT = redR + redBufferLen;
    mpn_limb_t *redY = redT + redBufferLen;
    mpn_limb_t *redBuffer = redY + redBufferLen;

    cpAMM52 ammFunc;
    switch (modulusBitSize) {
        case 1024:
            ammFunc = AMM52x20;
            break;
        case 2048:
            ammFunc = AMM52x40;
            break;
        case 3072:
            ammFunc = AMM52x60;
            break;
        case 4096:
            ammFunc = AMM52x79;
            break;
        default:
            ammFunc = AMM52;
            break;
    }

    /* convert modulus into reduced domain */
    ZEXPAND(redBuffer, redBufferLen, dataM, nsM);
    regular_dig52(redM, redBuffer, almMM_bitsize);

    /* compute taget domain Montgomery converter RR' */
    ZEROIZE(redBuffer, 0, redBufferLen);
    SET_BIT(redBuffer, (4 * redLen * EXP_DIGIT_SIZE_AVX512 - 4 * cnvMM_bitsize));
    regular_dig52(redY, redBuffer, almMM_bitsize);

    ZEXPAND(redBuffer, redBufferLen, dataRR, nsM);
    regular_dig52(redT, redBuffer, almMM_bitsize);
    ammFunc(redT, redT, redT, redM, k0, redLen, redBuffer);
    ammFunc(redT, redT, redY, redM, k0, redLen, redBuffer);

    /* convert base to Montgomery domain */
    ZEXPAND(redY, redBufferLen /*nsX+1*/, dataX, nsX);
    regular_dig52(redX, redY, almMM_bitsize);
    ammFunc(redX, redX, redT, redM, k0, redLen, redBuffer);

    /* init result */
    ZEROIZE(redR, 0, redBufferLen);
    redR[0] = 1;
    ammFunc(redR, redR, redT, redM, k0, redLen, redBuffer);
    COPY_BNU(redY, redR, redBufferLen);

    /* execute bits of E */
    for (; nsE > 0; nsE--) {
        mpn_limb_t eValue = dataE[nsE - 1];

        int n;
        for (n = MPN_LIMB_BITS; n > 0; n--) {
            /* T = ( msb(eValue) )? X : mont(1) */
            mpn_limb_t mask = ct_test_msb(eValue);
            eValue <<= 1;
            ct_masked_copy_MPZ_ULONG(redT, mask, redX, redR, redLen);

            /* squaring: Y = Y*Y */
            ammFunc(redY, redY, redY, redM, k0, redLen, redBuffer);
            /* and multiply: Y = Y * T */
            ammFunc(redY, redY, redT, redM, k0, redLen, redBuffer);
        }
    }

    /* convert result back to regular domain */
    ZEROIZE(redT, 0, redBufferLen);
    redT[0] = 1;
    ammFunc(redY, redY, redT, redM, k0, redLen, redBuffer);
    dig52_regular(dataY, redY, cnvMM_bitsize);

    return nsM;
}

#endif /* !MPI_USE_SLIDING_WINDOW_EXP */


#if defined(MPI_USE_SLIDING_WINDOW_EXP)
/*
// "fast" fixed-size window montgomery exponentiation
//
// scratch buffer structure:
//    precomuted table of multipliers[(1<<w)*redLen]
//    redM[redBufferLen]
//    redY[redBufferLen]
//    redT[redBufferLen]
//    redE[redBufferLen]
//    redBuffer[redBufferLen*3]
*/
unsigned int gsMontExpWin_BNU_avx512(mpn_limb_t *dataY, const mpn_limb_t *dataX, unsigned int nsX,
                                     const mpn_limb_t *dataE, unsigned int bitsizeE, MPZ_MOD_ENGINE *pMont,
                                     mpn_limb_t *pBuffer)
{
    const mpn_limb_t *dataM = MOD_MODULUS(pMont);
    const mpn_limb_t *dataRR = MOD_MNT_R2(pMont);
    unsigned int nsM = MOD_LEN(pMont);
    mpn_limb_t k0 = MOD_MNT_FACTOR(pMont);

    unsigned int nsE = MPN_BITS_TO_LIMBS(bitsizeE);

    int modulusBitSize = BITSIZE_BNU(dataM, nsM);
    int cnvMM_bitsize = __digit_num_avx512(modulusBitSize, MPN_LIMB_BITS) * MPN_LIMB_BITS;
    int almMM_bitsize = cnvMM_bitsize + 2;
    int redLen = __digit_num_avx512(almMM_bitsize, EXP_DIGIT_SIZE_AVX512);
    int redBufferLen = num_of_variable_buff_avx512(redLen);

    unsigned int window = mont_exp_win_size(bitsizeE);
    mpn_limb_t wmask = (1 << window) - 1;
    unsigned int nPrecomute = 1 << window;
    int n;

    mpn_limb_t *redE = pBuffer;
    mpn_limb_t *redM = redE + redBufferLen;
    mpn_limb_t *redY = redM + redBufferLen;
    mpn_limb_t *redT = redY + redBufferLen;
    mpn_limb_t *redBuffer = redT + redBufferLen;
    mpn_limb_t *redTable = redBuffer + redBufferLen * 3;

    cpAMM52 ammFunc;
    switch (modulusBitSize) {
        case 1024:
            ammFunc = AMM52x20;
            break;
        case 2048:
            ammFunc = AMM52x40;
            break;
        case 3072:
            ammFunc = AMM52x60;
            break;
        case 4096:
            ammFunc = AMM52x79;
            break;
        default:
            ammFunc = AMM52;
            break;
    }

    /* convert modulus into reduced domain */
    ZEXPAND(redBuffer, redBufferLen, dataM, nsM);
    regular_dig52(redM, redBuffer, almMM_bitsize);

    /* compute taget domain Montgomery converter RR' */
    ZEROIZE(redBuffer, 0, redBufferLen);
    SET_BIT(redBuffer, (4 * redLen * EXP_DIGIT_SIZE_AVX512 - 4 * cnvMM_bitsize));
    regular_dig52(redY, redBuffer, almMM_bitsize);

    ZEXPAND(redBuffer, redBufferLen, dataRR, nsM);
    regular_dig52(redT, redBuffer, almMM_bitsize);
    ammFunc(redT, redT, redT, redM, k0, redLen, redBuffer);
    ammFunc(redT, redT, redY, redM, k0, redLen, redBuffer);

    /*
       pre-compute T[i] = X^i, i=0,.., 2^w-1
    */
    ZEROIZE(redY, 0, redBufferLen);
    redY[0] = 1;
    ammFunc(redY, redY, redT, redM, k0, redLen, redBuffer);
    COPY_BNU(redTable + 0, redY, redLen);

    ZEXPAND(redBuffer, redBufferLen, dataX, nsX);
    regular_dig52(redY, redBuffer, almMM_bitsize);
    ammFunc(redY, redY, redT, redM, k0, redLen, redBuffer);
    COPY_BNU(redTable + redLen, redY, redLen);

    ammFunc(redT, redY, redY, redM, k0, redLen, redBuffer);
    COPY_BNU(redTable + redLen * 2, redT, redLen);

    for (n = 3; n < nPrecomute; n++) {
        ammFunc(redT, redT, redY, redM, k0, redLen, redBuffer);
        COPY_BNU(redTable + redLen * n, redT, redLen);
    }

    /* expand exponent */
    ZEXPAND(redE, nsE + 1, dataE, nsE);
    bitsizeE = ((bitsizeE + window - 1) / window) * window;

    /* exponentiation */
    {
        /* position of the 1-st (left) window */
        int eBit = bitsizeE - window;

        /* Note:  Static analysis can generate error/warning on the expression below.

        The value of "bitSizeE" is limited, (modulusBitSize > bitSizeE > 0),
        it is checked in initialization phase by (ippsRSA_GetSizePublickey() and ippsRSA_InitPublicKey).
        Buffer "redE" assigned for copy of dataE, is 1 (64-bit) chunk longer than size of RSA modulus,
        therefore the access "*((uint32_t*)((uint16_t*)redE+ eBit/16))" is always inside the boundary.
        */
        /* extract 1-st window value */
        uint32_t eChunk = *((uint32_t *)((uint16_t *)redE + eBit / 16));
        int shift = eBit & 0xF;
        unsigned int windowVal = (unsigned int)((eChunk >> shift) & wmask);

        /* initialize result */
        ZEXPAND(redY, redBufferLen, redTable + windowVal * redLen, redLen);

        for (eBit -= window; eBit >= 0; eBit -= window) {
            /* do squaring window-times */
            for (n = 0; n < window; n++) { ammFunc(redY, redY, redY, redM, k0, redLen, redBuffer); }

            /* extract next window value */
            eChunk = *((uint32_t *)((uint16_t *)redE + eBit / 16));
            shift = eBit & 0xF;
            windowVal = (unsigned int)((eChunk >> shift) & wmask);

            /* extract precomputed value and muptiply */
            if (windowVal) { ammFunc(redY, redY, redTable + windowVal * redLen, redM, k0, redLen, redBuffer); }
        }
    }

    /* convert result back */
    ZEROIZE(redT, 0, redBufferLen);
    redT[0] = 1;
    ammFunc(redY, redY, redT, redM, k0, redLen, redBuffer);
    dig52_regular(dataY, redY, cnvMM_bitsize);

    return nsM;
}

/*
// "safe" fixed-size window montgomery exponentiation
//
// scratch buffer structure:
//    precomuted table of multipliers[(1<<w)*redLen]
//    redM[redBufferLen]
//    redY[redBufferLen]
//    redT[redBufferLen]
//    redBuffer[redBufferLen*3]
//    redE[redBufferLen]
*/
unsigned int gsMontExpWin_BNU_sscm_avx512(mpn_limb_t *dataY, const mpn_limb_t *dataX, unsigned int nsX,
                                          const mpn_limb_t *dataE, unsigned int bitsizeE, MPZ_MOD_ENGINE *pMont,
                                          mpn_limb_t *pBuffer)
{
    const mpn_limb_t *dataM = MOD_MODULUS(pMont);
    const mpn_limb_t *dataRR = MOD_MNT_R2(pMont);
    unsigned int nsM = MOD_LEN(pMont);
    mpn_limb_t k0 = MOD_MNT_FACTOR(pMont);

    unsigned int nsE = MPN_BITS_TO_LIMBS(bitsizeE);

    int modulusBitSize = MOD_BITSIZE(pMont);
    int cnvMM_bitsize = __digit_num_avx512(modulusBitSize, MPN_LIMB_BITS) * MPN_LIMB_BITS;
    int almMM_bitsize = cnvMM_bitsize + 2;
    int redLen = __digit_num_avx512(almMM_bitsize, EXP_DIGIT_SIZE_AVX512);
    int redBufferLen = num_of_variable_buff_avx512(redLen);

    unsigned int window = mont_exp_win_size(bitsizeE);
    unsigned int nPrecomute = 1 << window;
    mpn_limb_t wmask = (mpn_limb_t)(nPrecomute - 1);
    int n;

#ifdef _EXP_AVX512_DEBUG_
    mpn_limb_t dbgValue[32];
#endif

    mpn_limb_t *redTable = (mpn_limb_t *)(MPZ_ALIGNED_PTR(pBuffer, CACHE_LINE_SIZE));
    mpn_limb_t *redM = redTable + mont_scramble_buffer_size(redLen, window);
    mpn_limb_t *redY = redM + redBufferLen;
    mpn_limb_t *redT = redY + redBufferLen;
    mpn_limb_t *redBuffer = redT + redBufferLen;
    mpn_limb_t *redE = redBuffer + redBufferLen * 3;

    cpAMM52 ammFunc;
    switch (modulusBitSize) {
        case 1024:
            ammFunc = AMM52x20;
            break;
        case 2048:
            ammFunc = AMM52x40;
            break;
        case 3072:
            ammFunc = AMM52x60;
            break;
        case 4096:
            ammFunc = AMM52x79;
            break;
        default:
            ammFunc = AMM52;
            break;
    }

    /* convert modulus into reduced domain */
    ZEXPAND(redBuffer, redBufferLen, dataM, nsM);
    regular_dig52(redM, redBuffer, almMM_bitsize);

    /* compute taget domain Montgomery converter RR' */
    ZEROIZE(redBuffer, 0, redBufferLen);
    SET_BIT(redBuffer, (4 * redLen * EXP_DIGIT_SIZE_AVX512 - 4 * cnvMM_bitsize));
    regular_dig52(redY, redBuffer, almMM_bitsize);

    ZEXPAND(redBuffer, redBufferLen, dataRR, nsM);
    regular_dig52(redT, redBuffer, almMM_bitsize);
    ammFunc(redT, redT, redT, redM, k0, redLen, redBuffer);
    ammFunc(redT, redT, redY, redM, k0, redLen, redBuffer);

    /*
       pre-compute T[i] = X^i, i=0,.., 2^w-1
    */
    ZEROIZE(redY, 0, redBufferLen);
    redY[0] = 1;
    ammFunc(redY, redY, redT, redM, k0, redLen, redBuffer);
    mont_scramble_put(redTable, 0, redY, redLen, window);
#ifdef _EXP_AVX512_DEBUG_
    debugToConvMontDomain(dbgValue, redY, redM, almMM_bitsize, dataM, dataRR, nsM, k0, redBuffer);
#endif

    ZEXPAND(redBuffer, redBufferLen, dataX, nsX);
    regular_dig52(redY, redBuffer, almMM_bitsize);
    ammFunc(redY, redY, redT, redM, k0, redLen, redBuffer);
    mont_scramble_put(redTable, 1, redY, redLen, window);
#ifdef _EXP_AVX512_DEBUG_
    debugToConvMontDomain(dbgValue, redY, redM, almMM_bitsize, dataM, dataRR, nsM, k0, redBuffer);
#endif

    ammFunc(redT, redY, redY, redM, k0, redLen, redBuffer);
    mont_scramble_put(redTable, 2, redT, redLen, window);
#ifdef _EXP_AVX512_DEBUG_
    debugToConvMontDomain(dbgValue, redT, redM, almMM_bitsize, dataM, dataRR, nsM, k0, redBuffer);
#endif

    for (n = 3; n < nPrecomute; n++) {
        ammFunc(redT, redT, redY, redM, k0, redLen, redBuffer);
        mont_scramble_put(redTable, n, redT, redLen, window);
#ifdef _EXP_AVX512_DEBUG_
        debugToConvMontDomain(dbgValue, redT, redM, almMM_bitsize, dataM, dataRR, nsM, k0, redBuffer);
#endif
    }

    /* expand exponent */
    ZEXPAND(redE, nsM + 1, dataE, nsE);
    bitsizeE = ((bitsizeE + window - 1) / window) * window;

    /* exponentiation */
    {
        /* position of the 1-st (left) window */
        int eBit = bitsizeE - window;

        /* extract 1-st window value */
        uint32_t eChunk = *((uint32_t *)((uint16_t *)redE + eBit / 16));
        int shift = eBit & 0xF;
        unsigned int windowVal = (unsigned int)((eChunk >> shift) & wmask);

        /* initialize result */
        mont_scramble_get_sscm(redY, redLen, redTable, windowVal, window);
#ifdef _EXP_AVX512_DEBUG_
        debugToConvMontDomain(dbgValue, redY, redM, almMM_bitsize, dataM, dataRR, nsM, k0, redBuffer);
#endif

        for (eBit -= window; eBit >= 0; eBit -= window) {
            /* do squaring window-times */
            for (n = 0; n < window; n++) { ammFunc(redY, redY, redY, redM, k0, redLen, redBuffer); }

            /* extract next window value */
            eChunk = *((uint32_t *)((uint16_t *)redE + eBit / 16));
            shift = eBit & 0xF;
            windowVal = (unsigned int)((eChunk >> shift) & wmask);

            /* exptact precomputed value and muptiply */
            mont_scramble_get_sscm(redT, redLen, redTable, windowVal, window);
            /* muptiply */
            ammFunc(redY, redY, redT, redM, k0, redLen, redBuffer);
        }
    }

    /* convert result back */
    ZEROIZE(redT, 0, redBufferLen);
    redT[0] = 1;
    ammFunc(redY, redY, redT, redM, k0, redLen, redBuffer);
    dig52_regular(dataY, redY, cnvMM_bitsize);

    return nsM;
}
#endif /* MPI_USE_SLIDING_WINDOW_EXP */

#endif /* __ARCH32E_K0 */
