/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_MOUDLE_TAG "TVIN-HDMIRX"
#define LOG_CLASS_TAG "DRV"

#include "../common.h"
#include "hdmirx_fuse.h"
#include "CHDMIRxManager.h"

#ifdef __cplusplus
extern "C" {
#endif

int hdmirx_drv_open(struct hdmirx_obj_s *phdmirx_obj, int flags)
{
	if (phdmirx_obj->mp_VdinDrvIf == NULL)
		phdmirx_obj->mp_VdinDrvIf = VdinDrvIf::GetInstance();

	if (phdmirx_obj->hdmirx0_drv_fd == -1)
	{
		LOGD("%s,%d\n",__func__, __LINE__);
		phdmirx_obj->hdmirx0_drv_fd = open(HDMI_DETECT_PATH, flags, 0);
		if (phdmirx_obj->hdmirx0_drv_fd < 0)
		{
			LOGE("Unable to open %s,err:%s\n", HDMI_DETECT_PATH,
				strerror(errno));
			return errno;
		}
		phdmirx_obj->edid.pData 	= new unsigned char[0x200];
		phdmirx_obj->hdcp_key.pData = new unsigned char[HDCP14_KEY_SIZE];
	}
	phdmirx_obj->open_cnt++;
	//ioctl test.
	int ret;
	enum hdcp14_key_mode_e mode = NORMAL_MODE;
	ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_HDCP14_KEY_MODE, &mode);
	//ioctl test end.

	LOGD("%s,%d,open_cnt=%d,ret=%d\n",__func__, __LINE__, phdmirx_obj->open_cnt, ret);

	return 0;
}

int hdmirx_drv_close(struct hdmirx_obj_s *phdmirx_obj)
{
	LOGD("%s,%d,open_cnt:%d\n",__func__, __LINE__, phdmirx_obj->open_cnt);
	phdmirx_obj->open_cnt--;
	if (phdmirx_obj->open_cnt == 0)
	{
		if (phdmirx_obj->hdmirx0_drv_fd > 0)
		{
			phdmirx_obj->mp_VdinDrvIf->CloseInput();
			close(phdmirx_obj->hdmirx0_drv_fd);
			phdmirx_obj->hdmirx0_drv_fd = -1;
		}

	}

	return 0;
}

int hdmirx_drv_s_input(struct hdmirx_obj_s *phdmirx_obj, int hwport)
{
	tv_source_input_t source_input;

	LOGD("%s,%d,hwport=%d\n",__func__, __LINE__, hwport);

	if (hwport <= V4L2_EXT_HDMI_INPUT_PORT_NONE || hwport > V4L2_EXT_HDMI_INPUT_PORT_3)
		return -EINVAL;

	if (hwport == V4L2_EXT_HDMI_INPUT_PORT_1)
		source_input = SOURCE_HDMI1;
	else if (hwport == V4L2_EXT_HDMI_INPUT_PORT_2)
		source_input = SOURCE_HDMI2;
	else if (hwport == V4L2_EXT_HDMI_INPUT_PORT_3)
		source_input = SOURCE_HDMI3;
	else
		return -EINVAL;

	phdmirx_obj->mp_VdinDrvIf->SetInput(source_input);

	phdmirx_obj->hwport = hwport;
	phdmirx_obj->source_input = source_input;
	LOGD("%s,hwport=%d,source_input=%d\n",__func__, hwport, source_input);
	return 0;
}

int hdmirx_drv_g_input(struct hdmirx_obj_s *phdmirx_obj)
{
	LOGD("%s,%d,input=%d\n",__func__, __LINE__, phdmirx_obj->hwport);

	return phdmirx_obj->hwport;
}

int hdmirx_drv_querycap(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_capability *cap)
{
	LOGD("%s,%d\n",__func__, __LINE__);

	memset(cap, 0, sizeof(struct v4l2_capability));

	strncpy((char *)&cap->driver[0], "other_dvb_eu", strlen("other_dvb_eu"));
	strncpy((char *)&cap->card[0], phdmirx_obj->obj_name, strlen(phdmirx_obj->obj_name));
	cap->version = 0x20220429;/* KERNEL_VERSION */
	cap->capabilities = (V4L2_CAP_VIDEO_CAPTURE_MPLANE | V4L2_CAP_STREAMING | V4L2_CAP_READWRITE);
	cap->device_caps = (V4L2_CAP_VIDEO_CAPTURE_MPLANE | V4L2_CAP_STREAMING | V4L2_CAP_READWRITE);

	return 0;
}

