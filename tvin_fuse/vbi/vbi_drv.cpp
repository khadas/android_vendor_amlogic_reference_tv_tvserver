/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_MOUDLE_TAG "TVIN-CVBS"
#define LOG_CLASS_TAG "TVIN-CUSE"

#include "../common.h"
#include "vbi_fuse.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VBI_FAKE_DATA_ARRY_NUM	36
v4l2_sliced_vbi_data vbi_fake_data[VBI_FAKE_DATA_ARRY_NUM] = {};

int vbi_drv_open(struct vbi_obj_s *pvbi_obj, int flags)
{
	printf("1,%s,%d\n",__func__, __LINE__);
#if 0
	if (pvbi_obj->video_fd == -1)
	{
		pvbi_obj->video_fd = open(AVIN_DETECT_PATH, flags, 0);
		if (pvbi_obj->video_fd < 0)
		{
			printf("Unable to open %s,err:%s\n", AVIN_DETECT_PATH,
				strerror(errno));
			return errno;
		}
	}
#endif
//	printf("%s,%d,video_fd:%d\n",__func__, __LINE__, pvbi_obj->video_fd);

	return 0;
}

int vbi_drv_close(struct vbi_obj_s *pvbi_obj)
{
	printf("%s,%d\n",__func__, __LINE__);
	if (pvbi_obj->video_fd > 0)
	{
		close(pvbi_obj->video_fd);
		pvbi_obj->video_fd = -1;
	}

	return 0;
}

int vbi_drv_read(struct vbi_obj_s *pvbi_obj, size_t &size, off_t &off, char *&buf, size_t &out_size)
{
	printf("%s,%d\n",__func__, __LINE__);
	int i;
	//temporary data for SoCTS vbi test
	for (i = 0;i < VBI_FAKE_DATA_ARRY_NUM;i++) {
		vbi_fake_data[i].id = V4L2_SLICED_TELETEXT_B;
	}
	buf = (char *)&vbi_fake_data[0];
	out_size = VBI_FAKE_DATA_ARRY_NUM * sizeof(v4l2_sliced_vbi_data);
	printf("%s,%d,out_size:%lx\n",__func__, __LINE__, out_size);

	return 0;
}

int vbi_drv_s_fmt(struct vbi_obj_s *pvbi_obj, struct v4l2_format *pfmt)
{
	printf("%s,%d,service_set:%x\n",__func__, __LINE__,
		pfmt->fmt.sliced.service_set);
	if (pfmt->type >= 0xff)
		return -EINVAL;
	memcpy(&pvbi_obj->fmt, pfmt, sizeof(struct v4l2_format));
	return 0;
}

int vbi_drv_g_fmt(struct vbi_obj_s *pvbi_obj, struct v4l2_format *pfmt)
{
	printf("%s,%d, type:%x\n",__func__, __LINE__, pfmt->type);
	if (pfmt->type >= 0xff)
		return -EINVAL;

	memcpy(pfmt, &pvbi_obj->fmt, sizeof(struct v4l2_format));
	return 0;
}

int vbi_drv_streamon(struct vbi_obj_s *pvbi_obj, int type)
{
	printf("%s,%d,type=%lx\n",__func__, __LINE__, type);
	if (type < 0)
		return -1;

	return 0;
}

int vbi_drv_streamoff(struct vbi_obj_s *pvbi_obj, int type)
{
	printf("%s,%d,type=%lx\n",__func__, __LINE__, type);
	if (type < 0)
		return -1;

	return 0;
}

/* V4L2_CID_EXT_VBI_COPY_PROTECTION_INFO */
int vbi_drv_g_ext_vbi_copy_protection_info(struct vbi_obj_s *pvbi_obj, struct v4l2_ext_vbi_copy_protection *info)
{
	printf("%s,%d\n",__func__, __LINE__);

	info->aps_cp_info  = V4L2_EXT_VBI_APS_OFF;
	info->cgms_cp_info = V4L2_EXT_VBI_CGMS_PERMIT;
	info->macrovision_cp_info = V4L2_EXT_VBI_MACROVISION_PSP_OFF;

	return 0;
}

/* V4L2_CID_EXT_VBI_FLUSH */
int vbi_drv_s_ext_vbi_flush(struct vbi_obj_s *pvbi_obj, int val)
{
	int ret = 0;

	printf("%s,%d,port=%d\n",__func__, __LINE__, val);
	if (val > V4L2_SLICED_VBI_625)
		return -EINVAL;
	//TODO
	pvbi_obj->v4l2_sliced = val;

	return 0;
}

#ifdef __cplusplus
}
#endif
