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
#ifndef MULTIPLE_PRECISION_MONTGOMERY_H
#define MULTIPLE_PRECISION_MONTGOMERY_H

#include <mpi/mpi.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    unsigned int modbits; /**< size of modulus in bit */
    unsigned int modsize; /**< size of modulus in mpn_limb_t */
    mpn_limb_t k0;        /**< low word of (1/modulus) mod R */
    mpn_limb_t *modulus;  /**< modulus */
    mpn_limb_t *montR;    /**< mont_enc(1) */
    mpn_limb_t *montRR;   /**< mont_enc(1) ^ 2 */

    mpn_optimizer_t *optimizer; /**< optimizer for montgomery operation */
} mpi_montgomery_t;

/**
 * mpi montgomery: create montgomery context
 *
 */
mpi_montgomery_t *mpi_montgomery_create(unsigned int mbits, unsigned int psize);

/**
 * mpi montgomery: destory montgomery context
 *
 */
void mpi_montgomery_destory(mpi_montgomery_t *mont);

/**
 * mpi montgomery: intialize montgomery context with modulus
 *
 */
int mpi_montgomery_set_modulus(mpi_montgomery_t *mont, const mpi_t *modulus);

/**
 * mpi montgomery: intialize montgomery context with modulus
 *
 */
int mpi_montgomery_set_modulus_bin(mpi_montgomery_t *mont, const mpn_limb_t *modulus, unsigned int mbits);

/**
 * mpi montgomery: exponentiation
 *
 */
int mpi_montgomery_exp(mpi_t *r, const mpi_t *x, const mpi_t *e, mpi_montgomery_t *mont);

/**
 * mpi montgomery: exponentiation(constant-time version)
 *
 */
int mpi_montgomery_exp_consttime(mpi_t *r, const mpi_t *x, const mpi_t *e, mpi_montgomery_t *mont);



/**
 * Multiple-Precision-Interger/Montgomery/Low-Level-API
 *
 */
/**
 * mpi montgomery: montgomery reduction
 *
 * @note:
 *   1. m0: low word of (1 / modulus) mod b
 *   2. r = T/R mod m
 */
void mpi_montgomery_reduce_bin(mpn_limb_t *r, mpn_limb_t *product, const mpn_limb_t *m, unsigned int mnum,
                               mpn_limb_t m0);

/**
 * mpi montgomery: r[] = to_mont(a[])
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpi_montgomery_enc_bin(mpn_limb_t *r, const mpn_limb_t *a, mpi_montgomery_t *mont);

/**
 * mpi montgomery: r[] = from_mont(a)
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpi_montgomery_dec_bin(mpn_limb_t *r, const mpn_limb_t *a, mpi_montgomery_t *mont);

/**
 * mpi montgomery: r = (a + b) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. length of b: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpi_montgomery_add_bin(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, mpi_montgomery_t *mont);

/**
 * mpi montgomery: r = (a - b) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. length of b: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpi_montgomery_sub_bin(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, mpi_montgomery_t *mont);

/**
 * mpi montgomery: r = -b mod m = (m - b) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpi_montgomery_neg_bin(mpn_limb_t *r, const mpn_limb_t *a, mpi_montgomery_t *mont);

/**
 * mpi montgomery: r = (a / 2) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpi_montgomery_div2_bin(mpn_limb_t *r, const mpn_limb_t *a, mpi_montgomery_t *mont);

/**
 * mpi montgomery: r = (a * 2) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpi_montgomery_mul2_bin(mpn_limb_t *r, const mpn_limb_t *a, mpi_montgomery_t *mont);

/**
 * mpi montgomery: r = (a * 3) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpi_montgomery_mul3_bin(mpn_limb_t *r, const mpn_limb_t *a, mpi_montgomery_t *mont);

/**
 * mpi montgomery: r = prod mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of rod: modsize
 *   4. memory size from the pool: N/A
 */
void mpi_montgomery_red_bin(mpn_limb_t *r, mpn_limb_t *prod, mpi_montgomery_t *mont);

/**
 * mpi montgomery: r = (a * b) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. length of b: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t) * 2
 */
void mpi_montgomery_mul_bin(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, mpi_montgomery_t *mont);

/**
 * mpi montgomery: r = (a ^ 2) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t) * 2
 */
void mpi_montgomery_sqr_bin(mpn_limb_t *r, const mpn_limb_t *a, mpi_montgomery_t *mont);

/**
 * montgomery factor k0 = -((modulus^-1 mod B) %B)
 */
mpn_limb_t mpi_montgomery_factor(mpn_limb_t m0);

/**
 * mpi montgomery: binary exponentiation
 *
 */
unsigned int mpi_montgomery_exp_bin(mpn_limb_t *y, const mpn_limb_t *x, unsigned int xsize, const mpn_limb_t *e,
                                    unsigned int ebits, mpi_montgomery_t *mont);

/**
 * mpi montgomery: binary exponentiation(consttime)
 *
 */
unsigned int mpi_montgomery_exp_consttime_bin(mpn_limb_t *y, const mpn_limb_t *x, unsigned int xsize,
                                              const mpn_limb_t *e, unsigned int ebits, mpi_montgomery_t *mont);

#if defined(__cplusplus)
}
#endif

#endif
