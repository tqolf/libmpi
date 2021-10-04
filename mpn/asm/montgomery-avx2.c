// target cpu: > __ARCH32E_L9
#include <assert.h>

#include "mpn-binary.h"
#include "mpn-montgomery.h"

#define RSA_AVX2_MIN_BITSIZE (1024)
#define RSA_AVX2_MAX_BITSIZE (13 * 1024)

#define NORM_DIGSIZE_AVX2 32 // uint32_t
#define NORM_BASE_AVX2    ((uint64_t)1 << NORM_DIGSIZE_AVX2)
#define NORM_MASK_AVX2    (NORM_BASE_AVX2 - 1)

#define EXP_DIGIT_SIZE_AVX2 (27)
#define EXP_DIGIT_BASE_AVX2 (1 << EXP_DIGIT_SIZE_AVX2)
#define EXP_DIGIT_MASK_AVX2 (EXP_DIGIT_BASE_AVX2 - 1)

/* basic operations */
void mont_mul1024_avx2(uint64_t *pR, const uint64_t *pA, const uint64_t *pB, const uint64_t *pModulus, int mLen, uint64_t m0);
void mont_mul4n_avx2(uint64_t *pR, const uint64_t *pA, const uint64_t *pB, const uint64_t *pModulus, int mLen, uint64_t m0, uint64_t *pScratchBuffer);
void mont_mul4n1_avx2(uint64_t *pR, const uint64_t *pA, const uint64_t *pB, const uint64_t *pModulus, int mLen, uint64_t m0, uint64_t *pScratchBuffer);
void mont_mul4n2_avx2(uint64_t *pR, const uint64_t *pA, const uint64_t *pB, const uint64_t *pModulus, int mLen, uint64_t m0, uint64_t *pScratchBuffer);
void mont_mul4n3_avx2(uint64_t *pR, const uint64_t *pA, const uint64_t *pB, const uint64_t *pModulus, int mLen, uint64_t m0, uint64_t *pScratchBuffer);

void mont_sqr1024_avx2(uint64_t *pR, const uint64_t *pA, const uint64_t *pModulus, int mLen, uint64_t k0, uint64_t *pBuffer);
void sqr1024_avx2(uint64_t *pR, const uint64_t *pA, int aLen, uint64_t *pBuffer);
void sqr_avx2(uint64_t *pR, const uint64_t *pA, int aLen, uint64_t *pBuffer);

void mont_red_avx2(uint64_t *pR, uint64_t *pProduct, const uint64_t *pModulus, int mLen, uint64_t k0);

/* number of "diSize" chunks in "bitSize" bit string */
MPN_INLINE int __digit_num_avx2(int bitSize, int digSize)
{
    return (bitSize + digSize - 1) / digSize;
}

/* number of "EXP_DIGIT_SIZE_AVX2" chunks in "bitSize" bit string matched for AMM */
MPN_INLINE unsigned int num_of_variable_avx2(int modulusBits)
{
    unsigned int ammBitSize = 2 + __digit_num_avx2(modulusBits, MPN_LIMB_BITS) * MPN_LIMB_BITS;
    unsigned int redNum = __digit_num_avx2(ammBitSize, EXP_DIGIT_SIZE_AVX2);
    return redNum;
}

/* buffer corresponding to num_of_variable_avx2() */
MPN_INLINE unsigned int num_of_variable_buff_avx2(int numV)
{
    return numV + 4;
}

/**
 *   converts regular (base = 2^32) representation (pRegular, regLen)
 *   into "redundant" (base = 2^DIGIT_SIZE) represenrartion (pRep, repLen)
 *
 *   note:
 *   1) repLen >= (bitsize +DIGIT_SIZE-1)/DIGIT_SIZE for complete conversion
 *   2) regular representation should expanded by at least one zero value,
 *      pre-requisite: pRegular[regLen] == 0 to make conversion correct
 *   3) caller must provide suitable lengths of regular and redundant respresentations
 *      so, conversion does correct
 */
