#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <log/log.h>
#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>
#include <linux/fb.h>
#include <sys/ioctl.h>



#include <utils/String8.h>

//#include "hwcomposer.h"
#include <cutils/log.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <hardware/gralloc.h>
//#include <gralloc_stericsson_ext.h>
#include <linux/hwmem.h>
#include <linux/compdev.h>
#include <limits.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
//#include "vsync_monitor.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "STE-HWComposer"


const size_t BURSTLEN_BYTES = 16 * 8;
const size_t MAX_PIXELS = 12 * 1024 * 1000;



/* Use the STE version if available, otherwise fall back to
 * the corresponding vanilla version.
 * Note: the STE version is normally only unavailable
 * during the bring-up period of an Android upgrade.
 */
#ifdef HWC_DEVICE_API_VERSION_0_3_STE
#define STE_HWC_DEVICE_API_CURRENT HWC_DEVICE_API_VERSION_0_3_STE
#else
#define STE_HWC_DEVICE_API_CURRENT HWC_DEVICE_API_VERSION_0_3
#endif

#define DEBUG_STE_HWCOMPOSER 0
#define DEBUG_STE_HWCOMPOSER_LAYER_DUMP 0
#define DEBUG_STE_HWCOMPOSER_ALLOC 0

#define HWMEM_PATH ("/dev/" HWMEM_DEFAULT_DEVICE_NAME)
#define COMPDEV_PATH "/dev/comp0"

#ifdef ENABLE_HDMI
#define HDMID_SOCKET_LISTEN_PATH "/dev/socket/hdmid"
#endif

/*
 * Only 2 buffers are needed, one for blitting and one for display HW.
 * If the display posting is asynchronous, 3 buffers are needed.
 */
#define OUT_IMG_COUNT 2

/* Max number of layers that can be cached. */
#define CACHED_LAYERS_SIZE 16


#ifdef ENABLE_HDMI
typedef struct hwc_hdmi_setting {
    bool                 hdmi_plugged;
    bool                 resolutionchanged;
    int                  hdmid_sockfd;
    struct compdev_rect  res;
} hwc_hdmi_settings_t;
#endif

struct worker_context {
    pthread_mutex_t mutex;
    pthread_t worker_thread;
    pthread_cond_t signal_update;
    pthread_cond_t signal_done;
    pthread_attr_t thread_attr;
    bool alive;
    bool working;
    int hwmem;
    int compdev;
    const struct gralloc_module_t *gralloc;
    hwc_display_contents_1_t* work_list;
    struct hwc_rect* frame_rect;
    uint32_t compdev_layer_count;
    uint32_t compdev_bypass_count;
    int disp_width;
    int disp_height;
    uint32_t ui_orientation;
    uint32_t hardware_rotation;
};

struct hwcomposer_context {
    struct hwc_composer_device_1 dev;
    pthread_mutex_t hwc_mutex;
    int hwmem;
    int compdev;
    const struct gralloc_module_t *gralloc;
    struct worker_context worker_ctx;
    int disp_width;
    int disp_height;
    bool bypass_ovly;
    uint32_t ui_orientation;
    uint32_t hardware_rotation;
    bool videoplayback;
#if DEBUG_STE_HWCOMPOSER
    int frames;
    int overlay_layers;
    int framebuffer_layers;
#endif
#ifdef ENABLE_HDMI
    hwc_hdmi_settings_t hdmi_settings;
#endif
    bool pending_rotation;
    buffer_handle_t *cached_layers;
    size_t cached_layers_count;
    bool grabbed_all_layers;
    int32_t *actual_composition_types;
    uint32_t actual_composition_types_size;
    hwc_procs_t const *procs;
    int fb;
    int32_t vsync_period;
    int32_t xres;
    int32_t yres;
    int32_t xdpi;
    int32_t ydpi;
};

static int hwc_prepare(struct hwc_composer_device_1 *dev,
		size_t numDisplays, hwc_display_contents_1_t **displays)
{
	for (size_t i = 0; i < displays[0]->numHwLayers; i++) {
		displays[0]->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
	}

	return 0;
}

static int hwc_set(struct hwc_composer_device_1 *dev,
		size_t numDisplays, hwc_display_contents_1_t **displays)
{

	ALOGI("I should hwc_set but i'm not");
	return 0;
}

static int hwc_setPowerMode(struct hwc_composer_device_1 *dev, int disp,
		int mode)
{
	ALOGI("%s: %d -> %d", __func__, disp, mode);
	return 0;
}

