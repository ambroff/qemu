/*
 * QEMU Deterministic Event Queue
 *
 * Copyright (c) 2025 Kyle Ambroff-Kao <kyle@ambroffkao.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#include "qemu/osdep.h"
#include "qemu/deterministic.h"
#include "qemu/queue.h"
#include "qemu/timer.h"
#include "qemu/main-loop.h"
#include "qemu/error-report.h"

typedef struct DeterministicEvent {
    uint64_t when_instrs;   /* When to execute (instruction count) */
    void (*callback)(void *);
    void *opaque;
    QTAILQ_ENTRY(DeterministicEvent) next;
} DeterministicEvent;

static QTAILQ_HEAD(, DeterministicEvent) event_queue = 
    QTAILQ_HEAD_INITIALIZER(event_queue);
static QemuMutex queue_lock;
static bool queue_initialized = false;
static uint64_t current_icount = 0;

void deterministic_queue_init(void)
{
    if (!queue_initialized) {
        qemu_mutex_init(&queue_lock);
        queue_initialized = true;
    }
}

void deterministic_queue_push(uint64_t when_instrs, void (*cb)(void *), void *opaque)
{
    DeterministicEvent *event, *e;

    if (!deterministic_enabled()) {
        /* If not in deterministic mode, execute immediately */
        cb(opaque);
        return;
    }

    if (!queue_initialized) {
        deterministic_queue_init();
    }

    event = g_new0(DeterministicEvent, 1);
    event->when_instrs = when_instrs;
    event->callback = cb;
    event->opaque = opaque;

    qemu_mutex_lock(&queue_lock);

    /* Insert in sorted order by instruction count */
    if (QTAILQ_EMPTY(&event_queue) || 
        when_instrs <= QTAILQ_FIRST(&event_queue)->when_instrs) {
        QTAILQ_INSERT_HEAD(&event_queue, event, next);
    } else {
        QTAILQ_FOREACH(e, &event_queue, next) {
            if (QTAILQ_NEXT(e, next) == NULL ||
                when_instrs <= QTAILQ_NEXT(e, next)->when_instrs) {
                QTAILQ_INSERT_AFTER(&event_queue, e, event, next);
                break;
            }
        }
    }

    qemu_mutex_unlock(&queue_lock);
}

void deterministic_queue_run(uint64_t icount)
{
    DeterministicEvent *event, *next;

    if (!deterministic_enabled() || !queue_initialized) {
        return;
    }

    current_icount = icount;

    qemu_mutex_lock(&queue_lock);

    QTAILQ_FOREACH_SAFE(event, &event_queue, next, next) {
        if (event->when_instrs > icount) {
            /* Events are sorted, so we can stop here */
            break;
        }

        /* Remove from queue */
        QTAILQ_REMOVE(&event_queue, event, next);

        /* Execute callback (unlock mutex first to avoid deadlock) */
        qemu_mutex_unlock(&queue_lock);
        event->callback(event->opaque);
        g_free(event);
        qemu_mutex_lock(&queue_lock);
    }

    qemu_mutex_unlock(&queue_lock);
}

uint64_t deterministic_queue_get_current_icount(void)
{
    return current_icount;
}

void deterministic_queue_cleanup(void)
{
    DeterministicEvent *event, *next;

    if (!queue_initialized) {
        return;
    }

    qemu_mutex_lock(&queue_lock);

    QTAILQ_FOREACH_SAFE(event, &event_queue, next, next) {
        QTAILQ_REMOVE(&event_queue, event, next);
        g_free(event);
    }

    qemu_mutex_unlock(&queue_lock);
    qemu_mutex_destroy(&queue_lock);
    queue_initialized = false;
}