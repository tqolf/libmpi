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
#ifndef MULTIPLE_PRECISION_OPTIMIZER_H
#define MULTIPLE_PRECISION_OPTIMIZER_H

#include <mpn/mpn-conf.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct mpn_optimizer_t {
    unsigned int size;            /**< offset of used chunk */
    unsigned int room;            /**< max size of chunk */
    mpn_limb_t *chunk;            /**< mpn chunk */
    struct mpn_optimizer_t *next; /**< next optimizer node */
} mpn_optimizer_t;

/**
 * mpn optimizer: create optimizer for mpn operation
 *
 * @note:
 *   1. room: room size of optimizer chunk, in unit of 'mpn_limb_t'
 */
mpn_optimizer_t *mpn_optimizer_create(unsigned int room);

/**
 * mpn optimizer: destory optimizer
 */
void mpn_optimizer_destory(mpn_optimizer_t *opt);

/**
 * mpn optimizer: get memory chunk for mpn operation
 *
 * @note:
 *   1. size: size of chunk, in unit of 'mpn_limb_t'
 */
mpn_limb_t *mpn_optimizer_get_limbs(mpn_optimizer_t *opt, unsigned int size);

/**
 * mpn optimizer: put back memory chunk
 */
void mpn_optimizer_put_limbs(mpn_optimizer_t *optimizer, unsigned int size);

/**
 * mpn optimizer: reset optimizer, mark all as unused
 */
void mpn_optimizer_reset(mpn_optimizer_t *opt);

#if defined(__cplusplus)
}
#endif

#endif
