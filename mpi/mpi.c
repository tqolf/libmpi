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
#include "mpi.h"
#include "mpi-binary.h"

/**
 * @addtogroup: mpi/construction-destruction
 */
/**
 * create mpi with expected bits |bits| to reserve
 *
 * |bits| == 0, to create empty room
 */
mpi_t *mpi_create(unsigned int bits)
{
    if (bits > MPI_MAX_BITS) {
        MPI_RAISE_ERROR(-ERANGE);
        return NULL;
    }

    unsigned int room = MPI_BITS_TO_LIMBS(bits);
    mpi_t *r = (mpi_t *)MPI_ALLOCATE(sizeof(mpi_t) + room * MPI_LIMB_BYTES + MPI_LIMB_BYTES);
    if (r != NULL) {
        r->attr = 0;
        r->size = 0;
        r->room = room;
        r->sign = MPI_SIGN_NON_NEGTIVE;
        r->data = mpi_aligned_pointer((unsigned char *)r + sizeof(mpi_t), MPI_LIMB_BYTES);
    }

    return r;
}

/**
 * create mpi(detached) with expected bits |bits| to reserve
 *
 * |bits| == 0, to create empty room
 */
mpi_t *mpi_create_detached(unsigned int bits)
{
    if (bits > MPI_MAX_BITS) {
        MPI_RAISE_ERROR(-ERANGE);
        return NULL;
    }

    mpi_t *r = (mpi_t *)MPI_ALLOCATE(sizeof(mpi_t));
    if (r != NULL) {
        r->size = 0;
        r->sign = MPI_SIGN_NON_NEGTIVE;
        r->attr = MPI_ATTR_DETACHED;
        unsigned int room = MPI_BITS_TO_LIMBS(bits);
        r->data = (mpi_limb_t *)MPI_ZALLOCATE(room, sizeof(mpi_limb_t));
        if (r->data == NULL) {
            MPI_DEALLOCATE(r);
            return NULL;
        }
        r->room = room;
    }

    return r;
}

/**
 * make mpi with given chunk(usually static or pre-allocated memory)
 */
void mpi_make(mpi_t *r, mpi_limb_t *data, unsigned int size)
{
    if (r != NULL) {
        r->room = size;
        r->data = data;
        r->sign = MPI_SIGN_NON_NEGTIVE;
        r->attr = MPI_ATTR_NOTOWNED | MPI_ATTR_DETACHED;
        r->size = mpi_fix_size_bin(data, size);
    }
}

/**
 * clear and release mpi |r|
 */
void mpi_destory(mpi_t *r)
{
    if (r == NULL) {
        MPI_RAISE_WARN("Invalid Integer: nullptr");
        return;
    }

    r->sign = MPI_SIGN_NON_NEGTIVE;
    ZEROIZE(r->data, 0, r->size);
    r->size = r->room = 0;
    if ((r->attr & MPI_ATTR_DETACHED) && !(r->attr & MPI_ATTR_NOTOWNED)) { MPI_DEALLOCATE(r->data); }
    MPI_DEALLOCATE(r);
}

/**
 * mpi: expand mpi to expected bits |bits|
 *
 * @note:
 *   1. maybe fail when no enough memory or invalid size given
 */
mpi_t *mpi_expand(mpi_t *a, unsigned int bits)
{
    if (a == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: nullptr");
        return NULL;
    }
    if (bits > MPI_MAX_BITS) {
        MPI_RAISE_ERROR(-ERANGE, "Required bits of mpi too large");
        return NULL;
    }

    unsigned int room = MPI_BITS_TO_LIMBS(bits);
    if (room <= a->room) {
        return a;
    } else {
#ifdef MPI_REALLOCATE
        mpi_t *r = (mpi_t *)MPI_REALLOCATE(a, sizeof(mpi_t) + room * MPI_LIMB_BYTES + MPI_LIMB_BYTES);
        if (r != NULL) {
            if (r != a) {
                unsigned int aoff = mpi_aligned_diff((unsigned char *)a + sizeof(mpi_t), MPI_LIMB_BYTES);
                unsigned int roff = mpi_aligned_diff((unsigned char *)r + sizeof(mpi_t), MPI_LIMB_BYTES);
                if (aoff != roff) { memmove(r + sizeof(mpi_t) + roff, r + sizeof(mpi_t) + aoff, r->size); }
            }
            r->room = room;
            r->data = mpi_aligned_pointer((unsigned char *)r + sizeof(mpi_t), MPI_LIMB_BYTES);
        }

        return r;
#else
        mpi_t *r = mpi_create(bits);
        if (r != NULL) {
            r->sign = a->sign;
            r->size = a->size;
            ZEXPAND(r->data, r->room, a->data, a->size);
            mpi_destory(a);
        }

        return r;
#endif
    }
}

/**
 * resize mpi to expected bits |bits|
 *
 * @note:
 *   1. maybe fail when no enough memory or invalid size given
 *
 */
mpi_t *mpi_resize(mpi_t *a, unsigned int bits)
{
    if (a == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: nullptr");
        return NULL;
    }
    if (bits > MPI_MAX_BITS) {
        MPI_RAISE_ERROR(-ERANGE, "Required bits of mpi too large");
        return NULL;
    }

    unsigned int room = MPI_BITS_TO_LIMBS(bits);
    if (room == a->room) {
        return a;
    } else if (room >= a->size) { // expand or shrink to proper size
#ifdef MPI_REALLOCATE
        mpi_t *r = (mpi_t *)MPI_REALLOCATE(a, sizeof(mpi_t) + room * MPI_LIMB_BYTES + MPI_LIMB_BYTES);
        if (r != NULL) {
            if (r != a) {
                unsigned int aoff = mpi_aligned_diff((unsigned char *)a + sizeof(mpi_t), MPI_LIMB_BYTES);
                unsigned int roff = mpi_aligned_diff((unsigned char *)r + sizeof(mpi_t), MPI_LIMB_BYTES);
                if (aoff != roff) { memmove(r + sizeof(mpi_t) + roff, r + sizeof(mpi_t) + aoff, r->size); }
            }
            r->room = room;
            r->data = mpi_aligned_pointer((unsigned char *)r + sizeof(mpi_t), MPI_LIMB_BYTES);
        }

        return r;
#else
        mpi_t *r = mpi_create(bits);
        if (r != NULL) {
            r->sign = a->sign;
            r->size = a->size;
            ZEXPAND(r->data, r->room, a->data, a->size);
            mpi_destory(a);
        }

        return r;
#endif
    } else {
        MPI_RAISE_ERROR(-ERANGE, "shrink to unacceptable size(%u -> %u)", mpi_bits(a), bits);
        return NULL;
    }
}