/* V4L2_CID_EXT_VBE_MUTE */
int hdmirx_drv_s_ext_power_off(struct hdmirx_obj_s *phdmirx_obj, int onoff)
{
	int ret = 0;

	LOGD("%s,%d,onoff=%d\n",__func__, __LINE__, onoff);

	return 0;
}

/* V4L2_CID_EXT_HDMI_DISCONNECT */
int hdmirx_drv_s_ext_disconnect(struct hdmirx_obj_s *phdmirx_obj, int port)
{
	int ret = 0;

	LOGD("%s,%d,onoff=%d\n",__func__, __LINE__, port);

	return 0;
}

/* V4L2_CID_EXT_HDMI_TIMING_INFO */
int hdmirx_drv_g_ext_timing_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_timing_info *info)
{
	int ret;
	struct tvin_info_s SignalInfo;
	struct tvin_format_s Format;

	memset(&SignalInfo, 0, sizeof(SignalInfo));
	memset(&Format, 0, sizeof(Format));
	//memset(info, 0, sizeof(struct v4l2_ext_avd_timing_info));

	LOGD("i am here %s,%d\n",__func__, __LINE__);

	ret = phdmirx_obj->mp_VdinDrvIf->DeviceIOCtl(TVIN_IOC_G_SIG_INFO, &SignalInfo);
	if (ret < 0)
		LOGD("TVIN_IOC_G_SIG_INFO failed.%s,%d\n",__func__, __LINE__);

	LOGD("trans_fmt:%d,fmt:%#x,status:%d,cfmt:%d,fps:%d,is_dvi:%d,hdr_info:%x,ratio:%d\n",
		SignalInfo.trans_fmt, SignalInfo.fmt, SignalInfo.status,
		SignalInfo.cfmt, SignalInfo.fps, SignalInfo.is_dvi,
		SignalInfo.hdr_info, SignalInfo.aspect_ratio);

	ret = phdmirx_obj->mp_VdinDrvIf->DeviceIOCtl(TVIN_IOC_G_INPUT_TIMING, &Format);
	if (ret < 0)
		LOGD("TVIN_IOC_G_INPUT_TIMING failed.%s,%d\n",__func__, __LINE__);

	LOGD("active:%dx%d,h_cnt:%d-%d-%d,hs_cnt:%d-%d,total:%dx%d,hs:%d %d %d,vs:%d %d %d\n",
		Format.h_active, Format.v_active, Format.h_cnt, Format.h_cnt_offset, Format.v_cnt_offset,
		Format.hs_cnt, Format.hs_cnt_offset,Format.h_total, Format.v_total,Format.hs_front,
		Format.hs_width,Format.hs_bp,Format.vs_front,Format.vs_width, Format.vs_bp);
	LOGD("pol:%d %d,scan:%d,clk:%d,vbi:%d %d,duration:%d\n",
		Format.hs_pol, Format.vs_pol, Format.scan_mode, Format.pixel_clk,
		Format.vbi_line_start, Format.vbi_line_end, Format.duration);

	//convert and fill in v4l2_ext_hdmi_timing_info.
	if (Format.h_total)
		info->h_freq = (Format.pixel_clk * 10) / Format.h_total;//unit kHz
	else
		info->h_freq = 0;//unit Hz
	info->v_vreq  = SignalInfo.fps;
	info->h_total = Format.h_total;
	info->v_total = Format.v_total;
	info->h_porch = Format.hs_front;
	info->v_porch = Format.vs_front;
	info->active.x = 0;
	info->active.y = 0;
	info->active.w = Format.h_active;
	info->active.h = Format.v_active;
	info->scan_type = (unsigned short)Format.scan_mode;
	info->dvi_hdmi  = V4L2_EXT_HDMI_MODE_HDMI;//todo
	info->color_depth = V4L2_EXT_HDMI_COLOR_DEPTH_8BIT;//todo
	info->allm_mode = 0;//todo

	memcpy(&phdmirx_obj->timing_info, info, sizeof(struct v4l2_ext_hdmi_timing_info));

	return 0;
}

