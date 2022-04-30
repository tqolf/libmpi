#include "mpn-binary.h"

#ifndef DIVREM_1_NORM_THRESHOLD
#define DIVREM_1_NORM_THRESHOLD ~0
#endif

#ifndef DIVREM_1_UNNORM_THRESHOLD
#define DIVREM_1_UNNORM_THRESHOLD ~0
#endif

mpn_limb_t mpn_divrem_1(mpn_limb_t *qp, mpn_size_t qxn, const mpn_limb_t *up, mpn_size_t un, mpn_limb_t d)
{
    mpn_size_t n;
    mpn_size_t i;
    mpn_limb_t n1, n0;
    mpn_limb_t r = 0;

    MPN_ASSERT(qxn >= 0);
    MPN_ASSERT(un >= 0);
    MPN_ASSERT(d != 0);
    /* FIXME: What's the correct overlap rule when qxn!=0? */
    // MPN_ASSERT(MPN_SAME_OR_SEPARATE_P(qp + qxn, up, un));

    n = un + qxn;
    if (n == 0) return 0;

    d <<= MPN_NAIL_BITS;

    qp += (n - 1); /* Make qp point at most significant quotient limb */

    if ((d & MPN_LIMB_HIGHBIT) != 0) {
        if (un != 0) {
            /* High quotient limb is 0 or 1, skip a divide step. */
            mpn_limb_t q;
            r = up[un - 1] << MPN_NAIL_BITS;
            q = (r >= d);
            *qp-- = q;
            r -= (d & -q);
            r >>= MPN_NAIL_BITS;
            n--;
            un--;
        }

        if (MPN_BELOW_THRESHOLD(n, DIVREM_1_NORM_THRESHOLD)) {
        plain:
            for (i = un - 1; i >= 0; i--) {
                n0 = up[i] << MPN_NAIL_BITS;
                UDIV_NND(*qp, r, r, n0, d);
                r >>= MPN_NAIL_BITS;
                qp--;
            }
            for (i = qxn - 1; i >= 0; i--) {
                UDIV_NND(*qp, r, r, CNST_LIMB(0), d);
                r >>= MPN_NAIL_BITS;
                qp--;
            }
            return r;
        } else {
            /* Multiply-by-inverse, divisor already normalized. */
            mpn_limb_t dinv;
            INVERT_LIMB(dinv, d);

            for (i = un - 1; i >= 0; i--) {
                n0 = up[i] << MPN_NAIL_BITS;
                UDIV_NND_PREINV(*qp, r, r, n0, d, dinv);
                r >>= MPN_NAIL_BITS;
                qp--;
            }
            for (i = qxn - 1; i >= 0; i--) {
                UDIV_NND_PREINV(*qp, r, r, CNST_LIMB(0), d, dinv);
                r >>= MPN_NAIL_BITS;
                qp--;
            }
            return r;
        }
    } else {
        /* Most significant bit of divisor == 0.  */
        int cnt;

        /* Skip a division if high < divisor (high quotient 0).  Testing here
       before normalizing will still skip as often as possible.  */
        if (un != 0) {
            n1 = up[un - 1] << MPN_NAIL_BITS;
            if (n1 < d) {
                r = n1 >> MPN_NAIL_BITS;
                *qp-- = 0;
                n--;
                if (n == 0) return r;
                un--;
            }
        }

        if (!UDIV_NEEDS_NORMALIZATION && MPN_BELOW_THRESHOLD(n, DIVREM_1_UNNORM_THRESHOLD)) { goto plain; }

        COUNT_LEADING_ZEROS(cnt, d);
        d <<= cnt;
        r <<= cnt;

        if (UDIV_NEEDS_NORMALIZATION && MPN_BELOW_THRESHOLD(n, DIVREM_1_UNNORM_THRESHOLD)) {
            mpn_limb_t nshift;
            if (un != 0) {
                n1 = up[un - 1] << MPN_NAIL_BITS;
                r |= (n1 >> (MPN_LIMB_BITS - cnt));
                for (i = un - 2; i >= 0; i--) {
                    n0 = up[i] << MPN_NAIL_BITS;
                    nshift = (n1 << cnt) | (n0 >> (MPN_NUMB_BITS - cnt));
                    UDIV_NND(*qp, r, r, nshift, d);
                    r >>= MPN_NAIL_BITS;
                    qp--;
                    n1 = n0;
                }
                UDIV_NND(*qp, r, r, n1 << cnt, d);
                r >>= MPN_NAIL_BITS;
                qp--;
            }
            for (i = qxn - 1; i >= 0; i--) {
                UDIV_NND(*qp, r, r, CNST_LIMB(0), d);
                r >>= MPN_NAIL_BITS;
                qp--;
            }
            return r >> cnt;
        } else {
            mpn_limb_t dinv, nshift;
            INVERT_LIMB(dinv, d);
            if (un != 0) {
                n1 = up[un - 1] << MPN_NAIL_BITS;
                r |= (n1 >> (MPN_LIMB_BITS - cnt));
                for (i = un - 2; i >= 0; i--) {
                    n0 = up[i] << MPN_NAIL_BITS;
                    nshift = (n1 << cnt) | (n0 >> (MPN_NUMB_BITS - cnt));
                    UDIV_NND_PREINV(*qp, r, r, nshift, d, dinv);
                    r >>= MPN_NAIL_BITS;
                    qp--;
                    n1 = n0;
                }
                UDIV_NND_PREINV(*qp, r, r, n1 << cnt, d, dinv);
                r >>= MPN_NAIL_BITS;
                qp--;
            }
            for (i = qxn - 1; i >= 0; i--) {
                UDIV_NND_PREINV(*qp, r, r, CNST_LIMB(0), d, dinv);
                r >>= MPN_NAIL_BITS;
                qp--;
            }
            return r >> cnt;
        }
    }
}