static int regular_dig27(uint64_t *pRep27, int repLen, const uint32_t *pRegular, int regLen)
{
    /* expected number of digit in redundant representation */
    int n = __digit_num_avx2(regLen * 32, EXP_DIGIT_SIZE_AVX2);

    MPN_ASSERT(pRegular[regLen] == 0);
    {
        int redBit; /* output representatin bit */
        int i;
        for (i = 0, redBit = 0; i < n; i++, redBit += EXP_DIGIT_SIZE_AVX2) {
            int idx = redBit / NORM_DIGSIZE_AVX2;   /* input digit number */
            int shift = redBit % NORM_DIGSIZE_AVX2; /* output digit position inside input one */
            uint64_t x = ((uint64_t *)(pRegular + idx))[0];
            x >>= shift;
            pRep27[i] = x & EXP_DIGIT_MASK_AVX2;
        }

        /* expands by zeros if necessary */
        for (; i < repLen; i++) { pRep27[i] = 0; }

        return 1;
    }
}

/**
 *   converts "redundant" (base = 2^DIGIT_SIZE) representation (pRep27, repLen)
 *   into regular (base = 2^32) representation (pRegular, regLen)
 *
 *   note:
 *   caller must provide suitable lengths of regular and redundant respresentations
 *   so, conversion does correct
 */
static int dig27_regular(uint32_t *pRegular, int regLen, const uint64_t *pRep27, int repLen)
{
    int shift = EXP_DIGIT_SIZE_AVX2;
    uint64_t x = pRep27[0];

    int idx, i;
    for (i = 1, idx = 0; i < repLen && idx < regLen; i++) {
        x += pRep27[i] << shift;
        shift += EXP_DIGIT_SIZE_AVX2;
        if (shift >= NORM_DIGSIZE_AVX2) {
            pRegular[idx++] = (uint32_t)(x & NORM_MASK_AVX2);
            x >>= NORM_DIGSIZE_AVX2;
            shift -= NORM_DIGSIZE_AVX2;
        }
    }

    if (idx < regLen) pRegular[idx++] = (uint32_t)x;

    return idx;
}

/* mont_mul wraper */
MPN_INLINE void __mont_mul_avx2(uint64_t *pR, const uint64_t *pA, const uint64_t *pB, const uint64_t *pModulus, int mLen, uint64_t k0, uint64_t *pBuffer)
{
    if (mLen == 38) { /* corresponds to 1024-bit regular representation */
        mont_mul1024_avx2(pR, pA, pB, pModulus, mLen, k0);
    } else {
        int caseFlag = mLen % 4;
        switch (caseFlag) {
            case 1:
                mont_mul4n1_avx2(pR, pA, pB, pModulus, mLen, k0, pBuffer);
                break;
            case 2:
                mont_mul4n2_avx2(pR, pA, pB, pModulus, mLen, k0, pBuffer);
                break;
            case 3:
                mont_mul4n3_avx2(pR, pA, pB, pModulus, mLen, k0, pBuffer);
                break;
            default:
                mont_mul4n_avx2(pR, pA, pB, pModulus, mLen, k0, pBuffer);
                break;
        }
    }
}

/* mont_sqr wraper */
MPN_INLINE void __mont_sqr_avx2(uint64_t *pR, const uint64_t *pA, const uint64_t *pModulus, int mLen, uint64_t k0, uint64_t *pBuffer)
{
    if (mLen == 38) /* corresponds to 1024-bit regular representation */
        mont_sqr1024_avx2(pR, pA, pModulus, mLen, k0, pBuffer);
    else {
        sqr_avx2(pBuffer, pA, mLen, pBuffer);
        mont_red_avx2(pR, pBuffer, pModulus, mLen, k0);
    }
}

#if !defined(MPI_USE_SLIDING_WINDOW_EXP)
unsigned int mont_exp_bin_buffer_avx2(int modulusBits)
{
    unsigned int redNum = num_of_variable_avx2(modulusBits);       /* "sizeof" variable */
    unsigned int redBufferNum = num_of_variable_buff_avx2(redNum); /* "sizeof" variable  buffer */
    return redBufferNum * 8;
}
#endif /* !MPI_USE_SLIDING_WINDOW_EXP */

