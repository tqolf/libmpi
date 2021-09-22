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
 * @addtogroup: mpi/optimizer
 */
/**
 * mpi(optimizer): create optimizer for mpi operation
 *
 * @note:
 *   1. room: room size of optimizer chunk, in unit of 'mpi_limb_t'
 */
mpi_optimizer_t *mpi_optimizer_create(unsigned int room)
{
    if (room == 0) {
        /* it's meaningless to create 0-length optimizer */
        return NULL;
    }
    size_t size = sizeof(mpi_optimizer_t) + MPI_LIMB_BYTES + room * sizeof(mpi_limb_t);
    mpi_optimizer_t *optimizer = (mpi_optimizer_t *)MPI_ALLOCATE(size);
    if (optimizer != NULL) {
        optimizer->size = 0;
        optimizer->next = NULL;
        optimizer->room = room;
        optimizer->chunk = mpi_aligned_pointer((unsigned char *)optimizer + sizeof(mpi_optimizer_t), MPI_LIMB_BYTES);
    }

    return optimizer;
}

/**
 * mpi(optimizer): reset optimizer, mark all as unused
 */
void mpi_optimizer_reset(mpi_optimizer_t *optimizer)
{
    mpi_optimizer_t *curr = optimizer;
    while (curr != NULL) {
        curr->size = 0;
        curr = curr->next;
    }
}

/**
 * mpi(optimizer): destory optimizer
 */
void mpi_optimizer_destory(mpi_optimizer_t *optimizer)
{
    mpi_optimizer_t *curr = optimizer, *next;
    while (curr != NULL) {
        next = curr->next;
        MPI_DEALLOCATE(curr); /* cleanse and free mpi_optimizer_t node */
        curr = next;
    }
}

/**
 * mpi(optimizer): get memory chunk for mpi operation
 *
 * @note:
 *   1. size: size of chunk, in unit of 'mpi_limb_t'
 */
mpi_limb_t *mpi_optimizer_get_limbs(mpi_optimizer_t *optimizer, unsigned int size)
{
    if (optimizer == NULL) {
        MPI_RAISE_ERROR(-EINVAL);
        return NULL;
    }
    if (size == 0) { return NULL; }

    unsigned int total = 0;
    mpi_optimizer_t *curr = optimizer, *prev = NULL;
    while (curr != NULL) {
        total += curr->size;
        prev = curr;
        curr = curr->next;
    }

    if (prev->room - prev->size >= size) {
        curr = prev;
    } else {
        unsigned int room = size + total / 2; // XXX: optimize growth rule
        prev->next = curr = mpi_optimizer_create(room);
    }

    if (curr != NULL) {
        mpi_limb_t *p = &curr->chunk[curr->size];
        curr->size += size;

        return p;
    } else {
        MPI_RAISE_ERROR(-ENOMEM);

        return NULL;
    }
}

/**
 * mpi(optimizer): put back memory chunk
 */
void mpi_optimizer_put_limbs(mpi_optimizer_t *optimizer, unsigned int size)
{
    if (optimizer == NULL) { return; }

    mpi_optimizer_t *curr = optimizer, *prev = NULL;
    while (curr != NULL) {
        prev = curr;
        curr = curr->next;
    }

    if (prev->size >= size) { prev->size -= size; }
}

/**
 * mpi(optimizer): get mpi with specified room from optimizer
 *
 * @note:
 *   1. size: size of chunk, in unit of 'mpi_limb_t'
 */
mpi_t *mpi_optimizer_get(mpi_optimizer_t *optimizer, unsigned int size)
{
    mpi_limb_t *chunk = mpi_optimizer_get_limbs(optimizer, MPI_ALIGNED_HEAD_LIMBS + size);
    if (chunk != NULL) {
        mpi_t *r = (mpi_t *)chunk;
        ZEROIZE(&chunk[MPI_ALIGNED_HEAD_LIMBS], 0, size);
        mpi_make(r, &chunk[MPI_ALIGNED_HEAD_LIMBS], size);

        return r;
    }

    return NULL;
}

/**
 * mpi(optimizer): put back mpi of specified room
 */
void mpi_optimizer_put(mpi_optimizer_t *optimizer, unsigned int size)
{
    mpi_optimizer_put_limbs(optimizer, MPI_ALIGNED_HEAD_LIMBS + size);
}