mpn_limb_t mpn_divrem_2(mpn_limb_t *qp, mpn_size_t qxn, mpn_limb_t *np, mpn_size_t nn, const mpn_limb_t *dp)
{
    mpn_limb_t most_significant_q_limb;
    mpn_size_t i;
    mpn_limb_t r1, r0, d1, d0;
    mpn_limb_t di;

    MPN_ASSERT(nn >= 2);
    MPN_ASSERT(qxn >= 0);
    MPN_ASSERT(dp[1] & MPN_NUMB_HIGHBIT);
    MPN_ASSERT(!MPN_OVERLAP_P(qp, nn - 2 + qxn, np, nn) || qp >= np + 2);
    ASSERT_MPN(np, nn);
    ASSERT_MPN(dp, 2);

    np += nn - 2;
    d1 = dp[1];
    d0 = dp[0];
    r1 = np[1];
    r0 = np[0];

    most_significant_q_limb = 0;
    if (r1 >= d1 && (r1 > d1 || r0 >= d0)) {
#if MPN_NAIL_BITS == 0
        USUB_AABB(r1, r0, r1, r0, d1, d0);
#else
        r0 = r0 - d0;
        r1 = r1 - d1 - (r0 >> MPN_LIMB_BITS - 1);
        r0 &= MPN_NUMB_MASK;
#endif
        most_significant_q_limb = 1;
    }

    INVERT_PI1(di, d1, d0);

    qp += qxn;

    for (i = nn - 2 - 1; i >= 0; i--) {
        mpn_limb_t n0, q;
        n0 = np[-1];
        UDIV_NNNDD(q, r1, r0, r1, r0, n0, d1, d0, di);
        np--;
        qp[i] = q;
    }

    if (UNLIKELY(qxn != 0)) {
        qp -= qxn;
        for (i = qxn - 1; i >= 0; i--) {
            mpn_limb_t q;
            UDIV_NNNDD(q, r1, r0, r1, r0, CNST_LIMB(0), d1, d0, di);
            qp[i] = q;
        }
    }

    np[1] = r1;
    np[0] = r0;

    return most_significant_q_limb;
}