#if defined(MPI_USE_SLIDING_WINDOW_EXP)
unsigned int mont_exp_win_buffer_avx2(int modulusBits)
{
    unsigned int w = mont_exp_win_size(modulusBits);

    unsigned int redNum = num_of_variable_avx2(modulusBits);       /* "sizeof" variable */
    unsigned int redBufferNum = num_of_variable_buff_avx2(redNum); /* "sizeof" variable  buffer */

    unsigned int bufferNum = CACHE_LINE_SIZE / (int32_t)sizeof(mpn_limb_t) + mont_scramble_buffer_size(redNum, w) /* pre-computed table */
                             + redBufferNum * 7;                                                                  /* addition 7 variables */
    return bufferNum;
}
#endif /* MPI_USE_SLIDING_WINDOW_EXP */


#if !defined(MPI_USE_SLIDING_WINDOW_EXP)
#define SET_BIT(ptr, nbit) (((uint8_t *)(ptr))[(nbit) / 8] |= ((1 << ((nbit) % 8)) & 0xFF))

/**
 * "fast" binary montgomery exponentiation
 *
 * scratch buffer structure:
 *    redX[redBufferLen]
 *    redT[redBufferLen]
 *    redY[redBufferLen]
 *    redM[redBufferLen]
 *    redBuffer[redBufferLen*3]
 */
unsigned int mont_exp_bin_avx2(mpn_limb_t *dataY, const mpn_limb_t *dataX, unsigned int nsX, const mpn_limb_t *dataE, unsigned int bitsizeE, const mpn_limb_t *dataM, unsigned int bitsizeM, const mpn_limb_t *dataRR, mpn_limb_t k0,
                               mpn_limb_t *pBuffer)
{
    unsigned int nsM = MPN_BITS_TO_LIMBS(bitsizeM);
    unsigned int nsE = MPN_BITS_TO_LIMBS(bitsizeE);

    int convModulusBitSize = __digit_num_avx2(bitsizeM, MPN_LIMB_BITS) * MPN_LIMB_BITS;
    int modulusLen32 = (bitsizeM + 31) / 32;
    int redLen = __digit_num_avx2(convModulusBitSize + 2, EXP_DIGIT_SIZE_AVX2);
    int redBufferLen = num_of_variable_buff_avx2(redLen);

    /* allocate buffers */
    mpn_limb_t *redX = pBuffer;
    mpn_limb_t *redT = redX + redBufferLen;
    mpn_limb_t *redY = redT + redBufferLen;
    mpn_limb_t *redM = redY + redBufferLen;
    mpn_limb_t *redBuffer = redM + redBufferLen;

    /* convert modulus into reduced domain */
    ZEXPAND(redT, nsM + 1, dataM, nsM);
    regular_dig27(redM, redBufferLen, (uint32_t *)redT, modulusLen32);

    /* compute taget domain Montgomery converter RR' */
    ZEROIZE(redT, 0, redBufferLen);
    SET_BIT(redT, (4 * redLen * EXP_DIGIT_SIZE_AVX2 - 4 * convModulusBitSize));
    regular_dig27(redY, redBufferLen, (uint32_t *)redT, modulusLen32);

    ZEXPAND(redX, nsM + 1, dataRR, nsM);
    regular_dig27(redT, redBufferLen, (uint32_t *)redX, modulusLen32);
    __mont_sqr_avx2(redT, redT, redM, redLen, k0, redBuffer);
    __mont_mul_avx2(redT, redT, redY, redM, redLen, k0, redBuffer);

    /* convert base to Montgomery domain */
    ZEXPAND(redY, redBufferLen /*nsX+1*/, dataX, nsX);
    regular_dig27(redX, redBufferLen, (uint32_t *)redY, nsX * sizeof(mpn_limb_t) / sizeof(uint32_t));
    __mont_mul_avx2(redX, redX, redT, redM, redLen, k0, redBuffer);

    /* init result */
    COPY(redY, redX, redLen);

    nsE = mpn_limbs(dataE, nsE);
    {
        /* execute most significant part pE */
        mpn_limb_t eValue = dataE[nsE - 1];
        int n = mpn_limb_nlz_consttime(eValue) + 1;

        eValue <<= n;
        for (; n < MPN_LIMB_BITS; n++, eValue <<= 1) {
            /* squaring/multiplication: Y = Y*Y */
            __mont_sqr_avx2(redY, redY, redM, redLen, k0, redBuffer);

            /* and multiply Y = Y*X */
            if (eValue & ((mpn_limb_t)1 << (MPN_LIMB_BITS - 1))) __mont_mul_avx2(redY, redY, redX, redM, redLen, k0, redBuffer);
        }

        /* execute rest bits of E */
        for (--nsE; nsE > 0; nsE--) {
            eValue = dataE[nsE - 1];

            for (n = 0; n < MPN_LIMB_BITS; n++, eValue <<= 1) {
                /* squaring: Y = Y*Y */
                __mont_sqr_avx2(redY, redY, redM, redLen, k0, redBuffer);

                /* and multiply: Y = Y*X */
                if (eValue & ((mpn_limb_t)1 << (MPN_LIMB_BITS - 1))) __mont_mul_avx2(redY, redY, redX, redM, redLen, k0, redBuffer);
            }
        }
    }

    /* convert result back to regular domain */
    ZEROIZE(redT, 0, redBufferLen);
    redT[0] = 1;
    __mont_mul_avx2(redY, redY, redT, redM, redLen, k0, redBuffer);
    dig27_regular((uint32_t *)dataY, nsM * sizeof(mpn_limb_t) / sizeof(uint32_t), redY, redLen);

    return nsM;
}

