/*
 * Copyright (C) ST-Ericsson SA 2012. All rights reserved.
 * This code is ST-Ericsson proprietary and confidential.
 * Any use of the code for whatever purpose is subject to
 * specific written permission of ST-Ericsson SA.
 *
 * Author: Julius Gustavson <julius.gustavsson@stericsson.com> for ST-Ericsson.
 */
#ifndef VSYNC_MONITOR_H
#define VSYNC_MONITOR_H (1)

#include "hardware/hwcomposer.h"
//#include "hwc_dev.h"

typedef struct vsync_monitor_context vsync_monitor_context_t;

//int vsync_monitor_init(int compdev_fd);
void vsync_monitor_init(vsync_monitor_context_t *hwc_dev);

void vsync_monitor_enable();
//int vsync_monitor_enable(struct hwc_procs* procs);

void vsync_monitor_disable();

void vsync_monitor_destroy();

#endif        //  #ifndef VSYNC_MONITOR_H