#if 0
void mpn_tdiv_qr(mpn_limb_t *qp, mpn_limb_t *rp, mpn_size_t qxn, const mpn_limb_t *np, mpn_size_t nn,
                 const mpn_limb_t *dp, mpn_size_t dn)
{
    ASSERT_ALWAYS(qxn == 0);

    ASSERT(nn >= 0);
    ASSERT(dn >= 0);
    ASSERT(dn == 0 || dp[dn - 1] != 0);
    ASSERT(!MPN_OVERLAP_P(qp, nn - dn + 1 + qxn, np, nn));
    ASSERT(!MPN_OVERLAP_P(qp, nn - dn + 1 + qxn, dp, dn));

    switch (dn) {
        case 0:
            MPI_RAISE_EXCEPTION();
            // DIVIDE_BY_ZERO;

        case 1: {
            rp[0] = mpn_divrem_1(qp, (mpn_size_t)0, np, nn, dp[0]);
            return;
        }

        case 2: {
            mpn_limb_t *n2p;
            mpn_limb_t qhl, cy;

            if ((dp[1] & MPN_NUMB_HIGHBIT) == 0) {
                int cnt;
                mpn_limb_t d2p[2];
                COUNT_LEADING_ZEROS(cnt, dp[1]);
                cnt -= MPN_NAIL_BITS;
                d2p[1] = (dp[1] << cnt) | (dp[0] >> (MPN_NUMB_BITS - cnt));
                d2p[0] = (dp[0] << cnt) & MPN_NUMB_MASK;
                n2p = malloc(MPN_LIMB_BYTES * (nn + 1));
                cy = mpn_lshift(n2p, np, nn, cnt);
                n2p[nn] = cy;
                qhl = mpn_divrem_2(qp, 0L, n2p, nn + (cy != 0), d2p);
                if (cy == 0) qp[nn - 2] = qhl; /* always store nn-2+1 quotient limbs */
                rp[0] = (n2p[0] >> cnt) | ((n2p[1] << (MPN_NUMB_BITS - cnt)) & MPN_NUMB_MASK);
                rp[1] = (n2p[1] >> cnt);
            } else {
                n2p = malloc(MPN_LIMB_BYTES * (nn));
                MPN_COPY(n2p, np, nn);
                qhl = mpn_divrem_2(qp, 0L, n2p, nn, dp);
                qp[nn - 2] = qhl; /* always store nn-2+1 quotient limbs */
                rp[0] = n2p[0];
                rp[1] = n2p[1];
            }
            free(n2p); // XXX
            return;
        }

        default: {
            int adjust;
            mpn_limb_t dinv;

            adjust = np[nn - 1] >= dp[dn - 1]; /* conservative tests for quotient size */
            if (nn + adjust >= 2 * dn) {
                mpn_limb_t *n2p, d2p;
                mpn_limb_t cy;
                int cnt;

                qp[nn - dn] = 0;                          /* zero high quotient limb */
                if ((dp[dn - 1] & MPN_NUMB_HIGHBIT) == 0) /* normalize divisor */
                {
                    COUNT_LEADING_ZEROS(cnt, dp[dn - 1]);
                    cnt -= MPN_NAIL_BITS;
                    d2p = malloc(MPN_LIMB_BYTES * dn);
                    mpn_lshift(d2p, dp, dn, cnt);
                    n2p = malloc(MPN_LIMB_BYTES * (nn + 1));
                    cy = mpn_lshift(n2p, np, nn, cnt);
                    n2p[nn] = cy;
                    nn += adjust;
                } else {
                    cnt = 0;
                    d2p = (mpn_limb_t *)dp;
                    n2p = malloc(MPN_LIMB_BYTES * (nn + 1));
                    MPN_COPY(n2p, np, nn);
                    n2p[nn] = 0;
                    nn += adjust;
                }

                INVERT_PI1(dinv, d2p[dn - 1], d2p[dn - 2]);
                if (MPN_BELOW_THRESHOLD(dn, DC_DIV_QR_THRESHOLD))
                    mpn_sbpi1_div_qr(qp, n2p, nn, d2p, dn, dinv.inv32);
                else if (BELOW_THRESHOLD(dn, MUPI_DIV_QR_THRESHOLD) ||                    /* fast condition */
                         BELOW_THRESHOLD(nn, 2 * MU_DIV_QR_THRESHOLD) ||                  /* fast condition */
                         (double)(2 * (MU_DIV_QR_THRESHOLD - MUPI_DIV_QR_THRESHOLD)) * dn /* slow... */
                                 + (double)MUPI_DIV_QR_THRESHOLD * nn
                             > (double)dn * nn) /* ...condition */
                    mpn_dcpi1_div_qr(qp, n2p, nn, d2p, dn, &dinv);
                else {
                    mpn_size_t itch = mpn_mu_div_qr_itch(nn, dn, 0);
                    mpn_limb_t *scratch = TMP_ALLOC_LIMBS(itch);
                    mpn_mu_div_qr(qp, rp, n2p, nn, d2p, dn, scratch);
                    n2p = rp;
                }

                if (cnt != 0) {
                    mpn_rshift(rp, n2p, dn, cnt);
                } else {
                    MPN_COPY(rp, n2p, dn);
                }
                // XXX: TMP_FREE;
                return;
            }

            /* When we come here, the numerator/partial remainder is less
               than twice the size of the denominator.  */

            {
                /* Problem:

                   Divide a numerator N with nn limbs by a denominator D with dn
                   limbs forming a quotient of qn=nn-dn+1 limbs.  When qn is small
                   compared to dn, conventional division algorithms perform poorly.
                   We want an algorithm that has an expected running time that is
                   dependent only on qn.

                   Algorithm (very informally stated):

                   1) Divide the 2 x qn most significant limbs from the numerator
                  by the qn most significant limbs from the denominator.  Call
                  the result qest.  This is either the correct quotient, but
                  might be 1 or 2 too large.  Compute the remainder from the
                  division.  (This step is implemented by an mpn_divrem call.)

                   2) Is the most significant limb from the remainder < p, where p
                  is the product of the most significant limb from the quotient
                  and the next(d)?  (Next(d) denotes the next ignored limb from
                  the denominator.)  If it is, decrement qest, and adjust the
                  remainder accordingly.

                   3) Is the remainder >= qest?  If it is, qest is the desired
                  quotient.  The algorithm terminates.

                   4) Subtract qest x next(d) from the remainder.  If there is
                  borrow out, decrement qest, and adjust the remainder
                  accordingly.

                   5) Skip one word from the denominator (i.e., let next(d) denote
                  the next less significant limb.  */

                mpn_size_t qn;
                mpn_limb_t *n2p, d2p;
                mpn_limb_t *tp;
                mpn_limb_t cy;
                mpn_size_t in, rn;
                mpn_limb_t quotient_too_large;
                unsigned int cnt;

                qn = nn - dn;
                qp[qn] = 0;   /* zero high quotient limb */
                qn += adjust; /* qn cannot become bigger */

                if (qn == 0) {
                    MPN_COPY(rp, np, dn);
                    // XXX: TMP_FREE;
                    return;
                }

                in = dn - qn; /* (at least partially) ignored # of limbs in ops */
                /* Normalize denominator by shifting it to the left such that its
                   most significant bit is set.  Then shift the numerator the same
                   amount, to mathematically preserve quotient.  */
                if ((dp[dn - 1] & MPN_NUMB_HIGHBIT) == 0) {
                    COUNT_LEADING_ZEROS(cnt, dp[dn - 1]);
                    cnt -= MPN_NAIL_BITS;

                    d2p = TMP_ALLOC_LIMBS(qn);
                    mpn_lshift(d2p, dp + in, qn, cnt);
                    d2p[0] |= dp[in - 1] >> (MPN_NUMB_BITS - cnt);

                    n2p = TMP_ALLOC_LIMBS(2 * qn + 1);
                    cy = mpn_lshift(n2p, np + nn - 2 * qn, 2 * qn, cnt);
                    if (adjust) {
                        n2p[2 * qn] = cy;
                        n2p++;
                    } else {
                        n2p[0] |= np[nn - 2 * qn - 1] >> (MPN_NUMB_BITS - cnt);
                    }
                } else {
                    cnt = 0;
                    d2p = (mpn_limb_t *)dp + in;

                    n2p = TMP_ALLOC_LIMBS(2 * qn + 1);
                    MPN_COPY(n2p, np + nn - 2 * qn, 2 * qn);
                    if (adjust) {
                        n2p[2 * qn] = 0;
                        n2p++;
                    }
                }

                /* Get an approximate quotient using the extracted operands.  */
                if (qn == 1) {
                    mpn_limb_t q0, r0;
                    udiv_qrnnd(q0, r0, n2p[1], n2p[0] << MPN_NAIL_BITS, d2p[0] << MPN_NAIL_BITS);
                    n2p[0] = r0 >> MPN_NAIL_BITS;
                    qp[0] = q0;
                } else if (qn == 2)
                    mpn_divrem_2(qp, 0L, n2p, 4L, d2p); /* FIXME: obsolete function */
                else {
                    invert_pi1(dinv, d2p[qn - 1], d2p[qn - 2]);
                    if (BELOW_THRESHOLD(qn, DC_DIV_QR_THRESHOLD))
                        mpn_sbpi1_div_qr(qp, n2p, 2 * qn, d2p, qn, dinv.inv32);
                    else if (BELOW_THRESHOLD(qn, MU_DIV_QR_THRESHOLD))
                        mpn_dcpi1_div_qr(qp, n2p, 2 * qn, d2p, qn, &dinv);
                    else {
                        mpn_size_t itch = mpn_mu_div_qr_itch(2 * qn, qn, 0);
                        mpn_limb_t *scratch = TMP_ALLOC_LIMBS(itch);
                        mpn_limb_t *r2p = rp;
                        if (np == r2p)      /* If N and R share space, put ... */
                            r2p += nn - qn; /* intermediate remainder at N's upper end. */
                        mpn_mu_div_qr(qp, r2p, n2p, 2 * qn, d2p, qn, scratch);
                        MPN_COPY(n2p, r2p, qn);
                    }
                }

                rn = qn;
                /* Multiply the first ignored divisor limb by the most significant
                   quotient limb.  If that product is > the partial remainder's
                   most significant limb, we know the quotient is too large.  This
                   test quickly catches most cases where the quotient is too large;
                   it catches all cases where the quotient is 2 too large.  */
                {
                    mpn_limb_t dl, x;
                    mpn_limb_t h, dummy;

                    if (in - 2 < 0)
                        dl = 0;
                    else
                        dl = dp[in - 2];

#if MPN_NAIL_BITS == 0
                    x = (dp[in - 1] << cnt) | ((dl >> 1) >> ((~cnt) % GMP_LIMB_BITS));
#else
                    x = (dp[in - 1] << cnt) & MPN_NUMB_MASK;
                    if (cnt != 0) x |= dl >> (MPN_NUMB_BITS - cnt);
#endif
                    umul_ppmm(h, dummy, x, qp[qn - 1] << MPN_NAIL_BITS);

                    if (n2p[qn - 1] < h) {
                        mpn_limb_t cy;

                        mpn_decr_u(qp, (mpn_limb_t)1);
                        cy = mpn_add_n(n2p, n2p, d2p, qn);
                        if (cy) {
                            /* The partial remainder is safely large.  */
                            n2p[qn] = cy;
                            ++rn;
                        }
                    }
                }

                quotient_too_large = 0;
                if (cnt != 0) {
                    mpn_limb_t cy1, cy2;

                    /* Append partially used numerator limb to partial remainder.  */
                    cy1 = mpn_lshift(n2p, n2p, rn, MPN_NUMB_BITS - cnt);
                    n2p[0] |= np[in - 1] & (MPN_NUMB_MASK >> cnt);

                    /* Update partial remainder with partially used divisor limb.  */
                    cy2 = mpn_submul_1(n2p, qp, qn, dp[in - 1] & (MPN_NUMB_MASK >> cnt));
                    if (qn != rn) {
                        ASSERT_ALWAYS(n2p[qn] >= cy2);
                        n2p[qn] -= cy2;
                    } else {
                        n2p[qn] = cy1 - cy2; /* & MPN_NUMB_MASK; */

                        quotient_too_large = (cy1 < cy2);
                        ++rn;
                    }
                    --in;
                }
                /* True: partial remainder now is neutral, i.e., it is not shifted up.  */

                tp = TMP_ALLOC_LIMBS(dn);

                if (in < qn) {
                    if (in == 0) {
                        MPN_COPY(rp, n2p, rn);
                        ASSERT_ALWAYS(rn == dn);
                        goto foo;
                    }
                    mpn_mul(tp, qp, qn, dp, in);
                } else
                    mpn_mul(tp, dp, in, qp, qn);

                cy = mpn_sub(n2p, n2p, rn, tp + in, qn);
                MPN_COPY(rp + in, n2p, dn - in);
                quotient_too_large |= cy;
                cy = mpn_sub_n(rp, np, tp, in);
                cy = mpn_sub_1(rp + in, rp + in, rn, cy);
                quotient_too_large |= cy;
            foo:
                if (quotient_too_large) {
                    mpn_decr_u(qp, (mpn_limb_t)1);
                    mpn_add_n(rp, rp, dp, dn);
                }
            }
            TMP_FREE;
            return;
        }
    }
}

