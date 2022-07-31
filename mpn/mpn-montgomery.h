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

#include <mpn/mpn-conf.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    mpn_size_t modbits;  /**< size of modulus in bit */
    mpn_size_t modsize;  /**< size of modulus in mpn_limb_t */
    mpn_limb_t k0;       /**< low word of (1/modulus) mod R */
    mpn_limb_t *modulus; /**< modulus */
    mpn_limb_t *montR;   /**< mont_enc(1) */
    mpn_limb_t *montRR;  /**< mont_enc(1) ^ 2 */

    mpn_optimizer_t *optimizer; /**< optimizer for montgomery operation */
} mpn_montgomery_t;

/**
 * mpn montgomery: create montgomery context
 *
 */
mpn_montgomery_t *mpn_montgomery_create(mpn_size_t mbits, mpn_size_t psize);

/**
 * mpn montgomery: destory montgomery context
 *
 */
void mpn_montgomery_destory(mpn_montgomery_t *mont);

/**
 * mpn montgomery: intialize montgomery context with modulus
 *
 */
int mpn_montgomery_set_modulus_bin(mpn_montgomery_t *mont, const mpn_limb_t *modulus, mpn_size_t mbits);

/**
 * mpn montgomery: montgomery reduction
 *
 * @note:
 *   1. m0: low word of (1 / modulus) mod b
 *   2. r = T/R mod m
 */
void mpn_montgomery_reduce_bin(mpn_limb_t *r, mpn_limb_t *product, const mpn_limb_t *m, mpn_size_t mnum, mpn_limb_t m0);

/**
 * mpn montgomery: r[] = to_mont(a[])
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpn_montgomery_encode(mpn_limb_t *r, const mpn_limb_t *a, mpn_montgomery_t *mont);

/**
 * mpn montgomery: r[] = from_mont(a)
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpn_montgomery_decode(mpn_limb_t *r, const mpn_limb_t *a, mpn_montgomery_t *mont);

/**
 * mpn montgomery: r = (a + b) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. length of b: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpn_montgomery_add(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, mpn_montgomery_t *mont);

/**
 * mpn montgomery: r = (a - b) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. length of b: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpn_montgomery_sub(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, mpn_montgomery_t *mont);

/**
 * mpn montgomery: r = -b mod m = (m - b) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpn_montgomery_negative(mpn_limb_t *r, const mpn_limb_t *a, mpn_montgomery_t *mont);

/**
 * mpn montgomery: r = (a / 2) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpn_montgomery_halve(mpn_limb_t *r, const mpn_limb_t *a, mpn_montgomery_t *mont);

/**
 * mpn montgomery: r = (a * 2) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpn_montgomery_double(mpn_limb_t *r, const mpn_limb_t *a, mpn_montgomery_t *mont);

/**
 * mpn montgomery: r = (a * 3) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t)
 */
void mpn_montgomery_triple(mpn_limb_t *r, const mpn_limb_t *a, mpn_montgomery_t *mont);

/**
 * mpn montgomery: r = prod mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of rod: modsize
 *   4. memory size from the pool: N/A
 */
void mpn_montgomery_reduce(mpn_limb_t *r, mpn_limb_t *prod, mpn_montgomery_t *mont);

/**
 * mpn montgomery: r = (a * b) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   3. length of b: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t) * 2
 */
void mpn_montgomery_mul(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, mpn_montgomery_t *mont);

/**
 * mpn montgomery: r = (a ^ 2) mod m
 *
 * @requirements:
 *   1. length of r: modsize
 *   2. length of a: modsize
 *   4. memory size from the pool: modsize * sizeof(mpn_limb_t) * 2
 */
void mpn_montgomery_square(mpn_limb_t *r, const mpn_limb_t *a, mpn_montgomery_t *mont);

/**
 * montgomery factor k0 = -((modulus^-1 mod B) %B)
 */
mpn_limb_t mpn_montgomery_factor(mpn_limb_t m0);

/**
 * mpn montgomery: binary exponentiation
 *
 */
mpn_size_t mpn_montgomery_exp(mpn_limb_t *y, const mpn_limb_t *x, mpn_size_t xsize, const mpn_limb_t *e,
                              mpn_size_t ebits, mpn_montgomery_t *mont);

/**
 * mpn montgomery: binary exponentiation(consttime)
 *
 */
mpn_size_t mpn_montgomery_exp_consttime(mpn_limb_t *y, const mpn_limb_t *x, mpn_size_t xsize, const mpn_limb_t *e,
                                        mpn_size_t ebits, mpn_montgomery_t *mont);

#if defined(__cplusplus)
}
#endif

#endif
