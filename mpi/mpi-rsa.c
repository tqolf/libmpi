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
#include "mpi-rsa.h"
#include <mpn/mpn-binary.h>
#include <mpn/mpn-montgomery.h>

/**
 * Number of Miller-Rabin rounds for an error rate of less than 1/2^80 for random 'b'-bit input, b >= 100.
 * (see Table 4.4, Handbook of Applied Cryptography [Menezes, van Oorschot, Vanstone; CRC Press 1996]
 */
#define MR_rounds_p80(b) \
    ((b) >= 1300  ? 2    \
     : (b) >= 850 ? 3    \
     : (b) >= 650 ? 4    \
     : (b) >= 550 ? 5    \
     : (b) >= 450 ? 6    \
     : (b) >= 400 ? 7    \
     : (b) >= 350 ? 8    \
     : (b) >= 300 ? 9    \
     : (b) >= 250 ? 12   \
     : (b) >= 200 ? 15   \
     : (b) >= 150 ? 18   \
                  : /*(b) >=  100*/ 27)
#define RSA_POOL_SIZE (2)

rsa_key_t *rsa_new(unsigned int ebits, unsigned int nbits, unsigned int primes)
{
    if (ebits == 0 || nbits == 0) {
        MPI_RAISE_ERROR(-EINVAL);
        return NULL;
    }
    MPN_ASSERT(primes == 2); // only two primes(p, q) supported now

    unsigned int quo = nbits / primes;
    unsigned int rem = nbits % primes;

    unsigned int pbits = quo;
    unsigned int qbits = quo + rem;

    unsigned int esize = MPN_BITS_TO_LIMBS(ebits);
    unsigned int nsize = MPN_BITS_TO_LIMBS(nbits);
    unsigned int psize = MPN_BITS_TO_LIMBS(pbits);
    unsigned int qsize = MPN_BITS_TO_LIMBS(qbits);

    // @note: assume that addition won't overflow
    size_t size = sizeof(rsa_key_t) + MPN_LIMB_BYTES;                     /* rsa_key_t, and alignment */
    size += sizeof(mpn_limb_t) * (esize + nsize + psize + qsize + psize); /* e, d, dp, dq, qinv */
    rsa_key_t *key = (rsa_key_t *)MPI_ALLOCATE(size);
    if (key == NULL) {
        MPI_RAISE_ERROR(-ENOMEM);
        return NULL;
    }
    key->primes = primes;

    // initialize rsa-key
    {
        key->nbits = nbits;
        key->pbits = pbits;
        key->qbits = qbits;
        key->ebits = ebits;
        key->dbits = nbits; /* max bits here, calcaulte later if needed */

        {
            mpn_limb_t *p = mpi_aligned_pointer((unsigned char *)key + sizeof(rsa_key_t), MPN_LIMB_BYTES);
            key->e = p;               /* esize */
            key->d = (p += esize);    /* <= nsize */
            key->dp = (p += nsize);   /* <= psize */
            key->dq = (p += psize);   /* <= qsize */
            key->qinv = (p += qsize); /* <= psize */
        }

        {
            key->montP = mpn_montgomery_create(pbits, MPN_BITS_TO_LIMBS(pbits) * 6);
            key->montQ = mpn_montgomery_create(qbits, MPN_BITS_TO_LIMBS(qbits) * 6);
            key->montN = mpn_montgomery_create(nbits, MPN_BITS_TO_LIMBS(nbits) * 6);
        }
    }

    return key;
}

void rsa_free(rsa_key_t *key)
{
    if (key != NULL) {
        mpn_montgomery_destory(key->montN);
        mpn_montgomery_destory(key->montP);
        mpn_montgomery_destory(key->montQ);
        MPI_DEALLOCATE(key);
    }
}