/**
 * "safe" binary montgomery exponentiation
 *
 * scratch buffer structure:
 *    redX[redBufferLen]
 *    redT[redBufferLen]
 *    redY[redBufferLen]
 *    redM[redBufferLen]
 *    redBuffer[redBufferLen*3]
 */
unsigned int mont_exp_bin_sscm_avx2(mpn_limb_t *dataY, const mpn_limb_t *dataX, unsigned int nsX, const mpn_limb_t *dataE, unsigned int bitsizeE, const mpn_limb_t *dataM, unsigned int bitsizeM, const mpn_limb_t *dataRR, mpn_limb_t k0,
                                    mpn_limb_t *pBuffer)
{
    unsigned int nsM = MPN_BITS_TO_LIMBS(bitsizeM);
    unsigned int nsE = MPN_BITS_TO_LIMBS(bitsizeE);

    int convModulusBitSize = __digit_num_avx2(bitsizeM, MPN_LIMB_BITS) * MPN_LIMB_BITS;
    int modulusLen32 = (bitsizeM + 31) / 32;
    int redLen = __digit_num_avx2(convModulusBitSize + 2, EXP_DIGIT_SIZE_AVX2);
    int redBufferLen = num_of_variable_buff_avx2(redLen);

    /* allocate buffers */
    mpn_limb_t *redX = pBuffer;
    mpn_limb_t *redM = redX + redBufferLen;
    mpn_limb_t *redR = redM + redBufferLen;
    mpn_limb_t *redT = redR + redBufferLen;
    mpn_limb_t *redY = redT + redBufferLen;
    mpn_limb_t *redBuffer = redY + redBufferLen;

    /* convert modulus into reduced domain */
    ZEXPAND(redT, nsM + 1, dataM, nsM);
    regular_dig27(redM, redBufferLen, (uint32_t *)redT, modulusLen32);

    /* compute taget domain Montgomery converter RR' */
    ZEROIZE(redT, 0, redBufferLen);
    SET_BIT(redT, (4 * redLen * EXP_DIGIT_SIZE_AVX2 - 4 * convModulusBitSize));
    regular_dig27(redY, redBufferLen, (uint32_t *)redT, modulusLen32);

    ZEXPAND(redX, nsM + 1, dataRR, nsM);
    regular_dig27(redT, redBufferLen, (uint32_t *)redX, modulusLen32);
    __mont_sqr_avx2(redT, redT, redM, redLen, k0, redBuffer);
    __mont_mul_avx2(redT, redT, redY, redM, redLen, k0, redBuffer);

    /* convert base to Montgomery domain */
    ZEXPAND(redY, redBufferLen /*nsX+1*/, dataX, nsX);
    regular_dig27(redX, redBufferLen, (uint32_t *)redY, nsX * sizeof(mpn_limb_t) / sizeof(uint32_t));
    __mont_mul_avx2(redX, redX, redT, redM, redLen, k0, redBuffer);

    /* init result */
    ZEROIZE(redR, 0, redBufferLen);
    redR[0] = 1;
    __mont_mul_avx2(redR, redR, redT, redM, redLen, k0, redBuffer);
    COPY(redY, redR, redBufferLen);

    /* execute bits of E */
    for (; nsE > 0; nsE--) {
        mpn_limb_t eValue = dataE[nsE - 1];

        int n;
        for (n = MPN_LIMB_BITS; n > 0; n--) {
            /* T = ( msb(eValue) )? X : mont(1) */
            mpn_limb_t mask = mpn_limb_test_msb_consttime(eValue);
            eValue <<= 1;
            mpn_masked_copy_consttime(redT, redX, redR, redLen, mask);

            /* squaring: Y = Y^2 */
            __mont_sqr_avx2(redY, redY, redM, redLen, k0, redBuffer);
            /* and multiply: Y = Y * T */
            __mont_mul_avx2(redY, redY, redT, redM, redLen, k0, redBuffer);
        }
    }

    /* convert result back to regular domain */
    ZEROIZE(redT, 0, redBufferLen);
    redT[0] = 1;
    __mont_mul_avx2(redY, redY, redT, redM, redLen, k0, redBuffer);
    dig27_regular((uint32_t *)dataY, nsM * sizeof(mpn_limb_t) / sizeof(uint32_t), redY, redLen);

    return nsM;
}
#endif /* !MPI_USE_SLIDING_WINDOW_EXP */