mpn_limb_t mpn_divrem(mpn_limb_t *qp, mpn_size_t qxn, mpn_limb_t *np, mpn_size_t nn, const mpn_limb_t *dp,
                      mpn_size_t dn)
{
    MPN_ASSERT(qxn >= 0);
    MPN_ASSERT(nn >= dn);
    MPN_ASSERT(dn >= 1);
    MPN_ASSERT(dp[dn - 1] & MPN_NUMB_HIGHBIT);
    MPN_ASSERT(!MPN_OVERLAP_P(np, nn, dp, dn));
    MPN_ASSERT(!MPN_OVERLAP_P(qp, nn - dn + qxn, np, nn) || qp == np + dn + qxn);
    MPN_ASSERT(!MPN_OVERLAP_P(qp, nn - dn + qxn, dp, dn));
    ASSERT_MPN(np, nn);
    ASSERT_MPN(dp, dn);

    if (dn == 1) {
        mpn_limb_t ret;
        mpn_limb_t *q2p;
        mpn_size_t qn;

        // XXX
        q2p = malloc(MPN_LIMB_BYTES * (nn + qxn));

        np[0] = mpn_divrem_1(q2p, qxn, np, nn, dp[0]);
        qn = nn + qxn - 1;
        COPY(qp, q2p, qn);
        ret = q2p[qn];

        // XXX
        free(q2p);

        return ret;
    } else if (dn == 2) {
        return mpn_divrem_2(qp, qxn, np, nn, dp);
    } else {
        mpn_limb_t *q2p;
        mpn_limb_t qhl;
        mpn_size_t qn;

        if (UNLIKELY(qxn != 0)) {
            mpn_limb_t *n2p;
            n2p = malloc(MPN_LIMB_BYTES * (nn + qxn));
            q2p = malloc(MPN_LIMB_BYTES * (nn - dn + qxn + 1));

            ZEROIZE(n2p, 0, qxn);
            COPY(n2p + qxn, np, nn);
            mpn_tdiv_qr(q2p, np, 0L, n2p, nn + qxn, dp, dn);
            qn = nn - dn + qxn;
            COPY(qp, q2p, qn);
            qhl = q2p[qn];

            free(n2p);
            free(q2p);
        } else {
            q2p = malloc(MPN_LIMB_BYTES * (nn - dn + 1));
            mpn_tdiv_qr(q2p, np, 0L, np, nn, dp, dn);
            qn = nn - dn;
            COPY(qp, q2p, qn);
            qhl = q2p[qn];
            free(q2p);
        }

        return qhl;
    }
}
#endif

static inline void bn_div_rem_words(mpn_limb_t *quotient_out, mpn_limb_t *rem_out, mpn_limb_t n0, mpn_limb_t n1,
                                    mpn_limb_t d0)
{
    UDIV_NND(*quotient_out, *rem_out, n1, n0, d0);
}