int rsa_import(rsa_key_t *key, const mpi_t *n, const mpi_t *e, const mpi_t *d, const mpi_t *dp, const mpi_t *dq,
               const mpi_t *qinv)
{
    if (key == NULL || (n == NULL && d == NULL) || (n == NULL || dp == NULL || dq == NULL || qinv == NULL)) {
        MPI_RAISE_ERROR(-EINVAL);
        return -EINVAL;
    }

    if (n != NULL) {
        if (key->nbits < mpi_bits(n)) {
            MPI_RAISE_ERROR(-EINVAL, "Mismatched integers");
            return -EINVAL;
        }
        key->nbits = mpi_bits(n); // XXX: room left will be shadowed
        mpn_montgomery_set_modulus_bin(key->montN, n->data, key->nbits);
    }

    if (e != NULL) {
        if (key->ebits < mpi_bits(e)) {
            MPI_RAISE_ERROR(-EINVAL, "Mismatched integers");
            return -EINVAL;
        }
        key->ebits = mpi_bits(e); // XXX: room left will be shadowed
        COPY(key->e, e->data, e->size);
    }

    if (d != NULL) {
        if (key->dbits < mpi_bits(d)) {
            MPI_RAISE_ERROR(-EINVAL, "Mismatched integers");
            return -EINVAL;
        }
        key->dbits = mpi_bits(d); // XXX: room left will be shadowed
        COPY(key->d, d->data, d->size);

        if (dp == NULL || dq == NULL || qinv == NULL) {
            mpn_montgomery_destory(key->montP);
            key->montP = NULL;
            mpn_montgomery_destory(key->montQ);
            key->montQ = NULL;
        }
    }

    if (dp != NULL && dq != NULL && qinv != NULL) {
        COPY(key->dp, dp->data, dp->size);
        COPY(key->dq, dq->data, dq->size);
        COPY(key->qinv, qinv->data, qinv->size);
    } else if (d != NULL) {
        /* do nothing */
    } else {
        MPI_RAISE_WARN("Private key material not be imported this time");
    }

    return 0;
}