#if defined(MPI_USE_SLIDING_WINDOW_EXP)
/**
 * "fast" fixed-size window montgomery exponentiation
 *
 * scratch buffer structure:
 *    precomuted table of multipliers[(1<<w)*redLen]
 *    redM[redBufferLen]
 *    redY[redBufferLen]
 *    redT[redBufferLen]
 *    redBuffer[redBufferLen*3]
 *    redE[redBufferLen]
 */
unsigned int mont_exp_win_avx2(mpn_limb_t *dataY, const mpn_limb_t *dataX, unsigned int nsX, const mpn_limb_t *dataE, unsigned int bitsizeE, const mpn_limb_t *dataM, unsigned int bitsizeM, const mpn_limb_t *dataRR, mpn_limb_t k0,
                               mpn_limb_t *pBuffer)
{
    unsigned int nsM = MPN_BITS_TO_LIMBS(bitsizeM);
    unsigned int nsE = MPN_BITS_TO_LIMBS(bitsizeE);

    int convModulusBitSize = __digit_num_avx2(bitsizeM, MPN_LIMB_BITS) * MPN_LIMB_BITS;
    int modulusLen32 = (bitsizeM + 31) / 32;
    int redLen = __digit_num_avx2(convModulusBitSize + 2, EXP_DIGIT_SIZE_AVX2);
    int redBufferLen = num_of_variable_buff_avx2(redLen);

    unsigned int window = mont_exp_win_size(bitsizeE);
    mpn_limb_t wmask = (1 << window) - 1;
    unsigned int nPrecomute = 1 << window;
    int n;

    mpn_limb_t *redTable = pBuffer;
    mpn_limb_t *redM = redTable + mont_scramble_buffer_size(redLen, window);
    mpn_limb_t *redY = redM + redBufferLen;
    mpn_limb_t *redT = redY + redBufferLen;
    mpn_limb_t *redBuffer = redT + redBufferLen;
    mpn_limb_t *redE = redBuffer + redBufferLen * 3;

    /* convert modulus into reduced domain */
    ZEXPAND(redE, nsM + 1, dataM, nsM);
    regular_dig27(redM, redBufferLen, (uint32_t *)redE, modulusLen32);

    /* compute taget domain Montgomery converter RR' */
    ZEROIZE(redT, 0, redBufferLen);
    SET_BIT(redT, (4 * redLen * EXP_DIGIT_SIZE_AVX2 - 4 * convModulusBitSize));
    regular_dig27(redY, redBufferLen, (uint32_t *)redT, modulusLen32);

    ZEXPAND(redE, nsM + 1, dataRR, nsM);
    regular_dig27(redT, redBufferLen, (uint32_t *)redE, modulusLen32);
    __mont_sqr_avx2(redT, redT, redM, redLen, k0, redBuffer);
    __mont_mul_avx2(redT, redT, redY, redM, redLen, k0, redBuffer);

    /*
       pre-compute T[i] = X^i, i=0,.., 2^w-1
    */
    ZEROIZE(redY, 0, redBufferLen);
    redY[0] = 1;
    __mont_mul_avx2(redY, redY, redT, redM, redLen, k0, redBuffer);
    COPY(redTable + 0, redY, redLen);

    ZEXPAND(redE, redBufferLen /*nsX+1*/, dataX, nsX);
    regular_dig27(redY, redBufferLen, (uint32_t *)redE, nsX * (int32_t)(sizeof(mpn_limb_t) / sizeof(uint32_t)));
    __mont_mul_avx2(redY, redY, redT, redM, redLen, k0, redBuffer);
    COPY(redTable + redLen, redY, redLen);

    __mont_sqr_avx2(redT, redY, redM, redLen, k0, redBuffer);
    COPY(redTable + redLen * 2, redT, redLen);

    for (n = 3; n < nPrecomute; n++) {
        __mont_mul_avx2(redT, redT, redY, redM, redLen, k0, redBuffer);
        COPY(redTable + redLen * n, redT, redLen);
    }

    /* expand exponent */
    ZEXPAND(redE, nsE + 1, dataE, nsE);
    bitsizeE = ((bitsizeE + window - 1) / window) * window;

    /* exponentiation */
    {
        /* position of the 1-st (left) window */
        int eBit = bitsizeE - window;

        /* Note:  Static analysis can generate error/warning on the expression below.

        The value of "bitSizeE" is limited, (bitsizeM > bitSizeE > 0),
        it is checked in initialization phase by (ippsRSA_GetSizePublickey() and ippsRSA_InitPublicKey).
        Buffer "redE" assigned for copy of dataE, is 1 (64-bit) chunk longer than size of RSA modulus,
        therefore the access "*((uint32_t*)((uint16_t*)redE+ eBit/16))" is always inside the boundary.
        */
        /* extract 1-st window value */
        uint32_t eChunk = *((uint32_t *)((uint16_t *)redE + eBit / 16));
        int shift = eBit & 0xF;
        unsigned int windowVal = (unsigned int)((eChunk >> shift) & wmask);

        /* initialize result */
        COPY(redY, redTable + windowVal * redLen, redLen);

        for (eBit -= window; eBit >= 0; eBit -= window) {
            /* do squaring window-times */
            for (n = 0; n < window; n++) { __mont_sqr_avx2(redY, redY, redM, redLen, k0, redBuffer); }
            /* extract next window value */
            eChunk = *((uint32_t *)((uint16_t *)redE + eBit / 16));
            shift = eBit & 0xF;
            windowVal = (unsigned int)((eChunk >> shift) & wmask);

            /* precomputed value muptiplication */
            if (windowVal) { __mont_mul_avx2(redY, redY, redTable + windowVal * redLen, redM, redLen, k0, redBuffer); }
        }
    }

    /* convert result back */
    ZEROIZE(redT, 0, redBufferLen);
    redT[0] = 1;
    __mont_mul_avx2(redY, redY, redT, redM, redLen, k0, redBuffer);
    dig27_regular((uint32_t *)dataY, nsM * (int32_t)(sizeof(mpn_limb_t) / sizeof(uint32_t)), redY, redLen);

    return nsM;
}