int BN_div(BIGNUM *quotient, BIGNUM *rem, const BIGNUM *numerator, const BIGNUM *divisor, BN_CTX *ctx)
{
    int norm_shift, loop;
    BIGNUM wnum;
    mpn_limb_t *resp, *wnump;
    mpn_limb_t d0, d1;
    int num_n, div_n;

    // This function relies on the historical minimal-width |BIGNUM| invariant.
    // It is already not constant-time (constant-time reductions should use
    // Montgomery logic), so we shrink all inputs and intermediate values to
    // retain the previous behavior.

    // Invalid zero-padding would have particularly bad consequences.
    int numerator_width = bn_minimal_width(numerator);
    int divisor_width = bn_minimal_width(divisor);
    if ((numerator_width > 0 && numerator->d[numerator_width - 1] == 0)
        || (divisor_width > 0 && divisor->d[divisor_width - 1] == 0)) {
        OPENSSL_PUT_ERROR(BN, BN_R_NOT_INITIALIZED);
        return 0;
    }

    if (BN_is_zero(divisor)) {
        OPENSSL_PUT_ERROR(BN, BN_R_DIV_BY_ZERO);
        return 0;
    }

    BN_CTX_start(ctx);
    BIGNUM *tmp = BN_CTX_get(ctx);
    BIGNUM *snum = BN_CTX_get(ctx);
    BIGNUM *sdiv = BN_CTX_get(ctx);
    BIGNUM *res = NULL;
    if (quotient == NULL) {
        res = BN_CTX_get(ctx);
    } else {
        res = quotient;
    }
    if (sdiv == NULL || res == NULL) { goto err; }

    // First we normalise the numbers
    norm_shift = BN_BITS2 - (BN_num_bits(divisor) % BN_BITS2);
    if (!BN_lshift(sdiv, divisor, norm_shift)) { goto err; }
    bn_set_minimal_width(sdiv);
    sdiv->neg = 0;
    norm_shift += BN_BITS2;
    if (!BN_lshift(snum, numerator, norm_shift)) { goto err; }
    bn_set_minimal_width(snum);
    snum->neg = 0;

    // Since we don't want to have special-case logic for the case where snum is
    // larger than sdiv, we pad snum with enough zeroes without changing its
    // value.
    if (snum->width <= sdiv->width + 1) {
        if (!bn_wexpand(snum, sdiv->width + 2)) { goto err; }
        for (int i = snum->width; i < sdiv->width + 2; i++) { snum->d[i] = 0; }
        snum->width = sdiv->width + 2;
    } else {
        if (!bn_wexpand(snum, snum->width + 1)) { goto err; }
        snum->d[snum->width] = 0;
        snum->width++;
    }

    div_n = sdiv->width;
    num_n = snum->width;
    loop = num_n - div_n;
    // Lets setup a 'window' into snum
    // This is the part that corresponds to the current
    // 'area' being divided
    wnum.neg = 0;
    wnum.d = &(snum->d[loop]);
    wnum.width = div_n;
    // only needed when BN_ucmp messes up the values between width and max
    wnum.dmax = snum->dmax - loop; // so we don't step out of bounds

    // Get the top 2 words of sdiv
    // div_n=sdiv->width;
    d0 = sdiv->d[div_n - 1];
    d1 = (div_n == 1) ? 0 : sdiv->d[div_n - 2];

    // pointer to the 'top' of snum
    wnump = &(snum->d[num_n - 1]);

    // Setup |res|. |numerator| and |res| may alias, so we save |numerator->neg|
    // for later.
    const int numerator_neg = numerator->neg;
    res->neg = (numerator_neg ^ divisor->neg);
    if (!bn_wexpand(res, loop + 1)) { goto err; }
    res->width = loop - 1;
    resp = &(res->d[loop - 1]);

    // space for temp
    if (!bn_wexpand(tmp, div_n + 1)) { goto err; }

    // if res->width == 0 then clear the neg value otherwise decrease
    // the resp pointer
    if (res->width == 0) {
        res->neg = 0;
    } else {
        resp--;
    }

    for (int i = 0; i < loop - 1; i++, wnump--, resp--) {
        mpn_limb_t q, l0;
        // the first part of the loop uses the top two words of snum and sdiv to
        // calculate a mpn_limb_t q such that | wnum - sdiv * q | < sdiv
        mpn_limb_t n0, n1, rm = 0;

        n0 = wnump[0];
        n1 = wnump[-1];
        if (n0 == d0) {
            q = BN_MASK2;
        } else {
            // n0 < d0
            bn_div_rem_words(&q, &rm, n0, n1, d0);

#ifdef BN_ULLONG
            BN_ULLONG t2 = (BN_ULLONG)d1 * q;
            for (;;) {
                if (t2 <= ((((BN_ULLONG)rm) << BN_BITS2) | wnump[-2])) { break; }
                q--;
                rm += d0;
                if (rm < d0) {
                    break; // don't let rm overflow
                }
                t2 -= d1;
            }
#else  // !BN_ULLONG
            mpn_limb_t t2l, t2h;
            BN_UMULT_LOHI(t2l, t2h, d1, q);
            for (;;) {
                if (t2h < rm || (t2h == rm && t2l <= wnump[-2])) { break; }
                q--;
                rm += d0;
                if (rm < d0) {
                    break; // don't let rm overflow
                }
                if (t2l < d1) { t2h--; }
                t2l -= d1;
            }
#endif // !BN_ULLONG
        }

        l0 = bn_mul_words(tmp->d, sdiv->d, div_n, q);
        tmp->d[div_n] = l0;
        wnum.d--;
        // ingore top values of the bignums just sub the two
        // mpn_limb_t arrays with bn_sub_words
        if (bn_sub_words(wnum.d, wnum.d, tmp->d, div_n + 1)) {
            // Note: As we have considered only the leading
            // two BN_ULONGs in the calculation of q, sdiv * q
            // might be greater than wnum (but then (q-1) * sdiv
            // is less or equal than wnum)
            q--;
            if (bn_add_words(wnum.d, wnum.d, sdiv->d, div_n)) {
                // we can't have an overflow here (assuming
                // that q != 0, but if q == 0 then tmp is
                // zero anyway)
                (*wnump)++;
            }
        }
        // store part of the result
        *resp = q;
    }

    bn_set_minimal_width(snum);

    if (rem != NULL) {
        if (!BN_rshift(rem, snum, norm_shift)) { goto err; }
        if (!BN_is_zero(rem)) { rem->neg = numerator_neg; }
    }

    bn_set_minimal_width(res);
    BN_CTX_end(ctx);
    return 1;

err:
    BN_CTX_end(ctx);
    return 0;
}