rsa_key_t *rsa_generate_key(const mpi_t *pubexp, unsigned int nbits, unsigned int primes,
                            int (*rand_bytes)(void *, unsigned char *, unsigned int), void *rand_state)
{
    rsa_key_t *key = rsa_new(mpi_bits(pubexp), nbits, primes);
    if (key == NULL) {
        MPI_RAISE_ERROR(-ENOMEM);
        return NULL;
    }

    unsigned int pbits = key->pbits;
    unsigned int qbits = key->qbits;
    unsigned int psize = MPN_BITS_TO_LIMBS(pbits);
    unsigned int qsize = MPN_BITS_TO_LIMBS(qbits);

    /* copy public exponent */
    {
        COPY(key->e, pubexp->data, pubexp->size);
    }

    {
        int err = 0;

        /* gennerate prime: p */
        {
            mpi_t p;
            mpi_make(&p, key->montP->modulus, psize);
            err = mpi_generate_prime(&p, pbits, 0, NULL, NULL, rand_bytes, rand_state);
            if (err != 0) {
                MPI_RAISE_ERROR(err);
                goto exit_with_error;
            }

            mpn_montgomery_set_modulus_bin(key->montP, p.data, pbits);
        }

        /* gennerate prime: q */
        {
            mpi_t q;
            mpi_make(&q, key->montQ->modulus, qsize);
            err = mpi_generate_prime(&q, qbits, 0, NULL, NULL, rand_bytes, rand_state);
            if (err != 0) {
                MPI_RAISE_ERROR(err);
                goto exit_with_error;
            }

            mpn_montgomery_set_modulus_bin(key->montQ, q.data, qbits);
        }

        {
            unsigned int nsize = MPN_BITS_TO_LIMBS(nbits);
            unsigned int buffsize = (nsize + 1) * 3 + psize;
            mpn_limb_t *buffer = (mpn_limb_t *)MPI_ZALLOCATE(buffsize, sizeof(mpn_limb_t));
            if (buffer == NULL) {
                MPI_RAISE_ERROR(-ENOMEM);
                goto exit_with_error;
            }

            mpn_montgomery_t *montN = key->montN;
            mpn_montgomery_t *montP = key->montP;
            mpn_montgomery_t *montQ = key->montQ;
            mpn_limb_t *dataN = montN->modulus;
            mpn_limb_t *dataP = montP->modulus;
            mpn_limb_t *dataQ = montQ->modulus;

            mpn_limb_t *exponentD = buffer;                    /* nsize + 1 */
            mpn_limb_t *exponentDBuff = exponentD + nsize + 1; /* nsize + 1 */
            mpn_limb_t *phi = exponentDBuff + nsize + 1;       /* nsize + 1 */
            mpn_limb_t *phiBuff = phi + nsize + 1;             /* psize */

            /* phi = (P - 1) * (Q - 1) */
            mpn_dec_vectorized(dataP, dataP, psize, 1);
            mpn_dec_vectorized(dataQ, dataQ, qsize, 1);
            mpn_mul(phi, dataP, psize, dataQ, qsize);

            /* D = 1 / E mod phi */
            unsigned int dsize = mpn_mod_invert(exponentD, pubexp->data, pubexp->size, phi, nsize, montN->optimizer);
            if (key->d != NULL) {
                COPY(key->d, exponentD, dsize);
                key->dbits = mpn_bits_consttime(exponentD, dsize);
            }

            /* compute dp = d mod (p - 1) */
            {
                COPY(exponentDBuff, exponentD, dsize);
                unsigned int size = mpn_mod(exponentDBuff, dsize, dataP, psize);
                ZEXPAND(key->dp, psize, exponentDBuff, size);
            }

            /* compute dq = d mod (q - 1) */
            {
                COPY(phi, exponentD, dsize);
                unsigned int size = mpn_mod(phi, dsize, dataQ, qsize);
                ZEXPAND(key->dq, qsize, phi, size);
            }

            /* restore P and Q */
            mpn_inc_vectorized(dataP, dataP, psize, 1);
            mpn_inc_vectorized(dataQ, dataQ, qsize, 1);

            /* re-initialize montgomery context */
            mpn_montgomery_set_modulus_bin(montP, dataP, pbits);
            mpn_montgomery_set_modulus_bin(montQ, dataQ, qbits);

            /* qinv = 1 / q mod p */
            {
                COPY(phiBuff, dataP, psize);
                unsigned int size = mpn_mod_invert(key->qinv, dataQ, qsize, phiBuff, psize, montP->optimizer);

                ZEROIZE(key->qinv, size, psize);
            }

            {
                mpn_mul(dataN, dataP, psize, dataQ, qsize);
                mpn_montgomery_set_modulus_bin(montN, dataN, nbits);

                key->nbits = nsize * MPN_LIMB_BITS - mpn_limb_ntz_consttime(dataN[nsize - 1]);
            }

            MPI_DEALLOCATE(buffer);
        }

        return key;
    }

exit_with_error:
    if (key != NULL) { rsa_free(key); }

    return NULL;
}

int rsa_pub_cipher(mpi_t *r, const mpi_t *x, const rsa_key_t *key)
{
    if (r == NULL || x == NULL || key == NULL || key->montN == NULL) { return -EINVAL; }

    r->sign = MPI_SIGN_NON_NEGTIVE;
    r->size = mpn_montgomery_exp(r->data, x->data, x->size, key->e, key->ebits, key->montN);

    return 0;
}

int rsa_prv_cipher(mpi_t *r, const mpi_t *x, const rsa_key_t *key)
{
    if (r == NULL || x == NULL || key == NULL || key->montN == NULL) { return -EINVAL; }

    r->sign = MPI_SIGN_NON_NEGTIVE;
    r->size = mpn_montgomery_exp_consttime(r->data, x->data, x->size, key->d, key->dbits, key->montN);

    return 0;
}

