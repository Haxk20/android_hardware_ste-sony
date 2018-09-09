/*
 * Copyright (C) ST-Ericsson SA 2012. All rights reserved.
 * This code is ST-Ericsson proprietary and confidential.
 * Any use of the code for whatever purpose is subject to
 * specific written permission of ST-Ericsson SA.
 *
 * Author: Julius Gustavson <julius.gustavsson@stericsson.com> for ST-Ericsson.
 */

#ifndef __HWC_DEV__
#define __HWC_DEV__

#define LOG_TAG "STE-HWComposer-VSyncMonitor"
#define DEBUG_STE_HWCOMPOSER 0
/* Uncomment to enable verbose prints */
// #define LOG_NDEBUG 0


#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <linux/compdev.h>
#include <cutils/log.h>

#include "vsync_monitor.h"

// Thread instance data
struct vsync_monitor_context {
    pthread_t               thread;
    pthread_mutex_t         mutex;
    pthread_cond_t          cond_var;
    bool                    stop;
    bool                    enable;
    struct hwc_procs*       procs;
    int                     compdev_fd;
#ifdef FAKE_VSYNC
    struct timespec         next_vsync;
#endif
};

typedef struct vsync_monitor_context vsync_monitor_context_t;

#endif /* __HWC_DEV__ */