/**
 *  duplicate big-numer |a|
 */
mpi_t *mpi_dup(const mpi_t *a)
{
    if (a == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: nullptr");
        return NULL;
    }

    mpi_t *r = mpi_create(mpi_bits(a));
    if (r != NULL) {
        /* attributes are set during creation */
        r->sign = a->sign;
        r->size = a->size;
        ZEXPAND(r->data, r->room, a->data, a->size);
    }

    return r;
}

/**
 *  copy big-numer |a| to |r|
 *
 * @note:
 *   1. resize |r| to proper size before copy
 */
int mpi_copy(mpi_t *r, const mpi_t *a)
{
    if (r == NULL || a == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "At least one of (dst, src) is NULL");
        return -EINVAL;
    }

    if (r != a) {
        if (r->room < a->size) {
            MPI_RAISE_ERROR(-ERANGE);
            return -ERANGE;
        }
        r->sign = a->sign;
        r->size = a->size;
        ZEXPAND(r->data, r->room, a->data, a->size);
    }

    return 0;
}
#define MPI_ABS_COPY(r, a)                  \
    ({                                      \
        int err;                            \
        if ((err = mpi_copy(r, a)) == 0) {  \
            r->sign = MPI_SIGN_NON_NEGTIVE; \
        } else {                            \
            MPI_RAISE_ERROR(err);           \
        }                                   \
        err;                                \
    })

/**
 * @addtogroup: mpi/information
 */
/**
 * get bit size of mpi |a|(constant-time version)
 *
 * @note:
 *   1. 0, if a is NULL
 */
unsigned int mpi_bits(const mpi_t *a)
{
    if (a == NULL) {
        MPI_RAISE_WARN("Invalid Integer: nullptr");
        return 0;
    }

    if (a->size == 0) { return 0; }
    return mpi_bits_consttime_bin(a->data, a->size);
}

/**
 * get byte size of mpi |a|(constant-time version)
 *
 * @note:
 *   1. 0, if a is NULL
 */
