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
#include "mpn-binary.h"

/**
 * mpn optimizer: create optimizer for mpn operation
 *
 * @note:
 *   1. room: room size of optimizer chunk, in unit of 'mpn_limb_t'
 */
mpn_optimizer_t *mpn_optimizer_create(mpn_size_t room)
{
    if (room == 0) {
        /* it's meaningless to create 0-length optimizer */
        return NULL;
    }
    size_t size = sizeof(mpn_optimizer_t) + MPN_LIMB_BYTES + room * sizeof(mpn_limb_t);
    mpn_optimizer_t *optimizer = (mpn_optimizer_t *)MPI_ALLOCATE(size);
    if (optimizer != NULL) {
        optimizer->size = 0;
        optimizer->next = NULL;
        optimizer->room = room;
        optimizer->chunk = mpi_aligned_pointer((unsigned char *)optimizer + sizeof(mpn_optimizer_t), MPN_LIMB_BYTES);
    }

    return optimizer;
}

/**
 * mpn optimizer: reset optimizer, mark all as unused
 */
void mpn_optimizer_reset(mpn_optimizer_t *optimizer)
{
    mpn_optimizer_t *curr = optimizer;
    while (curr != NULL) {
        curr->size = 0;
        curr = curr->next;
    }
}

/**
 * mpn optimizer: destory optimizer
 */
void mpn_optimizer_destory(mpn_optimizer_t *optimizer)
{
    mpn_optimizer_t *curr = optimizer, *next;
    while (curr != NULL) {
        next = curr->next;
        MPI_DEALLOCATE(curr); /* cleanse and free mpn_optimizer_t node */
        curr = next;
    }
}

/**
 * mpn optimizer: get memory chunk for mpn operation
 *
 * @note:
 *   1. size: size of chunk, in unit of 'mpn_limb_t'
 */
mpn_limb_t *mpn_optimizer_get_limbs(mpn_optimizer_t *optimizer, mpn_size_t size)
{
    if (optimizer == NULL) {
        MPI_RAISE_ERROR(-EINVAL);
        return NULL;
    }
    if (size == 0) { return NULL; }

    mpn_size_t total = 0;
    mpn_optimizer_t *curr = optimizer, *prev = NULL;
    while (curr != NULL) {
        total += curr->size;
        prev = curr;
        curr = curr->next;
    }

    if (prev->room - prev->size >= size) {
        curr = prev;
    } else {
        mpn_size_t room = size + total / 2; // XXX: optimize growth rule
        prev->next = curr = mpn_optimizer_create(room);
    }

    if (curr != NULL) {
        mpn_limb_t *p = &curr->chunk[curr->size];
        curr->size += size;

        return p;
    } else {
        MPI_RAISE_ERROR(-ENOMEM);

        return NULL;
    }
}

/**
 * mpn optimizer: put back memory chunk
 */
void mpn_optimizer_put_limbs(mpn_optimizer_t *optimizer, mpn_size_t size)
{
    if (optimizer == NULL) { return; }

    mpn_optimizer_t *curr = optimizer, *prev = NULL;
    while (curr != NULL) {
        prev = curr;
        curr = curr->next;
    }

    if (prev->size >= size) { prev->size -= size; }
}