/* V4L2_CID_EXT_HDMI_DRM_INFO */
int hdmirx_drv_g_ext_drm_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_drm_info *info)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;
	phdmirx_obj->drm_info.display_primaries_x0 = 123;
	phdmirx_obj->drm_info.display_primaries_y0 = 456;

	memcpy(info, &phdmirx_obj->drm_info, sizeof(struct v4l2_ext_hdmi_drm_info));
	return 0;
}

/* V4L2_CID_EXT_HDMI_VSI_INFO */
int hdmirx_drv_g_ext_vsi_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_vsi_info *info)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;
	phdmirx_obj->vsi_info.video_format = V4L2_EXT_HDMI_VSI_VIDEO_FORMAT_EXTENDED_RESOLUTION_FORMAT;
	phdmirx_obj->vsi_info.vic = V4L2_EXT_HDMI_VSI_VIC_4K2K_30HZ;

	memcpy(info, &phdmirx_obj->vsi_info, sizeof(struct v4l2_ext_hdmi_vsi_info));
	return 0;
}

/* V4L2_CID_EXT_HDMI_SPD_INFO */
int hdmirx_drv_g_ext_spd_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_spd_info *info)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;
	//phdmirx_obj->spd_info.vendor_name="amlogic";

	phdmirx_obj->spd_info.packet.type = 1122;

	memcpy(info, &phdmirx_obj->spd_info, sizeof(struct v4l2_ext_hdmi_spd_info));
	return 0;
}

/* V4L2_CID_EXT_HDMI_AVI_INFO */
int hdmirx_drv_g_ext_avi_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_avi_info *info)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;
	phdmirx_obj->avi_info.active_format_aspect_ratio = V4L2_EXT_HDMI_AVI_ACTIVE_FORMAT_ARC_16_9CENTER;
	phdmirx_obj->avi_info.mode = V4L2_EXT_HDMI_MODE_HDMI;

	memcpy(info, &phdmirx_obj->avi_info, sizeof(struct v4l2_ext_hdmi_avi_info));
	return 0;
}

/* V4L2_CID_EXT_HDMI_PACKET_INFO */
int hdmirx_drv_g_ext_packet_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_packet_info *info)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(info, &phdmirx_obj->packet_info, sizeof(struct v4l2_ext_hdmi_packet_info));
	return 0;
}

/* V4L2_CID_EXT_HDMI_DOLBY_HDR */
int hdmirx_drv_g_ext_dolby_hdr(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_dolby_hdr *dolby_hdr)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(dolby_hdr, &phdmirx_obj->dolby_hdr, sizeof(struct v4l2_ext_hdmi_dolby_hdr));
	return 0;
}

/* V4L2_CID_EXT_HDMI_EDID */
int hdmirx_drv_g_ext_edid(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_edid **edid)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	//memcpy(edid, &phdmirx_obj->edid, sizeof(struct v4l2_ext_hdmi_edid));
	*edid = &phdmirx_obj->edid;
	return 0;
}

/* V4L2_CID_EXT_HDMI_CONNECTION_STATE */
int hdmirx_drv_g_ext_connection_state(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_connection_state *state)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(state, &phdmirx_obj->connection_state, sizeof(struct v4l2_ext_hdmi_connection_state));
	return 0;
}

/* V4L2_CID_EXT_HDMI_HPD */
int hdmirx_drv_g_ext_hpd(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hpd *hpd)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(hpd, &phdmirx_obj->hpd, sizeof(struct v4l2_ext_hdmi_hpd));
	return 0;
}

/* V4L2_CID_EXT_HDMI_VRR_FREQUENCY */
int hdmirx_drv_g_ext_vrr_frequency(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_vrr_frequency *vrr_frequency)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(vrr_frequency, &phdmirx_obj->vrr_frequency, sizeof(struct v4l2_ext_hdmi_vrr_frequency));
	return 0;
}

/* V4L2_CID_EXT_HDMI_EMP_INFO */
int hdmirx_drv_g_ext_emp_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_emp_info *emp_info)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(emp_info, &phdmirx_obj->emp_info, sizeof(struct v4l2_ext_hdmi_emp_info));
	return 0;
}