unsigned int mpi_bytes(const mpi_t *a)
{
    return (mpi_bits(a) + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
}

/**
 * get max bit size of mpi |a|(constant-time version)
 *
 * @note:
 *   1. 0, if a is NULL
 */
unsigned int mpi_max_bits(const mpi_t *a)
{
    if (a == NULL) {
        MPI_RAISE_WARN("Invalid Integer: nullptr");
        return 0;
    }

    return MPI_LIMB_BITS * a->room;
}

/**
 * get max byte size of mpi |a|(constant-time version)
 *
 * @note:
 *   1. 0, if a is NULL
 */
unsigned int mpi_max_bytes(const mpi_t *a)
{
    return (mpi_max_bits(a) + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
}

/**
 * compare mpi |a| and |b|
 *   0, if |a| = |b|
 *   1, if |a| > |b|
 *  -1, if |a| < |b|
 * otherwise, error code
 */
int mpi_cmp(const mpi_t *a, const mpi_t *b)
{
    if (a == NULL || b == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "At least one of (dst, src) is NULL");
        return a == b ? 0 : -EINVAL;
    }

    if (a->sign != b->sign) { return a->sign == MPI_SIGN_NON_NEGTIVE ? 1 : -1; }
    return mpi_ucmp_bin(a->data, a->size, b->data, b->size);
}

/**
 * zeroize mpi |r|
 */
int mpi_zeroize(mpi_t *r)
{
    if (r == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: nullptr");
        return -EINVAL;
    }

    r->sign = MPI_SIGN_NON_NEGTIVE;
    r->size = 0;
    ZEROIZE(r->data, 0, r->room);

    return 0;
}

/**
 * set mpi |r| to unsigned sigle-precision integer |w|
 */
int mpi_set_limb(mpi_t *r, mpi_limb_t w)
{
    if (r == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: nullptr");
        return -EINVAL;
    }
    if (r->room < 1) {
        MPI_RAISE_ERROR(-ERANGE, "Invalid Integer: room not enough");
        return -ERANGE;
    }

    mpi_zeroize(r);
    if (w != 0) { r->data[r->size++] = w; }

    return 0;
}

/**
 * @addtogroup: mpi/conversion
 */
/**
 *  initialize mpi |pr| from octets |buff|/|bufflen|
 *
 * @note:
 *   1. if *|pr| is NULL, mpi will be created with proper size
 *   2. if *|pr| isn't NULL, mpi will be resized, and maybe *|pr| will be set to a new memory chunk
 */
int mpi_from_octets(mpi_t **pr, const unsigned char *buff, unsigned int bufflen)
{
    if (pr == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid pointer to write back the result");
        return -EINVAL;
    }
    if (buff == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid input buffer");
        return -EINVAL;
    }
    if (bufflen > MPI_MAX_BITS / BITS_PER_BYTE) {
        MPI_RAISE_ERROR(-ERANGE, "bit-size exceed the limitation");
        return -ERANGE;
    }

    mpi_t *r = NULL;
    if (*pr == NULL) {
        if ((r = mpi_create(bufflen * BITS_PER_BYTE)) == NULL) {
            MPI_RAISE_ERROR(-ENOMEM);
            return -ENOMEM;
        }
        *pr = r;
    } else {
        mpi_zeroize(*pr);
        if ((r = mpi_expand(*pr, bufflen * BITS_PER_BYTE)) == NULL) {
            MPI_RAISE_ERROR(-ENOMEM);
            return -ENOMEM;
        }
        *pr = r;
    }

    r->size = mpi_from_octets_bin(r->data, r->room, buff, bufflen);

    return 0;
}

/**
 *  convert mpi to big-endian octets
 */
int mpi_to_octets(const mpi_t *a, unsigned char *out, unsigned int outsize, unsigned int *outlen)
{
    if (a == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: nullptr");
        return -EINVAL;
    }
    if (out == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid output buffer: nullptr");
        return -EINVAL;
    }

    unsigned int reqsize = mpi_bytes(a);
    if (reqsize > outsize) {
        MPI_RAISE_ERROR(-ERANGE);
        return -ERANGE;
    }
    if (outlen != NULL) { *outlen = reqsize; }
    mpi_to_octets_bin(out, outsize, a->data, a->size);

    return 0;
}

/**
 *  initialize mpi |pr| from hex-string |a|
 */
int mpi_from_string(mpi_t **pr, const char *a)
{
    if (pr == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid pointer to write back the result");
        return -EINVAL;
    }
    if (a == NULL || *a == '\0') {
        MPI_RAISE_ERROR(-EINVAL, "Invalid input string: nullptr, or empty string");
        return -EINVAL;
    }

    mpi_t *r = NULL;

    /* check input string and count number of hex-characters */
    unsigned int nchars = 0;
    {
        unsigned int neg = 0;
        if (*a == '-') {
            neg = 1;
            a++;
        }
        for (nchars = 0; isxdigit(a[nchars]) && nchars < MPI_MAX_BITS / BITS_PER_CHAR; nchars++) { continue; }
        if (nchars == 0) {
            MPI_RAISE_ERROR(-EINVAL);
            return -EINVAL;
        }
        if (nchars == MPI_MAX_BITS / BITS_PER_CHAR) {
            MPI_RAISE_ERROR(-ERANGE, "exceed the limit, number of input characters is %u", nchars);
            return -ERANGE;
        }

        if (*pr == NULL) {
            if ((r = mpi_create(nchars * BITS_PER_CHAR)) == NULL) {
                MPI_RAISE_ERROR(-ENOMEM);
                return -ENOMEM;
            }
            *pr = r;
        } else {
            mpi_zeroize(*pr);
            if ((r = mpi_expand(*pr, nchars * BITS_PER_CHAR)) == NULL) {
                MPI_RAISE_ERROR(-ENOMEM);
                return -ENOMEM;
            }
            *pr = r;
        }
        r->sign = neg ? MPI_SIGN_NEGTIVE : MPI_SIGN_NON_NEGTIVE;
    }
    r->size = mpi_from_string_bin(r->data, r->room, a, nchars);
    if (r->size == 0) { r->sign = MPI_SIGN_NON_NEGTIVE; }

    return 0;
}

/**
 *  convert mpi to string
 *
 * @note:
 *   1. FREE the return pointer after usage
 */
char *mpi_to_string(const mpi_t *a)
{
    if (a == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: nullptr");
        return NULL;
    }

    char *out = NULL;
    unsigned int outlen = a->size * MPI_LIMB_BYTES * 2 + 2; // case(a->size == 0) requires 2 bytes
    if ((out = (char *)MPI_ALLOCATE(outlen)) == NULL) {
        MPI_RAISE_ERROR(-ENOMEM);
        return NULL;
    }

    char *p = out;
    if (a->sign == MPI_SIGN_NEGTIVE) {
        MPI_ASSERT(a->size != 0);
        *p++ = '-';
        outlen--;
    }

    p += mpi_to_string_bin(p, outlen, a->data, a->size);
    *p = '\0';

    return out;
}

/**
 * @addtogroup: mpi/bit-operation
 */
/**
 * get bit
 */
int mpi_get_bit(const mpi_t *a, unsigned int n)
{
    if (a == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: nullptr");
        return -EINVAL;
    }

    unsigned int nw = n / MPI_LIMB_BITS;
    unsigned int nb = n % MPI_LIMB_BITS;
    if (UNLIKELY(nw >= a->size)) {
        return 0;
    } else {
        return (int)(((a->data[nw]) >> nb) & ((mpi_limb_t)1));
    }
}

/**
 * set bit
 */
int mpi_set_bit(const mpi_t *a, unsigned int n)
{
    if (a == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: nullptr");
        return -EINVAL;
    }

    unsigned int nw = n / MPI_LIMB_BITS;
    unsigned int nb = n % MPI_LIMB_BITS;
    if (UNLIKELY(nw >= a->size)) {
        MPI_RAISE_ERROR(-EINVAL, "Out of range, expand before operation.");

        return -EINVAL;
    } else {
        a->data[nw] |= (((mpi_limb_t)1) << nb);

        return 0;
    }
}

/**
 * clr bit
 */
int mpi_clr_bit(const mpi_t *a, unsigned int n)
{
    if (a == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: nullptr");
        return -EINVAL;
    }

    unsigned int nw = n / MPI_LIMB_BITS;
    unsigned int nb = n % MPI_LIMB_BITS;
    if (LIKELY(nw < a->size)) { a->data[nw] &= (~(((mpi_limb_t)1) << nb)); }

    return 0;
}

/**
 * left-shift: |r| = |a| << n
 */
int mpi_lshift(mpi_t *r, const mpi_t *a, unsigned int n)
{
    if (r == NULL || a == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: at least one of (src, dst) is nullptr");
        return -EINVAL;
    }

    if (mpi_bits(a) == 0) { return mpi_zeroize(r); }
    if (mpi_max_bits(r) < mpi_bits(a) + n) { // @IMPORTANT: addition here will never overflow under the limitations
        return -ERANGE;
    }

    r->sign = a->sign;
    r->size = mpi_lshift_bin(r->data, a->data, a->size, n);
    ZEROIZE(r->data, r->size, r->room);

    return 0;
}

/**
 * right-shift: |r| = |a| >> n
 */
int mpi_rshift(mpi_t *r, const mpi_t *a, unsigned int n)
{
    if (r == NULL || a == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: at least one of (src, dst) is nullptr");
        return -EINVAL;
    }

    if (mpi_bits(a) <= n) { return mpi_zeroize(r); }
    if (mpi_max_bits(r) + n < mpi_bits(a)) { return -ERANGE; }

    r->sign = a->sign;
    r->size = mpi_rshift_bin(r->data, a->data, a->size, n);
    ZEROIZE(r->data, r->size, r->room);

    return 0;
}

/**
 * @addtogroup: mpi/addition-subtraction
 */
static int mpi_uadd(mpi_t *r, const mpi_t *a, const mpi_t *b)
{
    MPI_ASSERT(r != NULL && a != NULL && b != NULL);
    MPI_ASSERT(a->sign == b->sign);

    if (a->size < b->size) { SWAP(const mpi_t *, a, b); }
    if (r->room < a->size) {
        MPI_RAISE_ERROR(-ERANGE);
        return -ERANGE; // resize before subtraction
    }
    mpi_limb_t carry = mpi_uadd_bin(r->data, r->room, a->data, a->size, b->data, b->size);
    if (carry != 0 && a->size >= r->room) {
        MPI_RAISE_ERROR(-ERANGE);
        return -ERANGE; // resize before subtraction
    }
    r->size = a->size + (carry != 0 ? 1 : 0);

    return 0;
}

static int mpi_usub(mpi_t *r, const mpi_t *a, const mpi_t *b)
{
    MPI_ASSERT(r != NULL && a != NULL && b != NULL);
    MPI_ASSERT(a->size >= b->size);

    if (a->size > r->room) {
        MPI_RAISE_ERROR(-ERANGE);
        return -ERANGE; // resize before subtraction
    }
    r->size = mpi_usub_bin(r->data, r->room, a->data, a->size, b->data, b->size);

    return 0;
}

/**
 * mpi addition: |r| = |a| + |b|
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: MAX(bit_size(a), bit_size(b)) + 1
 */
int mpi_add(mpi_t *r, const mpi_t *a, const mpi_t *b)
{
    if (r == NULL || a == NULL || b == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: at least one of (sum, addends) is nullptr");
        return -EINVAL;
    }

    if (a->sign == b->sign) {
        r->sign = a->sign;
        return mpi_uadd(r, a, b);
    } else {
        int res = mpi_ucmp_bin(a->data, a->size, b->data, b->size);
        if (res > 0) { // a > b
            r->sign = a->sign;
            return mpi_usub(r, a, b);
        } else if (res < 0) { // a < b
            r->sign = b->sign;
            return mpi_usub(r, b, a);
        } else {
            return mpi_zeroize(r);
        }
    }
}

static int mpi_uinc(mpi_t *r, const mpi_t *a, mpi_limb_t w)
{
    MPI_ASSERT(r != NULL && a != NULL);
    MPI_ASSERT(a->sign == MPI_SIGN_NON_NEGTIVE);

    if (r->room < a->size) {
        MPI_RAISE_ERROR(-ERANGE);
        return -ERANGE; // resize before addition
    }

    mpi_limb_t carry = mpi_uinc_bin(r->data, r->room, a->data, a->size, w);
    if (carry != 0 && a->size >= r->room) {
        MPI_RAISE_ERROR(-ERANGE);
        return -ERANGE; // resize before addition
    }
    r->size = a->size + (carry != 0 ? 1 : 0);

    return 0;
}

static int mpi_udec(mpi_t *r, const mpi_t *a, mpi_limb_t w)
{
    MPI_ASSERT(r != NULL && a != NULL);

    if (r->room < a->size) {
        MPI_RAISE_ERROR(-ERANGE);
        return -ERANGE; // resize before subtraction
    }
    if (a->size == 1 && a->data[0] < w) {
        MPI_RAISE_ERROR(-EINVAL);
        return -EINVAL; // resize before subtraction
    }
    r->size = mpi_udec_bin(r->data, r->room, a->data, a->size, w);

    return 0;
}

/**
 * mpi addition: |r| = |a| + w
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: MAX(bit_size(a), bit_size(w)) + 1
 */
int mpi_add_limb(mpi_t *r, const mpi_t *a, mpi_limb_t w)
{
    if (r == NULL || a == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: at least one of (sum, addend) is nullptr");
        return -EINVAL;
    }

    if (a->sign == MPI_SIGN_NON_NEGTIVE) {
        r->sign = MPI_SIGN_NON_NEGTIVE;
        return mpi_uinc(r, a, w);
    } else {
        if (a->size == 1) {
            if (a->data[0] != w && r->room < 1) {
                MPI_RAISE_ERROR(-ERANGE, "Resize before operation");
                return -ERANGE;
            }
            if (a->data[0] >= w) {
                r->data[0] = a->data[0] - w;
                r->size = mpi_fix_size_bin(r->data, 1);
            } else {
                r->sign = MPI_SIGN_NON_NEGTIVE;
                r->data[0] = w - a->data[0];
                r->size = 1;
            }
            return 0;
        } else {
            r->sign = MPI_SIGN_NEGTIVE;
            return mpi_udec(r, a, w);
        }
    }
}

/**
 * mpi subtraction: |r| = |a| - w
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: MAX(bit_size(a), bit_size(w))
 */
int mpi_sub_limb(mpi_t *r, const mpi_t *a, mpi_limb_t w)
{
    if (r == NULL || a == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: at least one of (r, a) is nullptr");
        return -EINVAL;
    }

    if (a->sign == MPI_SIGN_NEGTIVE) {
        r->sign = MPI_SIGN_NEGTIVE;
        return mpi_uinc(r, a, w);
    } else {
        if (a->size == 1) {
            if (a->data[0] != w && r->room < 1) {
                MPI_RAISE_ERROR(-ERANGE, "Resize before operation");
                return -ERANGE;
            }
            if (a->data[0] >= w) {
                r->data[0] = a->data[0] - w;
                r->size = mpi_fix_size_bin(r->data, 1);
            } else {
                r->sign = MPI_SIGN_NEGTIVE;
                r->data[0] = w - a->data[0];
                r->size = 1;
            }
            return 0;
        } else {
            r->sign = MPI_SIGN_NON_NEGTIVE;
            return mpi_udec(r, a, w);
        }
    }
}

/**
 * mpi subtraction: |r| = |a| - |b|
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: MAX(bit_size(a), bit_size(b))
 */
int mpi_sub(mpi_t *r, const mpi_t *a, const mpi_t *b)
{
    if (r == NULL || a == NULL || b == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: at least one of (r, a, b) is nullptr");
        return -EINVAL;
    }

    if (a->sign != b->sign) {
        r->sign = b->sign;
        return mpi_uadd(r, a, b);
    } else {
        int res = mpi_ucmp_bin(a->data, a->size, b->data, b->size);
        r->sign = res >= 0 ? MPI_SIGN_NON_NEGTIVE : MPI_SIGN_NEGTIVE;
        if (res == 0) {
            mpi_zeroize(r);
            return 0;
        } else if (res > 0 && a->sign == MPI_SIGN_NON_NEGTIVE) {
            return mpi_usub(r, a, b);
        } else {
            return mpi_usub(r, b, a);
        }
    }
}

/**
 * @addtogroup: mpi/multiplication
 */
/**
 * mpi multiplication: |r| = |a| * |b|
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: bit_size(a) + bit_size(b) + MPI_LIMB_BITS
 */
int mpi_mul(mpi_t *r, const mpi_t *a, const mpi_t *b)
{
    if (r == NULL || a == NULL || b == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: at least one of (r, a, b) is nullptr");
        return -EINVAL;
    }

    // special case: 0 * any = 0
    if (mpi_bits(a) == 0 || mpi_bits(b) == 0) { return mpi_zeroize(r); }

    unsigned int size = a->size + b->size;
    if (size < a->size || r->room < size) { // overflow or no enough spaces
        MPI_RAISE_ERROR(-ERANGE, "room: %u, required-room: %u", r->room, size);
        mpi_zeroize(r);
        return -ERANGE; // resize before addition
    }

    r->sign = a->sign != b->sign;
    mpi_umul_bin(r->data, a->data, a->size, b->data, b->size);
    r->size = mpi_fix_size_bin(r->data, size);

    return 0;
}

/**
 * mpi multiplication: |r| = |a| * |b|
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: bit_size(a) + bit_size(b)
 */
int mpi_mul_limb(mpi_t *r, const mpi_t *a, mpi_limb_t b)
{
    if (r == NULL || a == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: at least one of (r, a) is nullptr");
        return -EINVAL;
    }
    if (r->room < a->size) {
        MPI_RAISE_ERROR(-ERANGE, "Destination's space it not enough");
        return -ERANGE;
    }

    ZEROIZE(r->data, 0, r->room);
    r->size = a->size;
    mpi_limb_t extension = mpi_umul_acc_bin(r->data, a->data, a->size, b);
    if (extension != 0) {
        if (r->room >= a->size) { r->data[r->size++] = extension; }
    }
    r->size = mpi_fix_size_bin(r->data, r->size);

    return 0;
}

/**
 * mpi square: |r| = |a| ^ 2
 *
 * @note:
 *   1. make sure r->room is enough to store the result
 *      minimal advise size: 2 * bit_size(a)
 */
int mpi_sqr(mpi_t *r, const mpi_t *a)
{
#ifdef MPI_LOW_FOOTPRINT
    return mpi_mul(r, a, a);
#else
    if (r == NULL || a == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: at least one of (r, a) is nullptr");
        return -EINVAL;
    }
    if (r->room < 2 * a->size) {
        MPI_RAISE_ERROR(-ERANGE, "Destination's space it not enough");
        return -ERANGE;
    }

    if (a->size == 0) {
        mpi_zeroize(r);
        return 0;
    }

    mpi_usqr_bin(r->data, a->data, a->size);
    r->size = mpi_fix_size_bin(r->data, 2 * a->size);

    return 0;
#endif
}

/**
 * @addtogroup: mpi/division
 */
/**
 * mpi division: |q|, |r| = |x| / |y|, |x| = |q| * |y| + |r|(0 <= |r| < |y|)
 *
 * @note:
 *   1. make sure room of |q|, |r| is enough to store the result
 *      minimal advise size: bit_size(r) = bit_size(y)
 */
int mpi_div(mpi_t *q, mpi_t *r, const mpi_t *x, const mpi_t *y)
{
    if (x == NULL || y == NULL || (q == NULL && r == NULL)) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: nullptr");
        return -EINVAL;
    }
    if (mpi_bits(y) == 0) {
        MPI_RAISE_ERROR(-EINVAL, "Syntax error: divide by zero");
        return -EINVAL;
    }
    if (r != NULL && mpi_max_bits(r) < mpi_bits(x)) {
        MPI_RAISE_ERROR(-ERANGE, "Remainder's space it not enough");
        return -ERANGE;
    }

    int ret = mpi_ucmp_bin(x->data, x->size, y->data, y->size);
    if (ret < 0) { // special case: x < y, q = 0, r = x
        mpi_zeroize(q);
        return MPI_ABS_COPY(r, x);
    } else if (ret == 0) { // special case: x == y, q = 1/-1, r = 0
        mpi_zeroize(r);
        int err = mpi_set_limb(q, 1);
        if (err == 0) { q->sign = x->sign != y->sign; }
        return err;
    }

    if (q != NULL) { ZEROIZE(q->data, 0, x->size - y->size); }
    if (r != NULL) { mpi_copy(r, x); }

    unsigned int qsize = q != NULL ? q->room : 0;
    unsigned int rsize = r != NULL ? r->size : 0;
    mpi_limb_t *qdata = q != NULL ? q->data : NULL;
    mpi_limb_t *rdata = r != NULL ? r->data : NULL;

    rsize = mpi_udiv_bin(qdata, &qsize, rdata, rsize, y->data, y->size);
    if (q != NULL) {
        q->size = qsize;
        q->sign = x->sign == y->sign ? MPI_SIGN_NON_NEGTIVE : MPI_SIGN_NEGTIVE;
    }
    if (r != NULL) {
        r->size = rsize;
        r->sign = MPI_SIGN_NON_NEGTIVE;
    }

    return 0;
}