int rsa_prv_cipher_crt(mpi_t *r, const mpi_t *x, const rsa_key_t *key)
{
    if (r == NULL || x == NULL || key == NULL || key->montN == NULL || key->montP == NULL || key->montQ == NULL) {
        return -EINVAL;
    }

    mpn_limb_t *scratch = (mpn_limb_t *)MPI_ZALLOCATE(1000, sizeof(mpn_limb_t));
    mpn_limb_t *buffer = &scratch[x->size];

    mpn_limb_t *dataY = r->data;
    mpn_limb_t *dataXp = x->data;
    mpn_limb_t *dataXq = scratch;

    /* P- and Q- montgometry engines */
    mpn_montgomery_t *montP = key->montP;
    mpn_montgomery_t *montQ = key->montQ;
    unsigned int nsP = montP->modsize;
    unsigned int nsQ = montQ->modsize;
    unsigned int bitSizeP = key->pbits;
    unsigned int bitSizeQ = key->qbits;
    unsigned int bitSizeDP = bitSizeP;
    unsigned int bitSizeDQ = bitSizeQ;

    /* compute xq = x^dQ mod Q */
    if (bitSizeP == bitSizeQ) { /* believe it's enough conditions for correct Mont application */
        ZEXPAND(buffer, nsQ + nsQ, x->data, x->size);
        mpn_montgomery_reduce(dataXq, buffer, montQ);
        mpn_montgomery_mul(dataXq, dataXq, montQ->montRR, montQ);
    } else {
        COPY(dataXq, x->data, x->size);
        mpn_div(NULL, NULL, dataXq, x->size, montQ->modulus, nsQ);
    }

    mpn_montgomery_exp_consttime(dataXq, dataXq, nsQ, key->dq, bitSizeDQ, montQ);

    /* compute xp = x^dP mod P */
    if (bitSizeP == bitSizeQ) { /* believe it's enough conditions for correct Mont application */
        ZEXPAND(buffer, nsP + nsP, x->data, x->size);
        mpn_montgomery_reduce(dataXp, buffer, montP);
        mpn_montgomery_mul(dataXp, dataXp, montP->montRR, montP);
    } else {
        COPY(dataXp, x->data, x->size);
        mpn_div(NULL, NULL, dataXp, x->size, montP->modulus, nsP);
    }

    mpn_montgomery_exp_consttime(dataXp, dataXp, nsP, key->dp, bitSizeDP, montP);

    /**
     * recombination
     *  xq = xq mod P
     *  must be sure that xq in the same residue domain as xp
     *  because of following (xp-xq) mod P operation
     */
    if (bitSizeP == bitSizeQ) { /* believe it's enough conditions for correct Mont application */
        ZEXPAND(buffer, nsP + nsP, dataXq, nsQ);
        // mpn_montgomery_reduce(buffer, buffer, montP);
        // mpn_montgomery_mul(buffer, buffer, montP->montRR, montP);
        mpn_montgomery_sub(buffer, buffer, montP->modulus, montP);
        /* xp = (xp - xq) mod P */
        mpn_montgomery_sub(dataXp, dataXp, buffer, montP);
    } else {
        COPY(buffer, dataXq, nsQ);
        {
            unsigned int nsQP = mpn_div(NULL, NULL, buffer, nsQ, montP->modulus, nsP);
            mpn_limb_t cf = mpn_sub_vectorized(dataXp, dataXp, buffer, nsQP);
            if (nsP - nsQP) cf = mpn_dec_vectorized(dataXp + nsQP, dataXp + nsQP, (nsP - nsQP), cf);
            if (cf) { mpn_add_vectorized(dataXp, dataXp, montP->modulus, nsP); }
        }
    }

    /* xp = xp*qInv mod P */
    /* convert invQ into Montgomery domain */
    mpn_montgomery_encode(buffer, key->qinv, montP);
    /* and multiply xp *= mont(invQ) mod P */
    mpn_montgomery_mul(dataXp, dataXp, buffer, montP);

    /* Y = xq + xp*Q */
    mpn_mul(buffer, dataXp, nsP, montQ->modulus, nsQ);
    {
        mpn_limb_t cf = mpn_add_vectorized(dataY, buffer, dataXq, nsQ);
        mpn_inc_vectorized(dataY + nsQ, buffer + nsQ, nsP, cf);
    }

    r->sign = MPI_SIGN_NON_NEGTIVE;
    r->size = mpn_limbs(r->data, nsP + nsQ);

    MPI_DEALLOCATE(scratch);

    return 0;
}