int BN_nnmod(BIGNUM *r, const BIGNUM *m, const BIGNUM *d, BN_CTX *ctx)
{
    if (!(BN_mod(r, m, d, ctx))) { return 0; }
    if (!r->neg) { return 1; }

    // now -|d| < r < 0, so we have to set r := r + |d|.
    return (d->neg ? BN_sub : BN_add)(r, r, d);
}

mpn_limb_t bn_reduce_once(mpn_limb_t *r, const mpn_limb_t *a, mpn_limb_t carry, const mpn_limb_t *m, size_t num)
{
    assert(r != a);
    // |r| = |a| - |m|. |bn_sub_words| performs the bulk of the subtraction, and
    // then we apply the borrow to |carry|.
    carry -= bn_sub_words(r, a, m, num);
    // We know 0 <= |a| < 2*|m|, so -|m| <= |r| < |m|.
    //
    // If 0 <= |r| < |m|, |r| fits in |num| words and |carry| is zero. We then
    // wish to select |r| as the answer. Otherwise -m <= r < 0 and we wish to
    // return |r| + |m|, or |a|. |carry| must then be -1 or all ones. In both
    // cases, |carry| is a suitable input to |bn_select_words|.
    //
    // Although |carry| may be one if it was one on input and |bn_sub_words|
    // returns zero, this would give |r| > |m|, violating our input assumptions.
    assert(carry == 0 || carry == (mpn_limb_t)-1);
    bn_select_words(r, carry, a /* r < 0 */, r /* r >= 0 */, num);
    return carry;
}

mpn_limb_t bn_reduce_once_in_place(mpn_limb_t *r, mpn_limb_t carry, const mpn_limb_t *m, mpn_limb_t *tmp, size_t num)
{
    // See |bn_reduce_once| for why this logic works.
    carry -= bn_sub_words(tmp, r, m, num);
    assert(carry == 0 || carry == (mpn_limb_t)-1);
    bn_select_words(r, carry, r /* tmp < 0 */, tmp /* tmp >= 0 */, num);
    return carry;
}

void bn_mod_sub_words(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, const mpn_limb_t *m, mpn_limb_t *tmp,
                      size_t num)
{
    // r = a - b
    mpn_limb_t borrow = bn_sub_words(r, a, b, num);
    // tmp = a - b + m
    bn_add_words(tmp, r, m, num);
    bn_select_words(r, 0 - borrow, tmp /* r < 0 */, r /* r >= 0 */, num);
}

void bn_mod_add_words(mpn_limb_t *r, const mpn_limb_t *a, const mpn_limb_t *b, const mpn_limb_t *m, mpn_limb_t *tmp,
                      size_t num)
{
    mpn_limb_t carry = bn_add_words(r, a, b, num);
    bn_reduce_once_in_place(r, carry, m, tmp, num);
}

int bn_div_consttime(BIGNUM *quotient, BIGNUM *remainder, const BIGNUM *numerator, const BIGNUM *divisor,
                     unsigned divisor_min_bits, BN_CTX *ctx)
{
    if (BN_is_negative(numerator) || BN_is_negative(divisor)) {
        OPENSSL_PUT_ERROR(BN, BN_R_NEGATIVE_NUMBER);
        return 0;
    }
    if (BN_is_zero(divisor)) {
        OPENSSL_PUT_ERROR(BN, BN_R_DIV_BY_ZERO);
        return 0;
    }

    // This function implements long division in binary. It is not very efficient,
    // but it is simple, easy to make constant-time, and performant enough for RSA
    // key generation.

    int ret = 0;
    BN_CTX_start(ctx);
    BIGNUM *q = quotient, *r = remainder;
    if (quotient == NULL || quotient == numerator || quotient == divisor) { q = BN_CTX_get(ctx); }
    if (remainder == NULL || remainder == numerator || remainder == divisor) { r = BN_CTX_get(ctx); }
    BIGNUM *tmp = BN_CTX_get(ctx);
    if (q == NULL || r == NULL || tmp == NULL || !bn_wexpand(q, numerator->width) || !bn_wexpand(r, divisor->width)
        || !bn_wexpand(tmp, divisor->width)) {
        goto err;
    }

    OPENSSL_memset(q->d, 0, numerator->width * sizeof(mpn_limb_t));
    q->width = numerator->width;
    q->neg = 0;

    OPENSSL_memset(r->d, 0, divisor->width * sizeof(mpn_limb_t));
    r->width = divisor->width;
    r->neg = 0;

    // Incorporate |numerator| into |r|, one bit at a time, reducing after each
    // step. We maintain the invariant that |0 <= r < divisor| and
    // |q * divisor + r = n| where |n| is the portion of |numerator| incorporated
    // so far.
    //
    // First, we short-circuit the loop: if we know |divisor| has at least
    // |divisor_min_bits| bits, the top |divisor_min_bits - 1| can be incorporated
    // without reductions. This significantly speeds up |RSA_check_key|. For
    // simplicity, we round down to a whole number of words.
    assert(divisor_min_bits <= BN_num_bits(divisor));
    int initial_words = 0;
    if (divisor_min_bits > 0) {
        initial_words = (divisor_min_bits - 1) / BN_BITS2;
        if (initial_words > numerator->width) { initial_words = numerator->width; }
        OPENSSL_memcpy(r->d, numerator->d + numerator->width - initial_words, initial_words * sizeof(mpn_limb_t));
    }

    for (int i = numerator->width - initial_words - 1; i >= 0; i--) {
        for (int bit = BN_BITS2 - 1; bit >= 0; bit--) {
            // Incorporate the next bit of the numerator, by computing
            // r = 2*r or 2*r + 1. Note the result fits in one more word. We store the
            // extra word in |carry|.
            mpn_limb_t carry = bn_add_words(r->d, r->d, r->d, divisor->width);
            r->d[0] |= (numerator->d[i] >> bit) & 1;
            // |r| was previously fully-reduced, so we know:
            //      2*0 <= r <= 2*(divisor-1) + 1
            //        0 <= r <= 2*divisor - 1 < 2*divisor.
            // Thus |r| satisfies the preconditions for |bn_reduce_once_in_place|.
            mpn_limb_t subtracted = bn_reduce_once_in_place(r->d, carry, divisor->d, tmp->d, divisor->width);
            // The corresponding bit of the quotient is set iff we needed to subtract.
            q->d[i] |= (~subtracted & 1) << bit;
        }
    }

    if ((quotient != NULL && !BN_copy(quotient, q)) || (remainder != NULL && !BN_copy(remainder, r))) { goto err; }

    ret = 1;

err:
    BN_CTX_end(ctx);
    return ret;
}