/**
 * mpi modular: r = a mod m
 */
int mpi_mod(mpi_t *r, const mpi_t *a, const mpi_t *m)
{
    return mpi_div(NULL, r, a, m);
}

/**
 * mpi division: q, r = a / w
 */
mpi_limb_t mpi_div_limb(mpi_t *a, mpi_limb_t w)
{
    /* special cases */
    {
        if (w == 0) { /* actually this an error (division by zero) */
            return (mpi_limb_t)-1;
        }
        if (a == NULL || a->size == 0) { return 0; }
    }

    mpi_limb_t rem = 0;
    unsigned int shifts = mpi_nlz_limb_consttime(w);

    /* normalize input (so UDIV_NND doesn't complain) */
    {
        w <<= shifts;
        if (mpi_lshift(a, a, shifts) != 0) { return (mpi_limb_t)-1; }
    }

    {
        for (unsigned int i = a->size; i > 0; i--) {
            mpi_limb_t l = a->data[i - 1], quo;
            UDIV_NND(quo, rem, rem, l, w);
            a->data[i - 1] = quo;
        }
        if ((a->size > 0) && (a->data[a->size - 1] == 0)) { a->size--; }
        if (!a->size) { a->sign = MPI_SIGN_NON_NEGTIVE; /* don't allow negative zero */ }
        a->size = mpi_fix_size_bin(a->data, a->size);
    }

    /* de-normalize */
    {
        rem >>= shifts;
    }

    return rem;
}

