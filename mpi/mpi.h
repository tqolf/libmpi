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
 * @brief: multiple precision integer: configurations, macros, and prototypes
 *
 * @note:
 *    1. Assume that all variable representing size will never overflow
 */

#ifndef MULTIPLE_PRECISION_INTEGER_H
#define MULTIPLE_PRECISION_INTEGER_H

#include <mpi/mpi-conf.h>
#include <mpi/mpi-optimizer.h>

#if defined(__cplusplus)
extern "C" {
#endif

/** High-Level APIs */
/**
 * create mpi with expected bits |bits| to reserve
 *
 * |bits| == 0, to create empty room
 *
 * @performance: Locality of reference and Cacheline alignment
 *   mpi_t and this->data will be allocated as a continuous memory chunk
 */
mpi_t *mpi_create(unsigned int bits);

/**
 * create mpi(detached) with expected bits |bits| to reserve
 *
 * |bits| == 0, to create empty room
 */
mpi_t *mpi_create_detached(unsigned int bits);

/**
 *  duplicate big-numer |a|
 */
mpi_t *mpi_dup(const mpi_t *a);

/**
 * clear and release mpi |v|
 */
void mpi_destory(mpi_t *v);

/**
 * make mpi with given chunk
 */
void mpi_make(mpi_t *r, mpi_limb_t *data, unsigned int size);

/**
 *  copy big-numer |a| to |r|
 *
 * @note:
 *   1. resize |r| to proper size before copy
 */
int mpi_copy(mpi_t *r, const mpi_t *a);

/**
 * compare mpi |a| and |b|
 *   0, if |a| = |b|
 *   1, if |a| > |b|
 *  -1, if |a| < |b|
 * otherwise, error code
 */
int mpi_cmp(const mpi_t *a, const mpi_t *b);

/**
 * get bit size of mpi |a|(constant-time version)
 *
 * @note:
 *   1. 0, if a is NULL
 */
unsigned int mpi_bits(const mpi_t *a);

/**
 * get byte size of mpi |a|(constant-time version)
 *
 * @note:
 *   1. 0, if a is NULL
 */
unsigned int mpi_bytes(const mpi_t *a);

/**
 * get max bit size of mpi |a|(constant-time version)
 *
 * @note:
 *   1. 0, if a is NULL
 */
unsigned int mpi_max_bits(const mpi_t *a);

/**
 * get max byte size of mpi |a|(constant-time version)
 *
 * @note:
 *   1. 0, if a is NULL
 */
unsigned int mpi_max_bytes(const mpi_t *a);

/**
 * mpi: expand mpi to expected bits |bits|
 *
 * @note:
 *   1. maybe fail when no enough memory or invalid size given
 */
mpi_t *mpi_expand(mpi_t *v, unsigned int bits);

/**
 * resize mpi to expected bits |bits|
 *
 * @note:
 *   1. maybe fail when no enough memory or invalid size given
 *
 */
mpi_t *mpi_resize(mpi_t *v, unsigned int bits);

/**
 * zeroize mpi |v|
 */
int mpi_zeroize(mpi_t *v);

/**
 * set mpi |r| to unsigned sigle-precision integer |v|
 */
int mpi_set_limb(mpi_t *r, mpi_limb_t v);

/**
 *  initialize mpi |v| from octets |buff|/|bufflen|
 *
 * @note:
 *   1. if *|v| is NULL, mpi will be created with proper size
 *   2. if *|v| isn't NULL, mpi-number will be resized, and maybe *|v| will be set to a new memory chunk
 */
int mpi_from_octets(mpi_t **v, const unsigned char *buff, unsigned int bufflen);

/**
 *  convert mpi to big-endian octets
 */
int mpi_to_octets(const mpi_t *a, unsigned char *out, unsigned int outsize, unsigned int *outlen);

/**
 *  initialize mpi |v| from hex-string |a|
 */
int mpi_from_string(mpi_t **v, const char *a);

/**
 *  convert mpi to string
 *
 * @note:
 *   1. FREE the return pointer after usage
 */
char *mpi_to_string(const mpi_t *v);

/**
 * mpi addition: |r| = |a| + |b|
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: MAX(bit_size(a), bit_size(b)) + 1
 */
int mpi_add(mpi_t *r, const mpi_t *a, const mpi_t *b);

/**
 * mpi addition: |r| = |a| + w
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: MAX(bit_size(a), bit_size(w)) + 1
 */
int mpi_add_limb(mpi_t *r, const mpi_t *a, mpi_limb_t w);

/**
 * mpi subtraction: |r| = |a| - |b|
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: MAX(bit_size(a), bit_size(b))
 *   2. make sure |a| >= |b| 'cause negative mpi not supported till now
 */
int mpi_sub(mpi_t *r, const mpi_t *a, const mpi_t *b);

/**
 * mpi subtraction: |r| = |a| - w
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: MAX(bit_size(a), bit_size(w))
 */
int mpi_sub_limb(mpi_t *r, const mpi_t *a, mpi_limb_t w);

/**
 * mpi multiplication: |r| = |a| * |b|
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: bit_size(a) + bit_size(b) + MPI_LIMB_BITS
 */
int mpi_mul(mpi_t *r, const mpi_t *a, const mpi_t *b);

/**
 * mpi multiplication: |r| = |a| * |b|
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: bit_size(a) + bit_size(b)
 */
int mpi_mul_limb(mpi_t *r, const mpi_t *a, mpi_limb_t b);

/**
 * mpi square: |r| = |a| ^ 2
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: 2 * bit_size(a)
 */
int mpi_sqr(mpi_t *r, const mpi_t *a);

/**
 * mpi division: |q|, |r| = |x| / |y|, |x| = |q| * |y| + |r|(0 <= |r| < |y|)
 *
 * @note:
 *   1. make sure room of |q|, |r| is enough to store the result
 *      minimal advise size: bit_size(r) = bit_size(y)
 */
int mpi_div(mpi_t *q, mpi_t *r, const mpi_t *x, const mpi_t *y);

/**
 * mpi division: q, r = a / w
 */
mpi_limb_t mpi_div_limb(mpi_t *a, mpi_limb_t w);

/**
 * mpi modular: r = a mod m
 */
mpi_limb_t mpi_mod_limb(const mpi_t *a, mpi_limb_t w);

/**
 * greatest common divisor
 */
int mpi_gcd(mpi_t *r, const mpi_t *a, const mpi_t *b, mpi_optimizer_t *optimizer);

/**
 * greatest common divisor(constant-time version)
 */
int mpi_gcd_consttime(mpi_t *r, const mpi_t *a, const mpi_t *b, mpi_optimizer_t *optimizer);

/**
 * mpi modular: r = a mod m
 */
int mpi_mod(mpi_t *r, const mpi_t *a, const mpi_t *m);

/**
 * mpi exponentiation: r = g ^ e
 */
int mpi_exp(mpi_t *r, const mpi_t *g, const mpi_t *e);

/**
 * mpi exponentiation(word): r = g ^ e
 */
int mpi_exp_limb(mpi_t *r, const mpi_t *g, mpi_limb_t e);

/**
 * get bit
 */
int mpi_get_bit(const mpi_t *a, unsigned int n);

/**
 * set bit
 */
int mpi_set_bit(const mpi_t *a, unsigned int n);

/**
 * clr bit
 */
int mpi_clr_bit(const mpi_t *a, unsigned int n);

/**
 * left-shift: |r| = |a| << n
 */
int mpi_lshift(mpi_t *r, const mpi_t *a, unsigned int n);

/**
 * right-shift: |r| = |a| >> n
 */
int mpi_rshift(mpi_t *r, const mpi_t *a, unsigned int n);

/**
 * conditional swap(constant-time version)
 */
int mpi_swap_consttime(unsigned condition, mpi_t *a, mpi_t *b, unsigned int n);

/**
 * mpi(prime): test if a is a prime
 *
 * @note:
 *   1. return 0 if the number is composite
 *      1 if it is prime with an error probability of less than 0.25^checks
 */
int mpi_is_prime(const mpi_t *a, unsigned int checks, unsigned do_trial_division, mpi_optimizer_t *optimizer, int (*rand_bytes)(void *, unsigned char *, unsigned int), void *rand_state);

/**
 * mpi(prime): enerates a pseudo-random prime number of at least bit length |bits|
 *
 * @note:
 *   1. The returned number is probably prime with a negligible error.
 *   2. If |add| is NULL the returned prime number will have exact bit length |bits| with the top most two bits set.
 *   3. The prime may have to fulfill additional requirements for use in Diffie-Hellman key exchange:
 *      If |add| is not NULL, the prime will fulfill the condition p % |add| == |rem| (p % |add| == 1 if |rem| == NULL) in order to suit a given generator.
 *
 *      If |safe| is true, it will be a safe prime (i.e. a prime p so hat (p-1)/2 is also prime).
 *      If |safe| is true, and |rem| == NULL the condition will be p % |add| == 3.
 *      It is recommended that |add| is a multiple of 4.
 */
int mpi_generate_prime(mpi_t *ret, unsigned int bits, unsigned safe, const mpi_t *add, const mpi_t *rem, int (*rand_bytes)(void *, unsigned char *, unsigned int), void *rand_state);

#if defined(__cplusplus)
}
#endif

#endif
