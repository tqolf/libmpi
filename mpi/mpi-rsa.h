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
#ifndef MULTIPLE_PRECISION_INTEGER_RSA_H
#define MULTIPLE_PRECISION_INTEGER_RSA_H

#include <mpi/mpi.h>
#include <mpi/mpi-montgomery.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    unsigned int nbits; /* RSA modulus bitsize */
    unsigned int ebits; /* RSA public exp bitsize */
    unsigned int dbits; /* RSA private exp bitsize */
    unsigned int pbits; /* RSA p-factor bitsize */
    unsigned int qbits; /* RSA q-factor bitsize */

    mpi_limb_t *e;    /* public exponent, bitsize(e) = ebits */
    mpi_limb_t *d;    /* private exponent, bitsize(d) = dbits <= nbits */
    mpi_limb_t *dp;   /* the first factor's CRT exponent, d mod (p - 1), bitsize(dp) <= pbits */
    mpi_limb_t *dq;   /* the second factor's CRT exponent, d mod (q - 1), bitsize(dq) <= qbits */
    mpi_limb_t *qinv; /* the (first) CRT coefficient, q^(-1) mode p, bitsize(qinv) <= pbits */

    mpi_montgomery_t *montN; /* montgomery context for (N, the modulus, bitsize(n) = nbits) */
    mpi_montgomery_t *montP; /* montgomery context for (P, the first factor) */
    mpi_montgomery_t *montQ; /* montgomery context for (Q, the second factor) */

    /* TODO: multiple-primes support */
    unsigned int primes;
    struct rsa_factor {
        unsigned int bits; /* bit-size of factor */
        mpi_limb_t *r;     /* factor */
        mpi_limb_t *d;     /* factor's CRT exponent */
        mpi_limb_t *t;     /* factor's CRT coefficient */
    } factors[0];
} rsa_key_t;

rsa_key_t *rsa_new(unsigned int ebits, unsigned int nbits, unsigned int primes);
void rsa_free(rsa_key_t *key);

int rsa_import(rsa_key_t *key, const mpi_t *n, const mpi_t *e, const mpi_t *d, const mpi_t *dp,
               const mpi_t *dq, const mpi_t *qinv);
rsa_key_t *rsa_generate_key(const mpi_t *pubexp, unsigned int nbits, unsigned int primes,
                            int (*rand_bytes)(void *, unsigned char *, unsigned int), void *rand_state);

int rsa_pub_cipher(mpi_t *r, const mpi_t *x, const rsa_key_t *key);
int rsa_prv_cipher(mpi_t *r, const mpi_t *x, const rsa_key_t *key);
int rsa_prv_cipher_crt(mpi_t *r, const mpi_t *x, const rsa_key_t *key);

#if defined(__cplusplus)
}
#endif

#endif