/**
 * mpi modular: r = a mod m
 */
mpi_limb_t mpi_mod_limb(const mpi_t *a, mpi_limb_t w)
{
    if (w == 0) { return (mpi_limb_t)-1; }

    /*
     * If |w| is too long and we don't have BN_ULLONG then we need to fall
     * back to using mpi_div_limb
     */
    if (w > ((mpi_limb_t)1 << BITS_S4)) {
        mpi_t *tmp = mpi_dup(a);
        if (tmp == NULL) { return (mpi_limb_t)-1; }

        mpi_limb_t rem = mpi_div_limb(tmp, w);
        mpi_destory(tmp);

        return rem;
    } else {
        mpi_limb_t rem = 0;
        for (unsigned int i = a->size; i > 0; i--) {
            /* rem < ((mpi_limb_t)1 << BITS_S4), won't overflow */
            mpi_limb_t aa = a->data[i - 1];
            rem = ((rem << BITS_S4) | ((aa >> BITS_S4) & MPI_LIMB_MASK_LO)) % w;
            rem = ((rem << BITS_S4) | (aa & MPI_LIMB_MASK_LO)) % w;
        }
        return rem;
    }
}

/**
 * @addtogroup: mpi/exponentiation
 */
/**
 * mpi exponentiation: r = g ^ e
 */
