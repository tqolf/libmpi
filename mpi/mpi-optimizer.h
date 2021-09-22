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
#ifndef MULTIPLE_PRECISION_INTEGER_OPTIMIZER_H
#define MULTIPLE_PRECISION_INTEGER_OPTIMIZER_H

#include <mpi/mpi-conf.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct mpi_optimizer_t {
    unsigned int size;            /**< offset of used chunk */
    unsigned int room;            /**< max size of chunk */
    mpi_limb_t *chunk;            /**< mpi chunk */
    struct mpi_optimizer_t *next; /**< next optimizer node */
} mpi_optimizer_t;

/**
 * mpi(optimizer): create optimizer for mpi operation
 *
 * @note:
 *   1. room: room size of optimizer chunk, in unit of 'mpi_limb_t'
 */
mpi_optimizer_t *mpi_optimizer_create(unsigned int room);

/**
 * mpi(optimizer): destory optimizer
 */
void mpi_optimizer_destory(mpi_optimizer_t *opt);

/**
 * mpi(optimizer): get memory chunk for mpi operation
 *
 * @note:
 *   1. size: size of chunk, in unit of 'mpi_limb_t'
 */
mpi_limb_t *mpi_optimizer_get_limbs(mpi_optimizer_t *opt, unsigned int size);

/**
 * mpi(optimizer): put back memory chunk
 */
void mpi_optimizer_put_limbs(mpi_optimizer_t *optimizer, unsigned int size);

/**
 * mpi(optimizer): get mpi with specified room from optimizer
 *
 * @note:
 *   1. size: size of chunk, in unit of 'mpi_limb_t'
 */
mpi_t *mpi_optimizer_get(mpi_optimizer_t *optimizer, unsigned int size);

/**
 * mpi(optimizer): put back mpi of specified room
 */
void mpi_optimizer_put(mpi_optimizer_t *optimizer, unsigned int size);

/**
 * mpi(optimizer): reset optimizer, mark all as unused
 */
void mpi_optimizer_reset(mpi_optimizer_t *opt);

#if defined(__cplusplus)
}
#endif

#endif