static BIGNUM *bn_scratch_space_from_ctx(size_t width, BN_CTX *ctx)
{
    BIGNUM *ret = BN_CTX_get(ctx);
    if (ret == NULL || !bn_wexpand(ret, width)) { return NULL; }
    ret->neg = 0;
    ret->width = width;
    return ret;
}

// bn_resized_from_ctx returns |bn| with width at least |width| or NULL on
// error. This is so it may be used with low-level "words" functions. If
// necessary, it allocates a new |BIGNUM| with a lifetime of the current scope
// in |ctx|, so the caller does not need to explicitly free it. |bn| must fit in
// |width| words.
static const BIGNUM *bn_resized_from_ctx(const BIGNUM *bn, size_t width, BN_CTX *ctx)
{
    if ((size_t)bn->width >= width) {
        // Any excess words must be zero.
        assert(bn_fits_in_words(bn, width));
        return bn;
    }
    BIGNUM *ret = bn_scratch_space_from_ctx(width, ctx);
    if (ret == NULL || !BN_copy(ret, bn) || !bn_resize_words(ret, width)) { return NULL; }
    return ret;
}

int BN_mod_add(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, const BIGNUM *m, BN_CTX *ctx)
{
    if (!BN_add(r, a, b)) { return 0; }
    return BN_nnmod(r, r, m, ctx);
}

int BN_mod_add_quick(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, const BIGNUM *m)
{
    BN_CTX *ctx = BN_CTX_new();
    int ok = ctx != NULL && bn_mod_add_consttime(r, a, b, m, ctx);
    BN_CTX_free(ctx);
    return ok;
}

int bn_mod_add_consttime(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, const BIGNUM *m, BN_CTX *ctx)
{
    BN_CTX_start(ctx);
    a = bn_resized_from_ctx(a, m->width, ctx);
    b = bn_resized_from_ctx(b, m->width, ctx);
    BIGNUM *tmp = bn_scratch_space_from_ctx(m->width, ctx);
    int ok = a != NULL && b != NULL && tmp != NULL && bn_wexpand(r, m->width);
    if (ok) {
        bn_mod_add_words(r->d, a->d, b->d, m->d, tmp->d, m->width);
        r->width = m->width;
        r->neg = 0;
    }
    BN_CTX_end(ctx);
    return ok;
}

int BN_mod_sub(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, const BIGNUM *m, BN_CTX *ctx)
{
    if (!BN_sub(r, a, b)) { return 0; }
    return BN_nnmod(r, r, m, ctx);
}

int bn_mod_sub_consttime(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, const BIGNUM *m, BN_CTX *ctx)
{
    BN_CTX_start(ctx);
    a = bn_resized_from_ctx(a, m->width, ctx);
    b = bn_resized_from_ctx(b, m->width, ctx);
    BIGNUM *tmp = bn_scratch_space_from_ctx(m->width, ctx);
    int ok = a != NULL && b != NULL && tmp != NULL && bn_wexpand(r, m->width);
    if (ok) {
        bn_mod_sub_words(r->d, a->d, b->d, m->d, tmp->d, m->width);
        r->width = m->width;
        r->neg = 0;
    }
    BN_CTX_end(ctx);
    return ok;
}

int BN_mod_sub_quick(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, const BIGNUM *m)
{
    BN_CTX *ctx = BN_CTX_new();
    int ok = ctx != NULL && bn_mod_sub_consttime(r, a, b, m, ctx);
    BN_CTX_free(ctx);
    return ok;
}

int BN_mod_mul(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, const BIGNUM *m, BN_CTX *ctx)
{
    BIGNUM *t;
    int ret = 0;

    BN_CTX_start(ctx);
    t = BN_CTX_get(ctx);
    if (t == NULL) { goto err; }

    if (a == b) {
        if (!BN_sqr(t, a, ctx)) { goto err; }
    } else {
        if (!BN_mul(t, a, b, ctx)) { goto err; }
    }

    if (!BN_nnmod(r, t, m, ctx)) { goto err; }

    ret = 1;

err:
    BN_CTX_end(ctx);
    return ret;
}

int BN_mod_sqr(BIGNUM *r, const BIGNUM *a, const BIGNUM *m, BN_CTX *ctx)
{
    if (!BN_sqr(r, a, ctx)) { return 0; }

    // r->neg == 0,  thus we don't need BN_nnmod
    return BN_mod(r, r, m, ctx);
}

int BN_mod_lshift(BIGNUM *r, const BIGNUM *a, int n, const BIGNUM *m, BN_CTX *ctx)
{
    BIGNUM *abs_m = NULL;
    int ret;

    if (!BN_nnmod(r, a, m, ctx)) { return 0; }

    if (m->neg) {
        abs_m = BN_dup(m);
        if (abs_m == NULL) { return 0; }
        abs_m->neg = 0;
    }

    ret = bn_mod_lshift_consttime(r, r, n, (abs_m ? abs_m : m), ctx);

    BN_free(abs_m);
    return ret;
}

int bn_mod_lshift_consttime(BIGNUM *r, const BIGNUM *a, int n, const BIGNUM *m, BN_CTX *ctx)
{
    if (!BN_copy(r, a)) { return 0; }
    for (int i = 0; i < n; i++) {
        if (!bn_mod_lshift1_consttime(r, r, m, ctx)) { return 0; }
    }
    return 1;
}

int BN_mod_lshift_quick(BIGNUM *r, const BIGNUM *a, int n, const BIGNUM *m)
{
    BN_CTX *ctx = BN_CTX_new();
    int ok = ctx != NULL && bn_mod_lshift_consttime(r, a, n, m, ctx);
    BN_CTX_free(ctx);
    return ok;
}

int BN_mod_lshift1(BIGNUM *r, const BIGNUM *a, const BIGNUM *m, BN_CTX *ctx)
{
    if (!BN_lshift1(r, a)) { return 0; }

    return BN_nnmod(r, r, m, ctx);
}