int mpi_exp(mpi_t *r, const mpi_t *g, const mpi_t *e)
{
    // FIXME
    if (r == NULL || g == NULL || e == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: nullptr");
        return -EINVAL;
    }

    int err;
    unsigned int tbits = (mpi_bits(g) + 1) << mpi_bits(e);
    mpi_t *t = mpi_create(tbits);

    if ((err = mpi_copy(t, g)) != 0) {
        MPI_RAISE_ERROR(err);
        mpi_destory(t);
        return err;
    }
    unsigned int ebits = mpi_bits(e);

    if (e->data[0] & 0x1) {
        if ((err = mpi_copy(r, g)) != 0) {
            MPI_RAISE_ERROR(err);
            mpi_destory(t);
            return err;
        }
    } else {
        if ((err = mpi_set_limb(r, 1)) != 0) {
            MPI_RAISE_ERROR(err);
            mpi_destory(t);
            return err;
        }
    }

    mpi_limb_t *p = e->data, mask = 0x1 << 1;
    for (unsigned int i = 1; i < ebits;) {
        if ((err = mpi_sqr(t, t)) != 0) {
            MPI_RAISE_ERROR(err);
            mpi_destory(t);
            return err;
        }
        if (*p & mask) {
            if ((err = mpi_mul(r, r, t)) != 0) {
                MPI_RAISE_ERROR(err);
                mpi_destory(t);
                return err;
            }
        }
        mask <<= 1;
        if (++i % MPI_LIMB_BITS == 0) {
            p++;
            mask = 0x1;
        }
    }

    mpi_destory(t);

    return 0;
}

/**
 * mpi exponentiation(word): r = g ^ e
 */
int mpi_exp_limb(mpi_t *r, const mpi_t *g, mpi_limb_t e)
{
    // FIXME
    mpi_t v;
    mpi_limb_t dataE = e;
    mpi_make(&v, &dataE, 1);
    return mpi_exp(r, g, &v);
}


/**
 * @addtogroup: mpi/greatest-common-divisor
 */
typedef struct {
    mpi_limb_t H, L;
} mpi_dlimb_t;

/**
 * mpi(binary): extension, r[] = a[] * x + b[] * y
 */
static mpi_dlimb_t mpi_uadd_with_multiplier_bin(mpi_limb_t *r, const mpi_limb_t *a, mpi_limb_t x, const mpi_limb_t *b, mpi_limb_t y, unsigned int n)
{
    mpi_limb_t extensionH = 0, extensionL = 0;
    for (unsigned int i = 0; i < n; i++) {
        mpi_limb_t aH, aL, bH, bL, rH;

        UMUL_AB(aH, aL, a[i], x); // aH, aL = a[i] * x
        UMUL_AB(bH, bL, b[i], y); // bH, bL = b[i] * y

        // rH, r[i] = aL + bL + extensionL
        UADD_ABC(rH, r[i], aL, bL, extensionL);

        // extensionH, extensionL = aH + bH + extensionH
        UADD_ABC(extensionH, extensionL, aH, bH, extensionH);

        extensionL += rH;
    }

    return (mpi_dlimb_t){extensionH, extensionL};
}

/**
 * mpi(binary): borrow, r[] = a[] * x - b[] * y
 */
static mpi_limb_t mpi_usub_with_multiplier_bin(mpi_limb_t *r, const mpi_limb_t *a, mpi_limb_t x, const mpi_limb_t *b, mpi_limb_t y, unsigned int n)
{
    MPI_ASSERT(r != NULL);
    MPI_ASSERT(a != NULL);
    MPI_ASSERT(b != NULL);

    mpi_limb_t borrow = 0, extension = 0;
    for (unsigned int i = 0; i < n; i++) {
        mpi_limb_t aH, aL, bH, bL, t;

        UMUL_AB(aH, aL, a[i], x); // aH, aL = a[i] * x
        UMUL_AB(bH, bL, b[i], y); // bH, bL = b[i] * y

        // aH, aL = (aH, aL) + (0, extension)
        UADD_AB(t, aL, aL, extension);
        aH += t; // will never overflow

        // borrow, r[i] = aL - bL
        USUB_AB(t, r[i], aL, bL);
        borrow += t; // will never overflow

        // borrow, extension = aH - bH - borrow
        USUB_ABC(borrow, extension, aH, bH, borrow);
    }

    if (borrow > 0) {
        r[0] = 0 - r[0];
        for (unsigned int i = 1; i < n; i++) { r[i] = (0 - r[i]) - 1; }
    }

    return borrow;
}

static mpi_limb_t __gcd_lehmer_common_quotient(mpi_limb_t x, mpi_limb_t y, mpi_slimb_t A, mpi_slimb_t B, mpi_slimb_t C, mpi_slimb_t D)
{
#define __ADD(n1, n0, uA, sB)                   \
    if (sB >= 0) {                              \
        UADD_AB(n1, n0, uA, (mpi_limb_t)sB);    \
    } else {                                    \
        USUB_AB(n1, n0, uA, (mpi_limb_t)(-sB)); \
    }

    mpi_limb_t q, q1;
    {
        mpi_limb_t nH, nL, dH, dL, rH, rL;
        __ADD(nH, nL, x, A);
        __ADD(dH, dL, y, C);

        UDIV_NNDD(q, rH, rL, nH, nL, dH, dL);

        __ADD(nH, nL, x, B);
        __ADD(dH, dL, y, D);

        UDIV_NNDD(q1, rH, rL, nH, nL, dH, dL);

        (void)rH;
        (void)rL;
    }
#undef __ADD

    return q == q1 ? q : MPI_LIMB_MASK;
}