static int hwc_query(struct hwc_composer_device_1 *dev, int what, int *value)
{
	switch (what) {
		case HWC_VSYNC_PERIOD:
			return -EINVAL;
		case HWC_BACKGROUND_LAYER_SUPPORTED:
			*value = 0;
			break;
		case HWC_DISPLAY_TYPES_SUPPORTED:
			*value = HWC_DISPLAY_PRIMARY_BIT;
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static void hwc_registerProcs(struct hwc_composer_device_1 *dev,
		hwc_procs_t const *procs)
{
	struct hwcomposer_context *ctx = (struct hwcomposer_context *)dev;
	ctx->procs = procs;
}

static int hwc_eventControl(struct hwc_composer_device_1 *dev,
		int disp, int event, int enabled)
{
	return 0;
}

static int hwc_getDisplayConfigs(struct hwc_composer_device_1 *dev,
		int disp, uint32_t *configs, size_t *numConfigs)
{
	if (*numConfigs < 1)
		return 0;

	if (disp == HWC_DISPLAY_PRIMARY) {
		configs[0] = 0;
		*numConfigs = 1;
		return 0;
	}

	return -EINVAL;
}

static int hwc_getDisplayAttributes(struct hwc_composer_device_1 *dev,
		int disp, uint32_t config, const uint32_t *attributes, int32_t *values)
{
	struct hwcomposer_context *ctx = (struct hwcomposer_context *)dev;
	int i = 0;
	if (disp != 0)
		return -EINVAL;

	while (attributes[i] != HWC_DISPLAY_NO_ATTRIBUTE) {
		switch (attributes[i]) {
		case HWC_DISPLAY_VSYNC_PERIOD:
			values[i] = ctx->vsync_period;
			break;
		case HWC_DISPLAY_WIDTH:
			values[i] = ctx->xres;
			break;
		case HWC_DISPLAY_HEIGHT:
			values[i] = ctx->yres;
			break;
		case HWC_DISPLAY_DPI_X:
			values[i] = ctx->xdpi;
			break;
		case HWC_DISPLAY_DPI_Y:
			values[i] = ctx->ydpi;
			break;
		default:
			return -EINVAL;
		}
		i++;
	}
	return 0;
}

static int hwc_device_close(struct hw_device_t *dev)
{
	struct hwcomposer_context *hwc = (struct hwcomposer_context *)dev;
	free(hwc);
	return 0;
}

static int hwc_open(const struct hw_module_t *module, const char *name,
		struct hw_device_t **device)
{
	struct hwcomposer_context *dev;
	struct fb_var_screeninfo lcdinfo;
        struct compdev_size display_size;

	int refreshRate;

	dev = (struct hwcomposer_context *)malloc(sizeof(*dev));
	if (!dev)
		return -ENOMEM;

	memset(dev, 0, sizeof(*dev));

        pthread_mutex_init(&dev->hwc_mutex, NULL);
        pthread_mutex_lock(&dev->hwc_mutex);

	dev->dev.common.tag = HARDWARE_DEVICE_TAG;
	dev->dev.common.version = HWC_DEVICE_API_VERSION_1_0;
	dev->dev.common.module = const_cast<hw_module_t*>(module);
	dev->dev.common.close = hwc_device_close;

	dev->dev.prepare = hwc_prepare;
	dev->dev.set = hwc_set;
	dev->dev.eventControl = hwc_eventControl;
	dev->dev.setPowerMode = hwc_setPowerMode;
	dev->dev.query = hwc_query;
	dev->dev.registerProcs = hwc_registerProcs;
	dev->dev.getDisplayConfigs = hwc_getDisplayConfigs;
	dev->dev.getDisplayAttributes = hwc_getDisplayAttributes;

	*device = &dev->dev.common;

	dev->hwmem = open(HWMEM_PATH, O_RDWR);
        if (dev->hwmem < 0) {
            ALOGE("%s: could not open %s (%s)", __func__, HWMEM_PATH, strerror(errno));
            goto error;
        }

        dev->compdev = open(COMPDEV_PATH, O_RDWR, 0);
        if (dev->compdev < 0) {
            ALOGE("%s: Error Opening " COMPDEV_PATH ": %s\n", __func__,
                    strerror(errno));
            goto compdev_error;

        }

        if (ioctl(dev->compdev, COMPDEV_GET_SIZE_IOC, (struct compdev_size*)&display_size)) {
            ALOGE("%s: Failed to get size from compdev, %s", __func__, strerror(errno));
            goto error;
        }

        dev->disp_width = display_size.width;
        dev->disp_height = display_size.height;

        if (hw_get_module(GRALLOC_HARDWARE_MODULE_ID,
            (const struct hw_module_t **)&dev->gralloc)) {
            ALOGE("Error opening gralloc module");
            goto gralloc_error;
        }


	dev->fb = open("/dev/graphics/fb0", O_RDWR);
	if (dev->fb < 0) {
		ALOGE("%s: failed to open FB", __func__);
		return -EINVAL;
	}

	if (ioctl(dev->fb, FBIOGET_VSCREENINFO, &lcdinfo) < 0) {
		ALOGE("%s: failed to get vscreeninfo", __func__);
		return -EINVAL;
	}


	ALOGI("%s: %d %d %d %d %d %d %d", __func__,lcdinfo.width, lcdinfo.height, lcdinfo.yres,
			lcdinfo.left_margin ,lcdinfo.right_margin, lcdinfo.xres, lcdinfo.pixclock);
/*	int refreshRate = 1000000000000LLU /
		(
		 uint64_t( lcdinfo.upper_margin + lcdinfo.lower_margin + lcdinfo.yres)
		 * ( lcdinfo.left_margin  + lcdinfo.right_margin + lcdinfo.xres)
		 * lcdinfo.pixclock
		);
*/
	refreshRate = 60;

	dev->vsync_period = 1000000000UL / refreshRate;
	dev->xres = lcdinfo.xres;
	dev->yres = lcdinfo.yres;
	dev->xdpi = (lcdinfo.xres * 25.4f * 1000.0f) / lcdinfo.width;
	dev->ydpi = (lcdinfo.yres * 25.4f * 1000.0f) / lcdinfo.height;

	pthread_mutex_unlock(&dev->hwc_mutex);

	return 0;
worker_error:
gralloc_error:
	close(dev->compdev);
compdev_error:
	close(dev->hwmem);
error:
	if (dev)
		free(dev);
	pthread_mutex_unlock(&dev->hwc_mutex);

	return -EINVAL;
}

static struct hw_module_methods_t hwc_module_methods = {
	.open = hwc_open,
};

hwc_module_t HAL_MODULE_INFO_SYM = {
	.common = {
		.tag = HARDWARE_MODULE_TAG,
		.version_major = 1,
		.version_minor = 0,
		.id = HWC_HARDWARE_MODULE_ID,
		.name = "Dummy hwcomposer",
		.author = "Simon Shields <simon@lineageos.org>",
		.methods = &hwc_module_methods,
	},
};
