/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_MOUDLE_TAG "TVIN-VBE"
#define LOG_CLASS_TAG "DRV"

#include "../common.h"
#include "vbe_fuse.h"

#ifdef __cplusplus
extern "C" {
#endif

int vbe_drv_open(struct vbe_obj_s *pvbe_obj, int flags)
{
	int ret;

	LOGD("%s,%d\n",__func__, __LINE__);

	if (pvbe_obj->vbe_drv_fd == -1)
	{
		pvbe_obj->vbe_drv_fd = open(VBE_DRV_DEV_PATH, flags, 0);
		if (pvbe_obj->vbe_drv_fd < 0)
		{
			LOGD("Unable to open %s,err:%s\n", VBE_DRV_DEV_PATH,
				strerror(errno));
			return errno;
		}
	}
	pvbe_obj->open_cnt++;
	struct lcd_optical_info_s info;
	memset(&info, 0, sizeof(info));
	info.features = 12345678;
	ret = ioctl(pvbe_obj->vbe_drv_fd, LCD_IOC_NR_SET_HDR_INFO, &info);
	LOGD("%s,%d,video_fd:%d,open_cnt:%d,ret=%d\n",__func__, __LINE__,
		pvbe_obj->vbe_drv_fd, pvbe_obj->open_cnt, ret);

	return 0;
}

int vbe_drv_close(struct vbe_obj_s *pvbe_obj)
{
	LOGD("%s,%d\n",__func__, __LINE__, pvbe_obj->open_cnt);

	if (pvbe_obj->open_cnt == 0)
	{
		if (pvbe_obj->vbe_drv_fd > 0)
		{
			close(pvbe_obj->vbe_drv_fd);
			pvbe_obj->vbe_drv_fd = -1;
		}
		//more need to do
	}

	return 0;
}

/* V4L2_CID_EXT_VBE_MUTE */
int vbe_drv_s_ext_vbe_mute(struct vbe_obj_s *pvbe_obj, int onoff)
{
	int ret = 0;

	LOGD("%s,%d,onoff=%d\n",__func__, __LINE__, onoff);

	//TODO
	pvbe_obj->vbe_mute = !!onoff;

	return 0;
}

/* V4L2_CID_EXT_VBE_MIRROR */
int vbe_drv_s_ext_vbe_mirror(struct vbe_obj_s *pvbe_obj, int val)
{
	int ret = 0;

	LOGD("%s,%d,port=%d\n",__func__, __LINE__, val);

	//TODO
	pvbe_obj->vbe_mirror = !!val;

	return 0;
}

/* V4L2_CID_EXT_VBE_MPLUS_BOEBYPASS */
int vbi_drv_s_ext_vbe_mplus_boebypass(struct vbe_obj_s *pvbe_obj, int val)
{
	int ret = 0;

	LOGD("%s,%d,port=%d\n",__func__, __LINE__, val);

	//TODO
	pvbe_obj->vbe_mirror = !!val;

	return 0;
}

/* V4L2_CID_EXT_VBE_CPC */
int vbi_drv_s_ext_vbe_cpc(struct vbe_obj_s *pvbe_obj, int val)
{
	int ret = 0;

	LOGD("%s,%d,port=%d\n",__func__, __LINE__, val);

	//TODO
	pvbe_obj->vbe_cpc = !!val;

	return 0;
}

/* V4L2_CID_EXT_VBE_GSR_STATUS */
int vbi_drv_g_ext_vbe_gsr_status(struct vbe_obj_s *pvbe_obj)
{
	LOGD("%s,%d\n",__func__, __LINE__);

	return 1;//for test
}

/* V4L2_CID_EXT_VBE_LOCK_STATUS */
int vbi_drv_g_ext_vbe_lock_status(struct vbe_obj_s *pvbe_obj)
{
	LOGD("%s,%d\n",__func__, __LINE__);

	return 0;
}

/* V4L2_CID_EXT_VBE_GSR_STRESS_COUNT */
int vbi_drv_g_ext_vbe_gsr_stress_count(struct vbe_obj_s *pvbe_obj)
{
	LOGD("%s,%d\n",__func__, __LINE__);

	return 0;
}

/* V4L2_CID_EXT_VBE_IRR_RESULT */
int vbi_drv_g_ext_vbe_irr_result(struct vbe_obj_s *pvbe_obj)
{
	LOGD("%s,%d\n",__func__, __LINE__);

	return 0;
}

/* V4L2_CID_EXT_VBE_INIT */
int vbi_drv_s_ext_vbe_init(struct vbe_obj_s *pvbe_obj, struct v4l2_ext_vbe_panel_info *info)
{
	LOGD("%s,%d\n",__func__, __LINE__);

	return 0;
}

/* V4L2_CID_EXT_VBE_MPLUS_PARAM */
int vbi_drv_s_ext_vbe_mplus_param(struct vbe_obj_s *pvbe_obj, struct v4l2_ext_vbe_mplus_param *param)
{
	LOGD("%s,%d,nFrameGainLimit=%x,nPixelGainLimit=%x\n",
		__func__, __LINE__, param->nFrameGainLimit, param->nPixelGainLimit);
	pvbe_obj->mplus_param.nFrameGainLimit = param->nFrameGainLimit;
	pvbe_obj->mplus_param.nPixelGainLimit = param->nPixelGainLimit;

	return 0;
}

/* V4L2_CID_EXT_VBE_MPLUS_PARAM */
int vbi_drv_g_ext_vbe_mplus_param(struct vbe_obj_s *pvbe_obj, struct v4l2_ext_vbe_mplus_param *param)
{
	LOGD("%s,%d,nFrameGainLimit=%x,nPixelGainLimit=%x\n", __func__, __LINE__,
		pvbe_obj->mplus_param.nFrameGainLimit,pvbe_obj->mplus_param.nPixelGainLimit);
	param->nFrameGainLimit = pvbe_obj->mplus_param.nFrameGainLimit;
	param->nPixelGainLimit = pvbe_obj->mplus_param.nPixelGainLimit;

	return 0;
}

#ifdef __cplusplus
}
#endif