/**
 * "safe" fixed-size window montgomery exponentiation
 *
 * scratch buffer structure:
 *    precomuted table of multipliers[(1<<w)*redLen]
 *    redM[redBufferLen]
 *    redY[redBufferLen]
 *    redT[redBufferLen]
 *    redBuffer[redBufferLen*3]
 *    redE[redBufferLen]
 */
unsigned int mont_exp_win_sscm_avx2(mpn_limb_t *dataY, const mpn_limb_t *dataX, unsigned int nsX, const mpn_limb_t *dataE, unsigned int bitsizeE, const mpn_limb_t *dataM, unsigned int bitsizeM, const mpn_limb_t *dataRR, mpn_limb_t k0,
                                    mpn_limb_t *pBuffer)
{
    unsigned int nsM = MPN_BITS_TO_LIMBS(bitsizeM);
    unsigned int nsE = MPN_BITS_TO_LIMBS(bitsizeE);

    int convModulusBitSize = __digit_num_avx2(bitsizeM, MPN_LIMB_BITS) * MPN_LIMB_BITS;
    int modulusLen32 = (bitsizeM + 31) / 32;
    int redLen = __digit_num_avx2(convModulusBitSize + 2, EXP_DIGIT_SIZE_AVX2);
    int redBufferLen = num_of_variable_buff_avx2(redLen);

    unsigned int window = mont_exp_win_size(bitsizeE);
    unsigned int nPrecomute = 1 << window;
    mpn_limb_t wmask = (mpn_limb_t)(nPrecomute - 1);
    int n;

    mpn_limb_t *redTable = (mpn_limb_t *)(MPZ_ALIGNED_PTR((pBuffer), CACHE_LINE_SIZE));
    mpn_limb_t *redM = redTable + mont_scramble_buffer_size(redLen, window);
    mpn_limb_t *redY = redM + redBufferLen;
    mpn_limb_t *redT = redY + redBufferLen;
    mpn_limb_t *redBuffer = redT + redBufferLen;
    mpn_limb_t *redE = redBuffer + redBufferLen * 3;

    /* convert modulus into reduced domain */
    ZEXPAND(redE, nsM + 1, dataM, nsM);
    regular_dig27(redM, redBufferLen, (uint32_t *)redE, modulusLen32);

    /* compute taget domain Montgomery converter RR' */
    ZEROIZE(redT, 0, redBufferLen);
    SET_BIT(redT, (4 * redLen * EXP_DIGIT_SIZE_AVX2 - 4 * convModulusBitSize));
    regular_dig27(redY, redBufferLen, (uint32_t *)redT, modulusLen32);

    ZEXPAND(redE, nsM + 1, dataRR, nsM);
    regular_dig27(redT, redBufferLen, (uint32_t *)redE, modulusLen32);
    __mont_sqr_avx2(redT, redT, redM, redLen, k0, redBuffer);
    __mont_mul_avx2(redT, redT, redY, redM, redLen, k0, redBuffer);

    /*
       pre-compute T[i] = X^i, i=0,.., 2^w-1
    */
    ZEROIZE(redY, 0, redBufferLen);
    redY[0] = 1;
    __mont_mul_avx2(redY, redY, redT, redM, redLen, k0, redBuffer);
    mont_scramble_put(redTable, 0, redY, redLen, window);

    ZEXPAND(redE, redBufferLen /*nsX+1*/, dataX, nsX);
    regular_dig27(redY, redBufferLen, (uint32_t *)redE, nsX * (int32_t)(sizeof(mpn_limb_t) / sizeof(uint32_t)));
    __mont_mul_avx2(redY, redY, redT, redM, redLen, k0, redBuffer);
    mont_scramble_put(redTable, 1, redY, redLen, window);

    __mont_sqr_avx2(redT, redY, redM, redLen, k0, redBuffer);
    mont_scramble_put(redTable, 2, redT, redLen, window);

    for (n = 3; n < nPrecomute; n++) {
        __mont_mul_avx2(redT, redT, redY, redM, redLen, k0, redBuffer);
        mont_scramble_put(redTable, n, redT, redLen, window);
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

        for (eBit -= window; eBit >= 0; eBit -= window) {
            /* do squaring window-times */
            for (n = 0; n < window; n++) { __mont_sqr_avx2(redY, redY, redM, redLen, k0, redBuffer); }

            /* extract next window value */
            eChunk = *((uint32_t *)((uint16_t *)redE + eBit / 16));
            shift = eBit & 0xF;
            windowVal = (unsigned int)((eChunk >> shift) & wmask);
            /* exptact precomputed value and muptiply */
            mont_scramble_get_sscm(redT, redLen, redTable, windowVal, window);
            __mont_mul_avx2(redY, redY, redT, redM, redLen, k0, redBuffer);
        }
    }

    /* convert result back */
    ZEROIZE(redT, 0, redBufferLen);
    redT[0] = 1;
    __mont_mul_avx2(redY, redY, redT, redM, redLen, k0, redBuffer);
    dig27_regular((uint32_t *)dataY, nsM * (int32_t)(sizeof(mpn_limb_t) / sizeof(uint32_t)), redY, redLen);

    return nsM;
}
#endif /* MPI_USE_SLIDING_WINDOW_EXP */
