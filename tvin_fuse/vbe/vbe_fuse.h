/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef VBI_FUSE_H
#define VBI_FUSE_H

#include <binder/Binder.h>
#include <binder/Parcel.h>
//#include "CTv.h"
#include <map>

using namespace android;

#ifdef __cplusplus
extern "C" {
#endif

/* **********************************
 * HDR info define
 * **********************************
 */
struct lcd_optical_info_s {
	unsigned int hdr_support;
	unsigned int features;
	unsigned int primaries_r_x;
	unsigned int primaries_r_y;
	unsigned int primaries_g_x;
	unsigned int primaries_g_y;
	unsigned int primaries_b_x;
	unsigned int primaries_b_y;
	unsigned int white_point_x;
	unsigned int white_point_y;
	unsigned int luma_max;
	unsigned int luma_min;
	unsigned int luma_avg;
};

/* **********************************
 * IOCTL define
 * **********************************
 */
#define LCD_IOC_TYPE               'C'
#define LCD_IOC_NR_GET_HDR_INFO    0x0
#define LCD_IOC_NR_SET_HDR_INFO    0x1

#define LCD_IOC_CMD_GET_HDR_INFO   \
	_IOR(LCD_IOC_TYPE, LCD_IOC_NR_GET_HDR_INFO, struct lcd_optical_info_s)
#define LCD_IOC_CMD_SET_HDR_INFO   \
	_IOW(LCD_IOC_TYPE, LCD_IOC_NR_SET_HDR_INFO, struct lcd_optical_info_s)

struct vbe_obj_s {
	const char		*obj_name;
	int 			vbe_drv_fd;
	unsigned char	open_cnt;
	int 			vbe_mute;
	int				vbe_mirror;
	int				vbe_mplus_boebypass;
	int				vbe_cpc;
	struct v4l2_ext_vbe_mplus_param mplus_param;
};

int vbe_drv_open(struct vbe_obj_s *pvbe_obj, int flags);
int vbe_drv_close(struct vbe_obj_s *pvbe_obj);

int vbe_drv_s_ext_vbe_mute(struct vbe_obj_s *pvbe_obj, int onoff);
int vbe_drv_s_ext_vbe_mirror(struct vbe_obj_s *pvbe_obj, int val);
int vbi_drv_s_ext_vbe_mplus_boebypass(struct vbe_obj_s *pvbe_obj, int val);

int vbi_drv_s_ext_vbe_cpc(struct vbe_obj_s *pvbe_obj, int val);
int vbi_drv_g_ext_vbe_gsr_status(struct vbe_obj_s *pvbe_obj);
int vbi_drv_g_ext_vbe_lock_status(struct vbe_obj_s *pvbe_obj);
int vbi_drv_g_ext_vbe_gsr_stress_count(struct vbe_obj_s *pvbe_obj);
int vbi_drv_g_ext_vbe_irr_result(struct vbe_obj_s *pvbe_obj);

int vbi_drv_s_ext_vbe_init(struct vbe_obj_s *pvbe_obj, struct v4l2_ext_vbe_panel_info *info);
int vbi_drv_s_ext_vbe_mplus_param(struct vbe_obj_s *pvbe_obj, struct v4l2_ext_vbe_mplus_param *param);
int vbi_drv_g_ext_vbe_mplus_param(struct vbe_obj_s *pvbe_obj, struct v4l2_ext_vbe_mplus_param *param);

#ifdef __cplusplus
}
#endif

#endif
