/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_MOUDLE_TAG "TVIN-FUSE"
#define LOG_CLASS_TAG "AV_DRV"

#include "../common.h"
#include "av_fuse.h"

#ifdef __cplusplus
extern "C" {
#endif

int av_drv_open(struct av_obj_s *pav_obj, int flags)
{
	if (pav_obj->mp_VdinDrvIf == NULL)
		pav_obj->mp_VdinDrvIf = VdinDrvIf::GetInstance();

	if (pav_obj->av_drv_fd < 0)
	{
		pav_obj->av_drv_fd = open(AVIN_DETECT_PATH, flags, 0);
		if (pav_obj->av_drv_fd < 0)
		{
			LOGE("Unable to open %s,err:%s\n", AVIN_DETECT_PATH,
				strerror(errno));
			return errno;
		}
	}
	pav_obj->open_cnt++;
	LOGD("%s,%d,open_cnt:%d\n", __func__, __LINE__, pav_obj->open_cnt);

	return 0;
}

int av_drv_close(struct av_obj_s *pav_obj)
{
	LOGD("%s,%d,open_cnt:%d,src_input:%d\n", __func__, __LINE__,
		pav_obj->open_cnt, pav_obj->source_input);
	pav_obj->open_cnt--;
	if (pav_obj->open_cnt == 0)
	{
		pav_obj->source_input = SOURCE_INVALID;
		pav_obj->mp_VdinDrvIf->CloseInput();
		close(pav_obj->av_drv_fd);
		pav_obj->av_drv_fd = -1;
		//release other resources
	}
	return 0;
}

int av_drv_s_input(struct av_obj_s *pav_obj, enum v4l2_ext_avd_input_src input)
{
	tv_source_input_t source_input;

	if (input < 0 || input > V4L2_EXT_AVD_INPUT_SRC_AVD_RESERVED2)
	{
		LOGE("%s,invalid input src %d\n", __func__, input);
		return -EINVAL;
	}

	if (input == V4L2_EXT_AVD_INPUT_SRC_ATV)
		source_input = SOURCE_TV;
	else if (input == V4L2_EXT_AVD_INPUT_SRC_AV)
		source_input = SOURCE_AV1;
	else if (input == V4L2_EXT_AVD_INPUT_SRC_NONE)
		source_input = SOURCE_INVALID;
	else
		return -EINVAL;

	pav_obj->mp_VdinDrvIf->SetInput(source_input);

	pav_obj->avd_input = input;
	pav_obj->source_input = source_input;
	LOGD("%s,input=%d,source_input=%d\n",__func__, input, source_input);
	return 0;
}

int av_drv_g_input(struct av_obj_s *pav_obj)
{
	LOGD("%s,input=%d\n", __func__, pav_obj->avd_input);

	return pav_obj->avd_input;
}

int av_drv_s_std(struct av_obj_s *pav_obj, v4l2_std_id id)
{
	LOGD("%s,id=%lx\n",__func__, id);
	if (id == 0) {
		LOGE("%s,invalid port=%d\n", __func__, id);
		return -EINVAL;
	}

	pav_obj->id = id;

	return 0;
}

v4l2_std_id av_drv_g_std(struct av_obj_s *pav_obj)
{
	LOGD("%s,id=%lx\n",__func__, pav_obj->id);

	return pav_obj->id;
}

int av_drv_querycap(struct av_obj_s *pav_obj, struct v4l2_capability *cap)
{
	LOGD("%s,%d\n",__func__, __LINE__);

	memset(cap, 0, sizeof(struct v4l2_capability));

	strncpy((char *)&cap->driver[0], "other_dvb_eu", strlen("other_dvb_eu"));
	strncpy((char *)&cap->card[0], pav_obj->obj_name, strlen(pav_obj->obj_name));
	cap->version = 0x20220429;/* KERNEL_VERSION */
	cap->capabilities = (V4L2_CAP_ASYNCIO | V4L2_CAP_READWRITE);
	cap->device_caps = (V4L2_CAP_ASYNCIO | V4L2_CAP_READWRITE);

	return 0;
}

/* V4L2_CID_EXT_AVD_TIMING_INFO */
int av_drv_g_ext_avd_timing_info(struct av_obj_s *pav_obj, struct v4l2_ext_avd_timing_info *info)
{
	int ret;
	struct tvin_info_s SignalInfo;
	struct tvin_format_s Format;

	memset(&SignalInfo, 0, sizeof(SignalInfo));
	memset(&Format, 0, sizeof(Format));
	memset(info, 0, sizeof(struct v4l2_ext_avd_timing_info));

	ret = pav_obj->mp_VdinDrvIf->DeviceIOCtl(TVIN_IOC_G_SIG_INFO, &SignalInfo);
	if (ret < 0)
		LOGE("TVIN_IOC_G_SIG_INFO failed.%s,%d\n",__func__, __LINE__);

	LOGD("trans_fmt:%d,fmt:%#x,status:%d,cfmt:%d,fps:%d,is_dvi:%d,hdr_info:%x,ratio:%d\n",
		SignalInfo.trans_fmt, SignalInfo.fmt, SignalInfo.status,
		SignalInfo.cfmt, SignalInfo.fps, SignalInfo.is_dvi,
		SignalInfo.hdr_info, SignalInfo.aspect_ratio);

	ret = pav_obj->mp_VdinDrvIf->DeviceIOCtl(TVIN_IOC_G_INPUT_TIMING, &Format);
	if (ret < 0)
		LOGE("TVIN_IOC_G_INPUT_TIMING failed.%s,%d\n",__func__, __LINE__);

	LOGD("active:%dx%d,h_cnt:%d-%d-%d,hs_cnt:%d-%d,total:%dx%d,hs:%d %d %d,vs:%d %d %d\n",
		Format.h_active, Format.v_active, Format.h_cnt, Format.h_cnt_offset, Format.v_cnt_offset,
		Format.hs_cnt, Format.hs_cnt_offset,Format.h_total, Format.v_total,Format.hs_front,
		Format.hs_width,Format.hs_bp,Format.vs_front,Format.vs_width, Format.vs_bp);
	LOGD("pol:%d %d,scan:%d,clk:%d,vbi:%d %d,duration:%d\n",
		Format.hs_pol, Format.vs_pol, Format.scan_mode, Format.pixel_clk,
		Format.vbi_line_start, Format.vbi_line_end, Format.duration);

	//convert and fill in v4l2_ext_avd_timing_info.
	if (Format.h_total)
		info->h_freq = (Format.pixel_clk * 10 * 1000) / Format.h_total;//unit Hz
	info->v_freq = SignalInfo.fps;
	info->h_porch = Format.hs_front;
	info->v_porch = Format.vs_front;
	info->active.x = 0;
	info->active.y = 0;
	info->active.w = Format.h_active;
	info->active.h = Format.v_active;
	if (SignalInfo.status == TVIN_SIG_STATUS_STABLE) {
		info->vd_lock = 1;
		info->h_lock = 1;
		info->v_lock = 1;
	} else {
		info->vd_lock = 0;
		info->h_lock = 0;
		info->v_lock = 0;
	}

	return 0;
}

/* V4L2_CID_EXT_AVD_PORT */
int av_drv_g_ext_avd_port(struct av_obj_s *pav_obj)
{
	LOGD("%s,%d;avd_port:%d\n",__func__, __LINE__, pav_obj->avd_port);

	return pav_obj->avd_port;
}

/* V4L2_CID_EXT_AVD_PORT */
int av_drv_s_ext_avd_port(struct av_obj_s *pav_obj, int port)
{
	int ret = 0;

	LOGD("%s,%d,port=%d\n",__func__, __LINE__, port);
	if (port < 0 || port > 255) {
		LOGD("%s invalid port=%d\n", __func__, port);
		return -EINVAL;
	}
	//TODO
	pav_obj->avd_port = port;

	return 0;
}

/* V4L2_CID_EXT_AVD_AUTO_TUNING_MODE */
int av_drv_s_ext_avd_auto_tuning_mode(struct av_obj_s *pav_obj, int mode)
{
	LOGD("%s,%d,mode=%d\n",__func__, __LINE__, mode);
	if (mode < 0) {
		LOGD("%s,invalid mode=%d\n",__func__, mode);
		return -EINVAL;
	}

	pav_obj->avd_auto_tuning_mode = !!mode;
	return 0;
}

/* V4L2_CID_EXT_AVD_AUTO_TUNING_MODE */
//TODO:ATV function
int av_drv_g_ext_avd_auto_tuning_mode(struct av_obj_s *pav_obj)
{
	LOGD("%s,%d\n",__func__, __LINE__);

	return pav_obj->avd_auto_tuning_mode;
}

/* V4L2_CID_EXT_AVD_CHANNEL_CHANGE */
//This function is not necessary
int av_drv_s_ext_avd_channel_change(struct av_obj_s *pav_obj, int val)
{
	LOGD("%s,%d,val=%d\n",__func__, __LINE__, val);

	pav_obj->avd_auto_tuning_mode = val;
	return 0;
}

/* V4L2_CID_EXT_AVD_VIDEO_SYNC */
//0 (AVD Sync Not Locked), 1 (AVD Sync Locked)
int av_drv_g_ext_avd_video_sync(struct av_obj_s *pav_obj)
{
	int ret;
	struct tvin_info_s SignalInfo;

	memset(&SignalInfo, 0, sizeof(SignalInfo));
	ret = pav_obj->mp_VdinDrvIf->DeviceIOCtl(TVIN_IOC_G_SIG_INFO, &SignalInfo);
	if (ret < 0)
	{
		LOGE("TVIN_IOC_G_SIG_INFO failed.%s,%d\n",__func__, __LINE__);
		return 0;
	}
	LOGD("%s,status=%d\n",__func__, SignalInfo.status);

	if (SignalInfo.status == TVIN_SIG_STATUS_STABLE)
		return 1;
	else
		return 0;
}

#ifdef __cplusplus
}
#endif