/* V4L2_CID_EXT_HDMI_HDCP_REPEATER */
int hdmirx_drv_g_ext_hdcp_repeater(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_repeater *hdcp_repeater)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(hdcp_repeater, &phdmirx_obj->hdcp_repeater, sizeof(struct v4l2_ext_hdmi_hdcp_repeater));
	return 0;
}

/* V4L2_CID_EXT_HDMI_QUERYCAP */
int hdmirx_drv_g_ext_querycap(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_querycap *querycap)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(querycap, &phdmirx_obj->querycap, sizeof(struct v4l2_ext_hdmi_querycap));
	return 0;
}

/* V4L2_CID_EXT_HDMI_HPD_LOW_DURATION_DC_ON */
int hdmirx_drv_g_ext_dc_on(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hpd_low_duration_dc_on *dc_on)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(dc_on, &phdmirx_obj->dc_on, sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on));
	return 0;
}

/* V4L2_CID_EXT_HDMI_SLEEP */
int hdmirx_drv_g_ext_sleep_mode(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_sleep *sleep_mode)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(sleep_mode, &phdmirx_obj->sleep_mode, sizeof(struct v4l2_ext_hdmi_sleep));
	return 0;
}

/* V4L2_CID_EXT_HDMI_DIAGNOSTICS_STATUS */
int hdmirx_drv_g_ext_diag_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_diagnostics_status *status)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(status, &phdmirx_obj->diag_status, sizeof(struct v4l2_ext_hdmi_diagnostics_status));
	return 0;
}

/* V4L2_CID_EXT_HDMI_PHY_STATUS */
int hdmirx_drv_g_ext_phy_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_phy_status *status)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(status, &phdmirx_obj->phy_status, sizeof(struct v4l2_ext_hdmi_phy_status));
	return 0;
}

/* V4L2_CID_EXT_HDMI_LINK_STATUS */
int hdmirx_drv_g_ext_link_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_link_status *status)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(status, &phdmirx_obj->link_status, sizeof(struct v4l2_ext_hdmi_link_status));
	return 0;
}

/* V4L2_CID_EXT_HDMI_VIDEO_STATUS */
int hdmirx_drv_g_ext_video_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_video_status *status)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(status, &phdmirx_obj->video_status, sizeof(struct v4l2_ext_hdmi_video_status));
	return 0;
}

/* V4L2_CID_EXT_HDMI_AUDIO_STATUS */
int hdmirx_drv_g_ext_audio_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_audio_status *status)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(status, &phdmirx_obj->video_status, sizeof(struct v4l2_ext_hdmi_video_status));
	return 0;
}

/* V4L2_CID_EXT_HDMI_HDCP_STATUS */
int hdmirx_drv_g_ext_hdcp_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_status *status)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(status, &phdmirx_obj->hdcp_status, sizeof(struct v4l2_ext_hdmi_hdcp_status));
	return 0;
}

/* V4L2_CID_EXT_HDMI_SCDC_STATUS */
int hdmirx_drv_g_ext_scdc_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_scdc_status *status)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(status, &phdmirx_obj->scdc_status, sizeof(struct v4l2_ext_hdmi_scdc_status));
	return 0;
}

/* V4L2_CID_EXT_HDMI_ERROR_STATUS */
int hdmirx_drv_g_ext_error_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_error_status *status)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(status, &phdmirx_obj->scdc_status, sizeof(struct v4l2_ext_hdmi_scdc_status));
	return 0;
}

/* V4L2_CID_EXT_HDMI_EDID */
int hdmirx_drv_s_ext_edid(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_edid *edid)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	LOGD("edid data:%x %x %x\n", edid->pData[0], edid->pData[1], edid->pData[2]);

	phdmirx_obj->edid.port = edid->port;
	phdmirx_obj->edid.size = edid->size;

	if (edid->size == V4L2_EXT_HDMI_EDID_SIZE_128)
		memcpy(phdmirx_obj->edid.pData, edid->pData, 128);
	else if (edid->size == V4L2_EXT_HDMI_EDID_SIZE_256)
		memcpy(phdmirx_obj->edid.pData, edid->pData, 256);
	else if (edid->size == V4L2_EXT_HDMI_EDID_SIZE_512)
		memcpy(phdmirx_obj->edid.pData, edid->pData, 512);

	LOGD("22edid data:%x %x %x\n", phdmirx_obj->edid.pData[0],
		phdmirx_obj->edid.pData[1], phdmirx_obj->edid.pData[2]);

	return 0;
}

