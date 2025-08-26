/*
 * QEMU Deterministic Execution Support
 *
 * Copyright (c) 2025 Kyle Ambroff-Kao <kyle@ambroffkao.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#ifndef QEMU_DETERMINISTIC_H
#define QEMU_DETERMINISTIC_H

#include <stdint.h>
#include <stdbool.h>

typedef struct DeterministicConfig {
    bool enabled;           /* enable deterministic mode */
    uint64_t random_seed;   /* seed for pseudo-random generator */
    uint64_t start_time_ns; /* initial virtual time in nanoseconds */
    uint64_t instr_slice;   /* number of instructions per scheduler slice */
    bool force_icount;      /* force ICOUNT_PRECISE */
    bool disable_mttcg;     /* disable multi-thread TCG */
} DeterministicConfig;

extern DeterministicConfig deterministic_cfg;

/* Initialize deterministic configuration with defaults */
void deterministic_init_config(void);

/* Check if deterministic mode is enabled */
static inline bool deterministic_enabled(void)
{
    return deterministic_cfg.enabled;
}

/* Deterministic event queue functions */
void deterministic_queue_init(void);
void deterministic_queue_push(uint64_t when_instrs, void (*cb)(void *), void *opaque);
void deterministic_queue_run(uint64_t current_icount);
uint64_t deterministic_queue_get_current_icount(void);
void deterministic_queue_cleanup(void);

#endif /* QEMU_DETERMINISTIC_H */