/**
 * mpi: greatest common divisor(Lehmer's gcd algorithm)
 */
int mpi_gcd(mpi_t *r, const mpi_t *a, const mpi_t *b, mpi_optimizer_t *optimizer)
{
    if (r == NULL || a == NULL || b == NULL || r->room < 1) {
        MPI_RAISE_ERROR(-EINVAL);
        return -EINVAL;
    }

    mpi_optimizer_t *opt = optimizer;
    if (opt == NULL) {
        opt = mpi_optimizer_create(r->room + a->room + b->room);
        if (opt == NULL) {
            MPI_RAISE_ERROR(-ENOMEM);
            return -ENOMEM;
        }
    }

    int err = 0;
    {
        mpi_limb_t *rr = r->data, *aa = a->data, *bb = b->data;
        unsigned int rsize = r->room, asize = a->size, bsize = b->size;

        r->sign = MPI_SIGN_NON_NEGTIVE; /* clear sign flag */

        // special cases
        {
            // Lehmer's algorithm requires that first number must be greater than the second
            int res = mpi_ucmp_bin(aa, asize, bb, bsize);
            if (res < 0) {
                SWAP(unsigned int, asize, bsize);
                SWAP(mpi_limb_t *, aa, bb);
            } else if (res == 0 || bsize == 0) {
                ZEXPAND(rr, rsize, aa, asize); // the result is truncated if rsize < xsize
                r->size = asize;               // expected size will be returned

                return 0;
            }

            if (asize == 1) {
                rr[0] = mpi_gcd_limb(aa[0], bb[0]);
                r->size = 1;

                return 0;
            }
        }

        // common cases
        {
            mpi_limb_t *xtemp = mpi_optimizer_get_limbs(opt, asize);
            mpi_limb_t *ytemp = mpi_optimizer_get_limbs(opt, bsize);
            mpi_limb_t *gtemp = mpi_optimizer_get_limbs(opt, rsize);

            if (xtemp == NULL || ytemp == NULL || gtemp == NULL) {
                MPI_RAISE_ERROR(-ENOMEM);
                err = -ENOMEM;

                goto operation_end;
            }

            mpi_limb_t *gdata = rr;
            unsigned int xsize = asize, xroom = asize;
            unsigned int ysize = bsize, yroom = bsize;
            unsigned int groom = rsize;

            ZEXPAND(xtemp, xroom, aa, xsize);
            ZEXPAND(ytemp, yroom, bb, ysize);

            ZEROIZE(gtemp, 0, groom);
            ZEROIZE(gdata, 0, groom);

            mpi_limb_t *T = gtemp, *u = gdata;
            while (xsize > 1) {
                /* xx and yy is the high-order digit of x and y (yy could be 0) */
                mpi_limb_t xx = (mpi_limb_t)(xtemp[xsize - 1]);
                mpi_limb_t yy = (ysize < xsize) ? 0 : (mpi_limb_t)(ytemp[ysize - 1]);

                mpi_slimb_t AA = 1, BB = 0, CC = 0, DD = 1;
                while (yy != (mpi_limb_t)(-CC) && yy != (mpi_limb_t)(-DD)) {
                    mpi_limb_t q = __gcd_lehmer_common_quotient(xx, yy, AA, BB, CC, DD);
                    if (q == MPI_LIMB_MASK) { break; }
                    mpi_slimb_t t = AA - (mpi_slimb_t)q * CC;
                    AA = CC;
                    CC = t;
                    t = BB - (mpi_slimb_t)q * DD;
                    BB = DD;
                    DD = t;
                    t = (mpi_slimb_t)(xx - q * yy);
                    xx = yy;
                    yy = (mpi_limb_t)t;
                }

                if (BB == 0) {
                    /* T = x mod y */
                    unsigned int tsize = mpi_umod_bin(xtemp, xsize, ytemp, ysize);
                    ZEXPAND(T, groom, xtemp, tsize);
                    /* a = b; b = T; */
                    ZEXPAND(xtemp, xroom, ytemp, ysize);
                    ZEXPAND(ytemp, yroom, T, ysize);
                } else {
                    /* T = AA * x + BB * y */
                    if ((AA >= 0) && (BB < 0)) {
                        /* T = A * x + B * y = A * x - (-B) * y */
                        mpi_usub_with_multiplier_bin(T, xtemp, (mpi_limb_t)AA, ytemp, (mpi_limb_t)(-BB), ysize);
                    } else if ((AA <= 0) && (BB > 0)) {
                        /* T = A * x + B * y = B * y - (-A) * x */
                        mpi_usub_with_multiplier_bin(T, ytemp, (mpi_limb_t)BB, xtemp, (mpi_limb_t)(-AA), ysize);
                    } else {
                        /* AA * BB >= 0 */
                        mpi_uadd_with_multiplier_bin(T, xtemp, (mpi_limb_t)AA, ytemp, (mpi_limb_t)BB, ysize);
                    }

                    /* u = CC * x + DD * y */
                    if ((CC <= 0) && (DD >= 0)) {
                        mpi_usub_with_multiplier_bin(u, ytemp, (mpi_limb_t)DD, xtemp, (mpi_limb_t)(-CC), ysize);
                    } else if ((CC >= 0) && (DD <= 0)) {
                        /* u= C * x + D * y = C * x - (-D) * y */
                        mpi_usub_with_multiplier_bin(u, xtemp, (mpi_limb_t)CC, ytemp, (mpi_limb_t)(-DD), ysize);
                    } else {
                        /* CC * DD >= 0 */
                        mpi_uadd_with_multiplier_bin(u, xtemp, (mpi_limb_t)CC, ytemp, (mpi_limb_t)DD, ysize);
                    }

                    /* x = T; y = u */
                    COPY(xtemp, T, ysize);
                    COPY(ytemp, u, ysize);
                }

                xsize = mpi_fix_size_bin(xtemp, xsize);
                ysize = mpi_fix_size_bin(ytemp, ysize);

                if (ysize > xsize) {
                    SWAP(unsigned int, xsize, ysize);
                    SWAP(mpi_limb_t *, aa, bb);
                }

                if (ysize == 1 && ytemp[ysize - 1] == 0) {
                    /* End evaluation */
                    ZEXPAND(gdata, groom, xtemp, xsize);
                    r->size = xsize;

                    goto operation_end;
                }
            }

            rr[0] = mpi_gcd_limb(xtemp[0], ytemp[0]);
            r->size = 1;

        operation_end:
            if (gtemp != NULL) { mpi_optimizer_put_limbs(opt, rsize); }
            if (ytemp != NULL) { mpi_optimizer_put_limbs(opt, bsize); }
            if (xtemp != NULL) { mpi_optimizer_put_limbs(opt, asize); }
            if (optimizer == NULL && opt != NULL) { mpi_optimizer_destory(opt); }
        }
    }

    return err;
}