/* V4L2_CID_EXT_HDMI_HPD */
int hdmirx_drv_s_ext_hpd(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hpd *hpd)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(&phdmirx_obj->hpd, hpd, sizeof(struct v4l2_ext_hdmi_hpd));
	return 0;
}

/* V4L2_CID_EXT_HDMI_HDCP_KEY */
int hdmirx_drv_s_hdcp_key(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_key *hdcp_key)
{
	LOGD("%s,%d;port:%d,version:%#x,pData:%d %d %d\n", __func__, __LINE__,
		hdcp_key->port,		hdcp_key->version,
		hdcp_key->pData[0], hdcp_key->pData[1], hdcp_key->pData[2]);
	if (hdcp_key->version >= V4L2_EXT_HDMI_HDCP_VERSION_RESERVED ||
		(hdcp_key->version == V4L2_EXT_HDMI_HDCP_VERSION_14 &&
		 hdcp_key->key_size != HDCP14_KEY_SIZE) ||
		(hdcp_key->version == V4L2_EXT_HDMI_HDCP_VERSION_22 &&
		 hdcp_key->key_size != HDCP22_KEY_SIZE))
		return -EINVAL;

	phdmirx_obj->hdcp_key.port     = hdcp_key->port;
	phdmirx_obj->hdcp_key.version  = hdcp_key->version;
	phdmirx_obj->hdcp_key.key_size = hdcp_key->key_size;
	memcpy(&phdmirx_obj->hdcp_key.pData, hdcp_key->pData, hdcp_key->key_size);
	LOGD("%s,%d\n", __func__, __LINE__);

	return 0;
}

/* V4L2_CID_EXT_HDMI_HDCP_REPEATER */
int hdmirx_drv_s_hdcp_repeater(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_repeater *hdcp_repeater)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(&phdmirx_obj->hdcp_repeater, hdcp_repeater, sizeof(struct v4l2_ext_hdmi_hdcp_repeater));
	return 0;
}

/* V4L2_CID_EXT_HDMI_OVERRIDE_EOTF */
int hdmirx_drv_s_override_eotf(struct hdmirx_obj_s *phdmirx_obj, v4l2_ext_hdmi_override_eotf eotf)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	phdmirx_obj->eotf = eotf;

	//memcpy(&phdmirx_obj->eotf, eotf, sizeof(struct v4l2_ext_hdmi_override_eotf));
	return 0;
}

/* V4L2_CID_EXT_HDMI_HPD_LOW_DURATION_DC_ON */
int hdmirx_drv_s_dc_on(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hpd_low_duration_dc_on *dc_on)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(&phdmirx_obj->dc_on, dc_on, sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on));
	return 0;
}

/* V4L2_CID_EXT_HDMI_HDCP_REPEATER_TOPOLOGY */
int hdmirx_drv_s_repeater_topology(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_repeater_topology *topology)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(&phdmirx_obj->topology, topology, sizeof(struct v4l2_ext_hdmi_hdcp_repeater_topology));
	return 0;
}

/* V4L2_CID_EXT_HDMI_HDCP_REPEATER_STREAM_MANAGE */
int hdmirx_drv_s_stream_manage(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_repeater_stream_manage *stream_manage)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(&phdmirx_obj->stream_manage, stream_manage, sizeof(struct v4l2_ext_hdmi_hdcp_repeater_stream_manage));
	return 0;
}

/* V4L2_CID_EXT_HDMI_SLEEP */
int hdmirx_drv_s_sleep(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_sleep *sleep_mode)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(&phdmirx_obj->sleep_mode, sleep_mode, sizeof(struct v4l2_ext_hdmi_sleep));
	return 0;
}
/* V4L2_CID_EXT_HDMI_EXPERT_SETTING */
int hdmirx_drv_s_expert_setting(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_expert_setting *expert_setting)
{
	LOGD("%s,%d\n", __func__, __LINE__);
	//for test
	//phdmirx_obj->timing_info.port = ;

	memcpy(&phdmirx_obj->expert_setting, expert_setting, sizeof(struct v4l2_ext_hdmi_expert_setting));
	return 0;
}

#ifdef __cplusplus
}
#endif
