/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef TVIN_FUSE_COMMON_H
#define TVIN_FUSE_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <linux/kdev_t.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <string>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define FUSE_USE_VERSION 31
#include <cuse_lowlevel.h>
#include <fuse_opt.h>

#include <linux/videodev2.h>
//#include <linux/v4l2-ext/videodev2-ext.h>
//#include <linux/v4l2-ext/v4l2-controls-ext.h>
//#include <linux/linuxtv-ext-ver.h>
#include "v4l2-ext/videodev2-ext.h"
#include "v4l2-ext/v4l2-controls-ext.h"
#include "linuxtv-ext-ver.h"

#include "CTvin.h"
#include "CTvLog.h"
#include <TvClient.h>
#include <TvClientWrapper.h>

#define AVIN_DETECT_PATH    "/dev/avin_detect"
#define HDMI_DETECT_PATH    "/dev/hdmirx0"
#define VBE_DRV_DEV_PATH    "/dev/lcd0"

class VdinDrvIf
{
public:
	VdinDrvIf();
	~VdinDrvIf();
	static VdinDrvIf *GetInstance();

	int OpenDev();
	int CloseDev();
	int OpenFePort(tvin_port_t port);
	int CloseFePort(tvin_port_t port);
	int SetInput(tv_source_input_t port);
	int CloseInput(void);
	int GetInput(tvin_port_t port);
	int DeviceIOCtl(int request, ...);
private:
	int32_t mVdin0DevFd = 0;
	static VdinDrvIf *mInstance;
	struct TvClientWrapper_t *pTvClientWrapper;
	tv_source_input_t cfg_port;
	tv_source_input_t active_port;
};

struct cuse_obj {
	unsigned		major;
	unsigned		minor;
	const char		*dev_name;
	int				is_help;
	pthread_t		tid;
	struct cuse_info ci;
	struct fuse_args args;
	struct cuse_lowlevel_ops *p_cuse_ops;
	void *pri;/* device object */
};

struct cusexmp_param {
	unsigned		major;
	unsigned		minor;
	char			*dev_name;
	int			is_help;
};

//using namespace android;

#endif

