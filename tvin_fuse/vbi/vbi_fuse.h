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

struct vbi_obj_s {
	const char		*obj_name;
	int 			video_fd;
	int 			v4l2_sliced;
	struct v4l2_format fmt;
};

int vbi_drv_open(struct vbi_obj_s *pvbi_obj, int flags);
int vbi_drv_close(struct vbi_obj_s *pvbi_obj);
int vbi_drv_read(struct vbi_obj_s *pvbi_obj, size_t &size, off_t &off, char *&buf, size_t &out_size);
int vbi_drv_s_fmt(struct vbi_obj_s *pvbi_obj, struct v4l2_format *pfmt);
int vbi_drv_g_fmt(struct vbi_obj_s *pvbi_obj, struct v4l2_format *pfmt);
int vbi_drv_streamon(struct vbi_obj_s *pvbi_obj, int type);
int vbi_drv_streamoff(struct vbi_obj_s *pvbi_obj, int type);
int vbi_drv_s_ext_vbi_flush(struct vbi_obj_s *pvbi_obj, int val);
int vbi_drv_g_ext_vbi_copy_protection_info(struct vbi_obj_s *pvbi_obj, struct v4l2_ext_vbi_copy_protection *info);

#ifdef __cplusplus
}
#endif

#endif