int bn_mod_lshift1_consttime(BIGNUM *r, const BIGNUM *a, const BIGNUM *m, BN_CTX *ctx)
{
    return bn_mod_add_consttime(r, a, a, m, ctx);
}

int BN_mod_lshift1_quick(BIGNUM *r, const BIGNUM *a, const BIGNUM *m)
{
    BN_CTX *ctx = BN_CTX_new();
    int ok = ctx != NULL && bn_mod_lshift1_consttime(r, a, m, ctx);
    BN_CTX_free(ctx);
    return ok;
}

mpn_limb_t BN_div_word(BIGNUM *a, mpn_limb_t w)
{
    mpn_limb_t ret = 0;
    int i, j;

    if (!w) {
        // actually this an error (division by zero)
        return (mpn_limb_t)-1;
    }

    if (a->width == 0) { return 0; }

    // normalize input for |bn_div_rem_words|.
    j = BN_BITS2 - BN_num_bits_word(w);
    w <<= j;
    if (!BN_lshift(a, a, j)) { return (mpn_limb_t)-1; }

    for (i = a->width - 1; i >= 0; i--) {
        mpn_limb_t l = a->d[i];
        mpn_limb_t d;
        mpn_limb_t unused_rem;
        bn_div_rem_words(&d, &unused_rem, ret, l, w);
        ret = l - (d * w);
        a->d[i] = d;
    }

    bn_set_minimal_width(a);
    ret >>= j;
    return ret;
}

mpn_limb_t BN_mod_word(const BIGNUM *a, mpn_limb_t w)
{
#ifndef BN_CAN_DIVIDE_ULLONG
    mpn_limb_t ret = 0;
#else
    BN_ULLONG ret = 0;
#endif
    int i;

    if (w == 0) { return (mpn_limb_t)-1; }

#ifndef BN_CAN_DIVIDE_ULLONG
    // If |w| is too long and we don't have |BN_ULLONG| division then we need to
    // fall back to using |BN_div_word|.
    if (w > ((mpn_limb_t)1 << BN_BITS4)) {
        BIGNUM *tmp = BN_dup(a);
        if (tmp == NULL) { return (mpn_limb_t)-1; }
        ret = BN_div_word(tmp, w);
        BN_free(tmp);
        return ret;
    }
#endif

    for (i = a->width - 1; i >= 0; i--) {
#ifndef BN_CAN_DIVIDE_ULLONG
        ret = ((ret << BN_BITS4) | ((a->d[i] >> BN_BITS4) & BN_MASK2l)) % w;
        ret = ((ret << BN_BITS4) | (a->d[i] & BN_MASK2l)) % w;
#else
        ret = (BN_ULLONG)(((ret << (BN_ULLONG)BN_BITS2) | a->d[i]) % (BN_ULLONG)w);
#endif
    }
    return (mpn_limb_t)ret;
}

int BN_mod_pow2(BIGNUM *r, const BIGNUM *a, size_t e)
{
    if (e == 0 || a->width == 0) {
        BN_zero(r);
        return 1;
    }

    size_t num_words = 1 + ((e - 1) / BN_BITS2);

    // If |a| definitely has less than |e| bits, just BN_copy.
    if ((size_t)a->width < num_words) { return BN_copy(r, a) != NULL; }

    // Otherwise, first make sure we have enough space in |r|.
    // Note that this will fail if num_words > INT_MAX.
    if (!bn_wexpand(r, num_words)) { return 0; }

    // Copy the content of |a| into |r|.
    OPENSSL_memcpy(r->d, a->d, num_words * sizeof(mpn_limb_t));

    // If |e| isn't word-aligned, we have to mask off some of our bits.
    size_t top_word_exponent = e % (sizeof(mpn_limb_t) * 8);
    if (top_word_exponent != 0) { r->d[num_words - 1] &= (((mpn_limb_t)1) << top_word_exponent) - 1; }

    // Fill in the remaining fields of |r|.
    r->neg = a->neg;
    r->width = (int)num_words;
    bn_set_minimal_width(r);
    return 1;
}

int BN_nnmod_pow2(BIGNUM *r, const BIGNUM *a, size_t e)
{
    if (!BN_mod_pow2(r, a, e)) { return 0; }

    // If the returned value was non-negative, we're done.
    if (BN_is_zero(r) || !r->neg) { return 1; }

    size_t num_words = 1 + (e - 1) / BN_BITS2;

    // Expand |r| to the size of our modulus.
    if (!bn_wexpand(r, num_words)) { return 0; }

    // Clear the upper words of |r|.
    OPENSSL_memset(&r->d[r->width], 0, (num_words - r->width) * BN_BYTES);

    // Set parameters of |r|.
    r->neg = 0;
    r->width = (int)num_words;

    // Now, invert every word. The idea here is that we want to compute 2^e-|x|,
    // which is actually equivalent to the twos-complement representation of |x|
    // in |e| bits, which is -x = ~x + 1.
    for (int i = 0; i < r->width; i++) { r->d[i] = ~r->d[i]; }

    // If our exponent doesn't span the top word, we have to mask the rest.
    size_t top_word_exponent = e % BN_BITS2;
    if (top_word_exponent != 0) { r->d[r->width - 1] &= (((mpn_limb_t)1) << top_word_exponent) - 1; }

    // Keep the minimal-width invariant for |BIGNUM|.
    bn_set_minimal_width(r);

    // Finally, add one, for the reason described above.
    return BN_add(r, r, BN_value_one());
}


// i from xsize - 1 down to ysize:
//   x <- x - q[i - t - 1]y * b^(i - t - 1)
// x = 76397902588E18083990FC2C02794FE36CD5A4D3A0DBE4E21A069472EEE5EBB76330D8BB8C12DCEFD6BE1CD7EECED69C00179DF038782ACDB95396F3549A6A1ADF438DBFBC975DBAB9868049CB8B9EF80FA8DF264D885D775F00000000000000
// y = CB3E64137DC7A52540D2791C1B0C24B1A0BAC9825CB4E1F62C2EF410476AF1875BAE201364A95DF010FEB4CCBBE00B3C0A49F587ED8685A033BF13BABADF6D6EA91C6053D0F43904FFE2CCF6BD5A9A389FAAB37F08CBE25BA500000000000000
// q = 9542D4891D37C
