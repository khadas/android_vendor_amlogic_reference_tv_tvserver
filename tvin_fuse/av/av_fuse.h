/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef AV_FUSE_H
#define AV_FUSE_H

#include <binder/Binder.h>
#include <binder/Parcel.h>
#include <map>
#include "../common.h"

using namespace android;

#define AVIN_DETECT_PATH	"/dev/avin_detect"

#ifdef __cplusplus
extern "C" {
#endif

struct av_obj_s {
	const char		*obj_name;
	int 			av_drv_fd;
	unsigned char 	open_cnt;
	int avd_port;
	int avd_auto_tuning_mode;
	int avd_channel_change;
	enum v4l2_ext_avd_input_src avd_input;
	tv_source_input_t source_input;
	v4l2_std_id id;
	VdinDrvIf *mp_VdinDrvIf;
};

int av_drv_open(struct av_obj_s *pav_obj, int flags);
int av_drv_close(struct av_obj_s *pav_obj);
int av_drv_s_input(struct av_obj_s *pav_obj, enum v4l2_ext_avd_input_src input);
int av_drv_g_input(struct av_obj_s *pav_obj);
int av_drv_s_std(struct av_obj_s *pav_obj, v4l2_std_id id);
int av_drv_querycap(struct av_obj_s *pav_obj, struct v4l2_capability *cap);
int av_drv_g_ext_avd_timing_info(struct av_obj_s *pav_obj, struct v4l2_ext_avd_timing_info *info);
int av_drv_g_ext_avd_port(struct av_obj_s *pav_obj);
int av_drv_s_ext_avd_port(struct av_obj_s *pav_obj, int port);
int av_drv_s_ext_avd_auto_tuning_mode(struct av_obj_s *pav_obj, int mode);
int av_drv_g_ext_avd_auto_tuning_mode(struct av_obj_s *pav_obj);
int av_drv_s_ext_avd_channel_change(struct av_obj_s *pav_obj, int val);
int av_drv_g_ext_avd_video_sync(struct av_obj_s *pav_obj);

v4l2_std_id av_drv_g_std(struct av_obj_s *pav_obj);

#ifdef __cplusplus
}
#endif

#endif
