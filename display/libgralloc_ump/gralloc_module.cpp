/*
 * Copyright (C) 2010 ARM Limited. All rights reserved.
 *
 * Portions of this code have been modified from the original.
 * These modifications are:
 *    * includes
 *    * enums
 *    * gralloc_device_open()
 *    * gralloc_register_buffer()
 *    * gralloc_unregister_buffer()
 *    * gralloc_lock()
 *    * gralloc_unlock()
 *    * gralloc_module_methods
 *    * HAL_MODULE_INFO_SYM
 *
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <hardware/hardware.h>
#include <hardware/gralloc.h>
#include <fcntl.h>

#include <gralloc1-adapter.h>

#include "gralloc_priv.h"
//#include "alloc_device.h"
//#include "framebuffer_device.h"

//#include "ump.h"
//#include "ump_ref_drv.h"
//#include "secion.h"
//#include "s5p_fimc.h"
//#include "exynos_mem.h"

static int gralloc_map(gralloc_module_t const* module,
        buffer_handle_t handle, void** vaddr)
{
    return 0;
}

static int gralloc_unmap(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    return 0;
}

static int gralloc_device_open(const hw_module_t* module, const char* name, hw_device_t** device)
{
    int status = -EINVAL;

#ifdef ADVERTISE_GRALLOC1
    if (!strcmp(name, GRALLOC_HARDWARE_MODULE_ID)) {
        return gralloc1_adapter_device_open(module, name, device);
    }
#endif

    if (!strcmp(name, GRALLOC_HARDWARE_GPU0))
        status = -1; //alloc_device_open(module, name, device);
    else if (!strcmp(name, GRALLOC_HARDWARE_FB0))
        status = -1; //framebuffer_device_open(module, name, device);

    return status;
}

static int gralloc_register_buffer(gralloc_module_t const* module, buffer_handle_t handle)
{
    return 0;
}

static int gralloc_unregister_buffer(gralloc_module_t const* module, buffer_handle_t handle)
{
    return 0;
}

static int gralloc_lock(gralloc_module_t const* module, buffer_handle_t handle,
                        int usage, int l, int t, int w, int h, void** vaddr)
{
    return 0;
}

static int gralloc_unlock(gralloc_module_t const* module, buffer_handle_t handle)
{
    return 0;
}

static int gralloc_perform(struct gralloc_module_t const* module,
                    int operation, ... )
{
    int res = -EINVAL;
    return res;
}

static int gralloc_getphys(gralloc_module_t const* module, buffer_handle_t handle, void** paddr)
{
    return 0;
}

/* There is one global instance of the module */
static struct hw_module_methods_t gralloc_module_methods =
{
    .open = gralloc_device_open
};

struct private_module_t HAL_MODULE_INFO_SYM =
{
    .base =
    {
        .common =
        {
            .tag = HARDWARE_MODULE_TAG,
#ifdef ADVERTISE_GRALLOC1
            .version_major = GRALLOC1_ADAPTER_MODULE_API_VERSION_1_0,
#else
            .version_major = 1,
#endif
            .version_minor = 0,
            .id = GRALLOC_HARDWARE_MODULE_ID,
            .name = "Graphics Memory Allocator Module",
            .author = "ARM Ltd.",
            .methods = &gralloc_module_methods,
            .dso = NULL,
        },
        .registerBuffer = gralloc_register_buffer,
        .unregisterBuffer = gralloc_unregister_buffer,
        .lock = gralloc_lock,
        .unlock = gralloc_unlock,
//      .getphys = gralloc_getphys,
        .perform = gralloc_perform,
    },
    .framebuffer = NULL,
    .flags = 0,
    .numBuffers = 0,
    .bufferMask = 0,
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .currentBuffer = NULL,
};