/**
 * conditional swap(constant-time version)
 */
int mpi_swap_consttime(unsigned condition, mpi_t *a, mpi_t *b, unsigned int n)
{
    if (a == b) { return 0; }
    if (a == NULL || b == NULL) { return -EINVAL; }
    if (a->room < n || b->room < n) { return -EINVAL; }

    // conditionally swap the size
    {
        unsigned int cond = condition, t;
        cond = ((~cond & ((cond - 1))) >> (sizeof(unsigned int) * BITS_PER_BYTE - 1)) - 1;

        // swap size
        {
            t = (a->size ^ b->size) & cond;
            a->size ^= t;
            b->size ^= t;
        }

        // swap number sign
        {
            t = (a->sign ^ b->sign) & cond;
            a->sign ^= t;
            b->sign ^= t;
        }
    }

    // conditionally swap the data
    {
        mpi_limb_t cond = condition, t;
        cond = ((~cond & ((cond - 1))) >> (sizeof(mpi_limb_t) * BITS_PER_BYTE - 1)) - 1;
        for (unsigned int i = 0; i < n; i++) {
            t = (a->data[i] ^ b->data[i]) & cond;
            a->data[i] ^= t;
            b->data[i] ^= t;
        }
    }

    return 0;
}

/**
 * greatest common divisor(constant-time version)
 *
 * @note:
 *    1. reference: openssl/bn_gcd.c; <Fast constant-time gcd computation and modular inversion>
 */
int mpi_gcd_consttime(mpi_t *r, const mpi_t *a, const mpi_t *b, mpi_optimizer_t *optimizer)
{
    if (r == NULL || a == NULL || b == NULL) { return -EINVAL; }

    {
        /**
         * special cases: at least one of (a, b) is zero
         *
         * @security:
         *   assume that no side-channel information the attacker needed under the case
         */
        if (mpi_bits(b) == 0) { return MPI_ABS_COPY(r, a); }
        if (mpi_bits(a) == 0) { return MPI_ABS_COPY(r, b); }
    }

    {
        /* general cases */
        int err = 0;
        unsigned int abits = mpi_bits(a);
        unsigned int bbits = mpi_bits(b);
        unsigned int mbits = abits >= bbits ? abits : bbits;
        if (mpi_max_bits(r) < mbits + MPI_LIMB_BITS) { return -ERANGE; }

        mpi_optimizer_t *opt = optimizer;
        if (opt == NULL) {
            opt = mpi_optimizer_create(r->room + a->room + b->room);
            if (opt == NULL) {
                MPI_RAISE_ERROR(-ENOMEM);
                return -ENOMEM;
            }
        }

        /* allocate buffers */
        mpi_t *g = mpi_optimizer_get(opt, MPI_BITS_TO_LIMBS(mbits) + 1);
        mpi_t *t = mpi_optimizer_get(opt, MPI_BITS_TO_LIMBS(mbits) + 2);
        if (g == NULL || t == NULL) {
            err = -ENOMEM;
            goto operation_end;
        }

        /* find shared powers of two */
        mpi_limb_t bit = 1;
        unsigned int shifts = 0;
        for (unsigned i = 0; i < a->room && i < b->room; i++) {
            mpi_limb_t mask = ~(a->data[i] | b->data[i]);
            for (unsigned int j = 0; j < MPI_LIMB_BITS; j++) {
                bit &= mask;
                shifts += (unsigned int)bit;
                mask >>= 1;
            }
        }
        /* subtract shared powers of two */
        mpi_rshift(r, a, shifts);
        mpi_rshift(g, b, shifts);

        /* expand to biggest nword, with room for a possible extra word */
        unsigned top = 1 + ((r->size >= g->size) ? r->size : g->size);
        MPI_ASSERT(r->room >= top && g->room >= top && t->room >= top); // ensured via `mpi_max_bits(r) < mbits` checking

        /* re-arrange inputs s.t. r is odd */
        mpi_swap_consttime((~r->data[0]) & 1, r, g, top);

        /* compute the number of iterations */
        unsigned int rlen = mpi_bits(r);
        unsigned int glen = mpi_bits(g);
        unsigned int m = 4 + 3 * ((rlen >= glen) ? rlen : glen); // will never overflow under the limitations

        int delta = 1;
        for (unsigned i = 0; i < m; i++) {
            /* conditionally flip signs if delta is positive and g is odd */
            unsigned cond = (unsigned)(-delta >> (8 * sizeof(delta) - 1)) /* delta is positive */
                            & (unsigned)(g->data[0] & 1)                  /* g is odd */
                            & (unsigned)(~((g->size - 1) >> (sizeof(g->size) * 8 - 1))) /* g->size > 0 */;
            delta = (int)((-cond & (unsigned)(-delta)) | ((cond - 1) & (unsigned)delta)) + 1;
            r->sign ^= cond;
            mpi_swap_consttime(cond, r, g, top);

            mpi_add(t, g, r);

            cond = (unsigned)(g->data[0] & 1) /* g is odd */
                   & (unsigned)(~((g->size - 1) >> (sizeof(g->size) * 8 - 1))) /* g->size > 0 */;
            mpi_swap_consttime(cond, g, t, top);
            mpi_rshift(g, g, 1);
        }
        r->sign = MPI_SIGN_NON_NEGTIVE;

        /* add powers of 2 removed */
        mpi_lshift(r, r, shifts);

    operation_end:
        if (t != NULL) { mpi_optimizer_put(optimizer, MPI_BITS_TO_LIMBS(mbits) + 2); }
        if (g != NULL) { mpi_optimizer_put(optimizer, MPI_BITS_TO_LIMBS(mbits) + 1); }
        if (optimizer == NULL && opt != NULL) { mpi_optimizer_destory(opt); }

        return err;
    }
}
