/*
 * QEMU Deterministic Execution Support
 *
 * Copyright (c) 2025 Kyle Ambroff-Kao <kyle@ambroffkao.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#include "qemu/osdep.h"
#include "qemu/deterministic.h"

/* Global deterministic configuration */
DeterministicConfig deterministic_cfg = {
    .enabled = false,
    .random_seed = 0,
    .start_time_ns = 0,
    .instr_slice = 10000,  /* Default instruction slice */
    .force_icount = false,
    .disable_mttcg = false
};

void deterministic_init_config(void)
{
    /* Initialize with defaults if needed */
    if (deterministic_cfg.instr_slice == 0) {
        deterministic_cfg.instr_slice = 10000;
    }
}