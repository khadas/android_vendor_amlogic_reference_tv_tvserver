/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_MOUDLE_TAG "TVIN-HDMIRX"
#define LOG_CLASS_TAG "HDMIRX-FUSE"

#include "../common.h"
#include "hdmirx_fuse.h"

#ifdef __cplusplus
extern "C" {
#endif

pthread_mutex_t hdmirx_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_lock(&hdmirx_mutex);
//pthread_mutex_unlock(&hdmirx_mutex);

static void hdmirx_cuse_open(fuse_req_t req, struct fuse_file_info *fi)
{
	int ret = 0;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct hdmirx_obj_s *phdmirx_obj = (struct hdmirx_obj_s *)(pcuse_obj->pri);

	pthread_mutex_lock(&hdmirx_mutex);
	ret = hdmirx_drv_open(phdmirx_obj, fi->flags);
	if (ret == 0)
		fuse_reply_open(req, fi);
	else
		fuse_reply_err(req, ret);
	pthread_mutex_unlock(&hdmirx_mutex);
}

static void hdmirx_cuse_release(fuse_req_t req, struct fuse_file_info *fi)
{
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct hdmirx_obj_s *phdmirx_obj = (struct hdmirx_obj_s *)(pcuse_obj->pri);

	pthread_mutex_lock(&hdmirx_mutex);
	hdmirx_drv_close(phdmirx_obj);

	fuse_reply_buf(req, NULL, 0);
	pthread_mutex_unlock(&hdmirx_mutex);
}

static void hdmirx_cuse_read(fuse_req_t req, size_t size, off_t off,
		      struct fuse_file_info *fi)
{
	LOGD("%s,%d\n",__func__, __LINE__);
	fuse_reply_buf(req, NULL, 0);
}

static void hdmirx_cuse_write(fuse_req_t req, const char *buf, size_t size, off_t off,
		       struct fuse_file_info *fi)
{
	LOGD("%s,%d\n",__func__, __LINE__);
    fuse_reply_write(req, size);
}

static void hdmirx_cuse_g_ctrl(fuse_req_t req, const void *in_buf)
{
	int ret = 0;
	struct v4l2_control *control = (struct v4l2_control *)in_buf;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct hdmirx_obj_s *phdmirx_obj = (struct hdmirx_obj_s *)(pcuse_obj->pri);

	LOGD("%s,%d\n",__func__, __LINE__);

    if (!control) {
        fuse_reply_err(req, EINVAL);
        return;
    }

    switch (control->id) {
        default:
			ret = -EINVAL;
            break;
    }
    fuse_reply_ioctl(req, ret, control, sizeof(*control));
}

static void hdmirx_cuse_s_ctrl(fuse_req_t req, const void *in_buf)
{
	int ret = 0;
	struct v4l2_control *control = (struct v4l2_control *)in_buf;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct hdmirx_obj_s *phdmirx_obj = (struct hdmirx_obj_s *)(pcuse_obj->pri);

	if (!control) {
		fuse_reply_err(req, EINVAL);
		return;
	}

	switch (control->id) {
		case V4L2_CID_EXT_HDMI_POWER_OFF:
			ret = hdmirx_drv_s_power_off(phdmirx_obj, control->value);
			break;
		case V4L2_CID_EXT_HDMI_DISCONNECT:
			ret = hdmirx_drv_s_disconnect(phdmirx_obj, control->value);
			break;
		default:
			break;
	}
	fuse_reply_ioctl(req, ret, control, sizeof(*control));
}

static void hdmirx_cuse_g_ext_ctrls(fuse_req_t req, void *arg, const void *in_buf,
    size_t in_bufsz, size_t out_bufsz)
{
    int ret = 0;
    struct iovec iov_r[2], iov_w[2], iov, iov_ret[2];
    struct v4l2_ext_controls *ext_controls;
    struct v4l2_ext_control *ext_control;
    struct v4l2_ext_avd_timing_info avd_timing_info;
    struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
    struct hdmirx_obj_s *phdmirx_obj = (struct hdmirx_obj_s *)(pcuse_obj->pri);

    LOGD("%d, in_bufsz:%d\n", __LINE__, in_bufsz);

    //fetch v4l2_ext_controls
    iov_r[0].iov_base = arg;
    iov_r[0].iov_len  = sizeof(struct v4l2_ext_controls);
    if (!in_bufsz) {
        fuse_reply_ioctl_retry(req, iov_r, 1, NULL, 0);
        return;
    }

    ext_controls = (struct v4l2_ext_controls *)in_buf;
    in_buf   += sizeof(struct v4l2_ext_controls);
    in_bufsz -= sizeof(struct v4l2_ext_controls);
    LOGD("%d, in_bufsz:%d ctls_size:%d\n", __LINE__, in_bufsz, sizeof(struct v4l2_ext_controls));

    //fetch v4l2_ext_control
    iov_r[1].iov_base = ext_controls->controls;
    iov_r[1].iov_len = sizeof(struct v4l2_ext_control);
    if (!in_bufsz) {
        fuse_reply_ioctl_retry(req, iov_r, 2, NULL, 0);
        return;
    }

    ext_control = (struct v4l2_ext_control *)in_buf;
    in_buf   += sizeof(struct v4l2_ext_control);
    in_bufsz -= sizeof(struct v4l2_ext_control);
    LOGD("%d, in_bufsz:%d ctl_size:%d\n", __LINE__, in_bufsz, sizeof(struct v4l2_ext_control));

    iov_w[0].iov_base = ext_control->ptr;

    switch (ext_control->id) {
		case V4L2_CID_EXT_HDMI_TIMING_INFO:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_timing_info);
			LOGD("%d,out_bufsz:%x,info:%x,ctl_size:%x\n", __LINE__, out_bufsz,
				sizeof(struct v4l2_ext_hdmi_timing_info), ext_control->size);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_timing_info info;
			LOGD("%d,out_bufsz:%x,info:%x,ctl_size:%x\n", __LINE__, out_bufsz,
				sizeof(struct v4l2_ext_hdmi_timing_info), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_timing_info))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			ret = hdmirx_drv_g_ext_timing_info(phdmirx_obj, &info);
			iov.iov_base = &info;
			iov.iov_len  = sizeof(info);
			break;
		case V4L2_CID_EXT_HDMI_DRM_INFO:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_drm_info);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_drm_info drm_info;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_drm_info), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_drm_info))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&drm_info, 0, sizeof(drm_info));
			ret = hdmirx_drv_g_ext_drm_info(phdmirx_obj, &drm_info);
			iov.iov_base = &drm_info;
			iov.iov_len = sizeof(drm_info);
			break;
		case V4L2_CID_EXT_HDMI_VSI_INFO:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_vsi_info);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_vsi_info vsi_info;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_vsi_info), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_vsi_info))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&vsi_info, 0, sizeof(vsi_info));
			ret = hdmirx_drv_g_ext_vsi_info(phdmirx_obj, &vsi_info);
			iov.iov_base = &vsi_info;
			iov.iov_len = sizeof(vsi_info);

			break;

		case V4L2_CID_EXT_HDMI_SPD_INFO:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_spd_info);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_spd_info spd_info;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_spd_info), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_spd_info))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&spd_info, 0, sizeof(spd_info));
			ret = hdmirx_drv_g_ext_spd_info(phdmirx_obj, &spd_info);
			iov.iov_base = &spd_info;
			iov.iov_len = sizeof(spd_info);

			break;
		case V4L2_CID_EXT_HDMI_AVI_INFO:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_avi_info);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_avi_info avi_info;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_avi_info), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_avi_info))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&avi_info, 0, sizeof(avi_info));
			ret = hdmirx_drv_g_ext_avi_info(phdmirx_obj, &avi_info);
			iov.iov_base = &avi_info;
			iov.iov_len = sizeof(avi_info);
			break;
		case V4L2_CID_EXT_HDMI_PACKET_INFO:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_packet_info);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_packet_info packet_info;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_packet_info), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_packet_info))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&packet_info, 0, sizeof(packet_info));
			ret = hdmirx_drv_g_ext_packet_info(phdmirx_obj, &packet_info);
			iov.iov_base = &packet_info;
			iov.iov_len = sizeof(packet_info);
			break;
		case V4L2_CID_EXT_HDMI_DOLBY_HDR:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_dolby_hdr);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_dolby_hdr dolby_hdr;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_dolby_hdr), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_dolby_hdr))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			ret = hdmirx_drv_g_ext_dolby_hdr(phdmirx_obj, &dolby_hdr);
			iov.iov_base = &dolby_hdr;
			iov.iov_len = sizeof(dolby_hdr);
			break;
		case V4L2_CID_EXT_HDMI_EDID:
			struct v4l2_ext_hdmi_edid *pedid;
			int size;
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_edid))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len = sizeof(struct v4l2_ext_hdmi_edid);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			pedid = (struct v4l2_ext_hdmi_edid *)in_buf;
			in_buf += sizeof(struct v4l2_ext_hdmi_edid);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_edid);
			iov_w[0].iov_base = pedid->pData;
			if (pedid->size == V4L2_EXT_HDMI_EDID_SIZE_128)
				size = 128;
			else if (pedid->size == V4L2_EXT_HDMI_EDID_SIZE_256)
				size = 256;
			else if (pedid->size == V4L2_EXT_HDMI_EDID_SIZE_512)
				size = 512;
			iov_w[0].iov_len  = size;//pedid->size;
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			ret = hdmirx_drv_g_ext_edid(phdmirx_obj, pedid);
			iov_ret[0].iov_base = pedid->pData;
			iov_ret[0].iov_len = size;
			fuse_reply_ioctl_iov(req, ret, iov_ret, 1);
			return;
		case V4L2_CID_EXT_HDMI_CONNECTION_STATE:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_connection_state);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_connection_state connection_state;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_connection_state), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_connection_state))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&connection_state, 0, sizeof(connection_state));
			ret = hdmirx_drv_g_ext_connection_state(phdmirx_obj, &connection_state);
			iov.iov_base = &connection_state;
			iov.iov_len = sizeof(connection_state);
			break;
		case V4L2_CID_EXT_HDMI_HPD:
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_hpd))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_hpd
			struct v4l2_ext_hdmi_hpd *in_hpd;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_hpd);
			LOGD("%d, in_bufsz:%d hpd_size:%d\n", __LINE__, in_bufsz, sizeof(struct v4l2_ext_hdmi_hpd));
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_hpd = (struct v4l2_ext_hdmi_hpd *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_hpd);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_hpd);
			LOGD("%d, in_bufsz:%d hpd_size:%d\n", __LINE__, in_bufsz, sizeof(struct v4l2_ext_hdmi_hpd));
			//fetch write address
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_hpd);
			LOGD("out_bufsz:%d, info:%d, ctl_size:%d\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_hpd), ext_control->size);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get hpd info from driver
			struct v4l2_ext_hdmi_hpd hpd;
			memset(&hpd, 0, sizeof(hpd));
			hpd.port = in_hpd->port;
			ret = hdmirx_drv_g_ext_hpd(phdmirx_obj, &hpd);

			iov.iov_base = &hpd;
			iov.iov_len = sizeof(hpd);
			break;
		case V4L2_CID_EXT_HDMI_VRR_FREQUENCY:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_vrr_frequency);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_vrr_frequency vrr_frequency;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_vrr_frequency), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_vrr_frequency))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&vrr_frequency, 0, sizeof(vrr_frequency));
			ret = hdmirx_drv_g_ext_vrr_frequency(phdmirx_obj, &vrr_frequency);
			iov.iov_base = &vrr_frequency;
			iov.iov_len = sizeof(vrr_frequency);
			break;
		case V4L2_CID_EXT_HDMI_EMP_INFO:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_emp_info);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_emp_info emp_info;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_emp_info), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_emp_info))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&emp_info, 0, sizeof(emp_info));
			ret = hdmirx_drv_g_ext_emp_info(phdmirx_obj, &emp_info);
			iov.iov_base = &emp_info;
			iov.iov_len = sizeof(emp_info);
			break;
		case V4L2_CID_EXT_HDMI_HDCP_REPEATER:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_hdcp_repeater);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_hdcp_repeater hdcp_repeater;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_hdcp_repeater), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_hdcp_repeater))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&hdcp_repeater, 0, sizeof(hdcp_repeater));
			ret = hdmirx_drv_g_ext_hdcp_repeater(phdmirx_obj, &hdcp_repeater);
			iov.iov_base = &hdcp_repeater;
			iov.iov_len = sizeof(hdcp_repeater);
			break;
		case V4L2_CID_EXT_HDMI_HDCP_REPEATER_STREAM_MANAGE:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_hdcp_repeater_stream_manage);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_hdcp_repeater_stream_manage stream_manage;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_hdcp_repeater_stream_manage), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_hdcp_repeater_stream_manage))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&stream_manage, 0, sizeof(stream_manage));
			ret = hdmirx_drv_g_ext_stream_manage(phdmirx_obj, &stream_manage);
			iov.iov_base = &stream_manage;
			iov.iov_len = sizeof(stream_manage);
			break;
		case V4L2_CID_EXT_HDMI_QUERYCAP:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_querycap);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_querycap querycap;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_querycap), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_querycap))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&querycap, 0, sizeof(querycap));
			ret = hdmirx_drv_g_ext_querycap(phdmirx_obj, &querycap);
			iov.iov_base = &querycap;
			iov.iov_len = sizeof(querycap);
			break;
		case V4L2_CID_EXT_HDMI_HPD_LOW_DURATION_DC_ON:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_hpd_low_duration_dc_on dc_on;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&dc_on, 0, sizeof(dc_on));
			ret = hdmirx_drv_g_ext_dc_on(phdmirx_obj, &dc_on);
			iov.iov_base = &dc_on;
			iov.iov_len = sizeof(dc_on);
			break;
		case V4L2_CID_EXT_HDMI_SLEEP:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_sleep);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_sleep sleep_mode;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_sleep), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_sleep))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&sleep_mode, 0, sizeof(sleep_mode));
			ret = hdmirx_drv_g_ext_sleep_mode(phdmirx_obj, &sleep_mode);
			iov.iov_base = &sleep_mode;
			iov.iov_len = sizeof(sleep_mode);
			break;
		case V4L2_CID_EXT_HDMI_DIAGNOSTICS_STATUS:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_diagnostics_status);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_diagnostics_status status;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_diagnostics_status), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_diagnostics_status))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&status, 0, sizeof(status));
			ret = hdmirx_drv_g_ext_diag_status(phdmirx_obj, &status);
			iov.iov_base = &status;
			iov.iov_len = sizeof(status);
			break;
		case V4L2_CID_EXT_HDMI_PHY_STATUS:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_phy_status);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_phy_status phy_status;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_phy_status), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_phy_status))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&phy_status, 0, sizeof(phy_status));
			ret = hdmirx_drv_g_ext_phy_status(phdmirx_obj, &phy_status);
			iov.iov_base = &phy_status;
			iov.iov_len = sizeof(phy_status);
			break;
		case V4L2_CID_EXT_HDMI_LINK_STATUS:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_link_status);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_link_status link_status;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_link_status), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_link_status))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&link_status, 0, sizeof(link_status));
			ret = hdmirx_drv_g_ext_link_status(phdmirx_obj, &link_status);
			iov.iov_base = &link_status;
			iov.iov_len = sizeof(link_status);
			break;
		case V4L2_CID_EXT_HDMI_VIDEO_STATUS:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_video_status);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_video_status video_status;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_video_status), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_video_status))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&video_status, 0, sizeof(video_status));
			ret = hdmirx_drv_g_ext_video_status(phdmirx_obj, &video_status);
			iov.iov_base = &video_status;
			iov.iov_len = sizeof(video_status);
			break;
		case V4L2_CID_EXT_HDMI_AUDIO_STATUS:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_audio_status);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_audio_status audio_status;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_audio_status), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_audio_status))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&audio_status, 0, sizeof(audio_status));
			ret = hdmirx_drv_g_ext_audio_status(phdmirx_obj, &audio_status);
			iov.iov_base = &audio_status;
			iov.iov_len = sizeof(audio_status);
			break;
		case V4L2_CID_EXT_HDMI_HDCP_STATUS:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_hdcp_status);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_hdcp_status hdcp_status;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_hdcp_status), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_hdcp_status))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&hdcp_status, 0, sizeof(hdcp_status));
			ret = hdmirx_drv_g_ext_hdcp_status(phdmirx_obj, &hdcp_status);
			iov.iov_base = &hdcp_status;
			iov.iov_len = sizeof(hdcp_status);
			break;
		case V4L2_CID_EXT_HDMI_SCDC_STATUS:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_hdcp_status);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_scdc_status scdc_status;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_hdcp_status), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_hdcp_status))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&scdc_status, 0, sizeof(scdc_status));
			ret = hdmirx_drv_g_ext_scdc_status(phdmirx_obj, &scdc_status);
			iov.iov_base = &scdc_status;
			iov.iov_len = sizeof(scdc_status);
			break;
		case V4L2_CID_EXT_HDMI_ERROR_STATUS:
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_error_status);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 2, iov_w, 1);
			    return;
			}
			struct v4l2_ext_hdmi_error_status error_status;
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_hdmi_error_status), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_error_status))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			memset(&error_status, 0, sizeof(error_status));
			ret = hdmirx_drv_g_ext_error_status(phdmirx_obj, &error_status);
			iov.iov_base = &error_status;
			iov.iov_len = sizeof(error_status);
			break;

		default:
			fuse_reply_err(req, EINVAL);
			return;
			//break;
	}
	fuse_reply_ioctl_iov(req, ret, &iov, 1);
}

static void hdmirx_cuse_s_ext_ctrls(fuse_req_t req, void *arg,
    const void *in_buf, size_t in_bufsz)
{
	int ret = 0;
	unsigned int size = 0;
	struct iovec iov_r[4];
	struct v4l2_ext_controls *ext_controls;
	struct v4l2_ext_control *ext_control;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct hdmirx_obj_s *phdmirx_obj = (struct hdmirx_obj_s *)(pcuse_obj->pri);

    iov_r[0].iov_base = arg;
    iov_r[0].iov_len = sizeof(struct v4l2_ext_controls);
    if (!in_bufsz) {
        fuse_reply_ioctl_retry(req, iov_r, 1, NULL, 0);
        return;
    }

    ext_controls = (struct v4l2_ext_controls *)in_buf;
    in_buf += sizeof(struct v4l2_ext_controls);
    in_bufsz -= sizeof(struct v4l2_ext_controls);

    iov_r[1].iov_base = ext_controls->controls;
    iov_r[1].iov_len = sizeof(struct v4l2_ext_control);
    if (!in_bufsz) {
        fuse_reply_ioctl_retry(req, iov_r, 2, NULL, 0);
        return;
    }

    ext_control = (struct v4l2_ext_control *)in_buf;
    in_buf += sizeof(struct v4l2_ext_control);
    in_bufsz -= sizeof(struct v4l2_ext_control);

    switch (ext_control->id) {
        case V4L2_CID_EXT_HDMI_EDID:
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len = sizeof(struct v4l2_ext_hdmi_edid);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			struct v4l2_ext_hdmi_edid *hdmi_edid;
			hdmi_edid = (struct v4l2_ext_hdmi_edid *)in_buf;
			in_buf += sizeof(struct v4l2_ext_hdmi_edid);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_edid);
			if (hdmi_edid->size == V4L2_EXT_HDMI_EDID_SIZE_128)
				size = 128;
			else if (hdmi_edid->size == V4L2_EXT_HDMI_EDID_SIZE_256)
				size = 256;
			else if (hdmi_edid->size == V4L2_EXT_HDMI_EDID_SIZE_512)
				size = 512;

			iov_r[3].iov_base = hdmi_edid->pData;
			iov_r[3].iov_len  = size;
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 4, NULL, 0);
				return;
			}
			hdmi_edid->pData = (unsigned char *)in_buf;
			ret = hdmirx_drv_s_ext_edid(phdmirx_obj, hdmi_edid);
			break;
		case V4L2_CID_EXT_HDMI_HPD:
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len = sizeof(struct v4l2_ext_hdmi_hpd);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			ret = hdmirx_drv_s_ext_hpd(phdmirx_obj, (struct v4l2_ext_hdmi_hpd *)in_buf);
			break;
		case V4L2_CID_EXT_HDMI_HDCP_KEY:
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len = sizeof(struct v4l2_ext_hdmi_hdcp_key);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			struct v4l2_ext_hdmi_hdcp_key *hdcp_key;
			hdcp_key = (struct v4l2_ext_hdmi_hdcp_key *)in_buf;
			in_buf += sizeof(struct v4l2_ext_hdmi_hdcp_key);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_hdcp_key);
			iov_r[3].iov_base = hdcp_key->pData;
			iov_r[3].iov_len = hdcp_key->key_size;
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 4, NULL, 0);
				return;
			}
			hdcp_key->pData = (unsigned char *)in_buf;
			ret = hdmirx_drv_s_ext_hdcp_key(phdmirx_obj, hdcp_key);
			break;
		case V4L2_CID_EXT_HDMI_HDCP_REPEATER:
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len = sizeof(struct v4l2_ext_hdmi_hdcp_repeater);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			ret = hdmirx_drv_s_ext_hdcp_repeater(phdmirx_obj, (struct v4l2_ext_hdmi_hdcp_repeater *)in_buf);
			break;
		case V4L2_CID_EXT_HDMI_OVERRIDE_EOTF:
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len = sizeof(v4l2_ext_hdmi_override_eotf);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			ret = hdmirx_drv_s_ext_override_eotf(phdmirx_obj, *(v4l2_ext_hdmi_override_eotf *)in_buf);
			break;
		case V4L2_CID_EXT_HDMI_HPD_LOW_DURATION_DC_ON:
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len = sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			ret = hdmirx_drv_s_ext_dc_on(phdmirx_obj, (struct v4l2_ext_hdmi_hpd_low_duration_dc_on *)in_buf);
			break;
		case V4L2_CID_EXT_HDMI_HDCP_REPEATER_TOPOLOGY:
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len = sizeof(struct v4l2_ext_hdmi_hdcp_repeater_topology);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			ret = hdmirx_drv_s_ext_repeater_topology(phdmirx_obj, (struct v4l2_ext_hdmi_hdcp_repeater_topology *)in_buf);
			break;
			break;
		case V4L2_CID_EXT_HDMI_SLEEP:
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len = sizeof(struct v4l2_ext_hdmi_sleep);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			ret = hdmirx_drv_s_ext_sleep(phdmirx_obj, (struct v4l2_ext_hdmi_sleep *)in_buf);
			break;
		case V4L2_CID_EXT_HDMI_EXPERT_SETTING:
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len = sizeof(struct v4l2_ext_hdmi_expert_setting);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			ret = hdmirx_drv_s_ext_expert_setting(phdmirx_obj, (struct v4l2_ext_hdmi_expert_setting *)in_buf);
			break;
		default:
			break;
	}
	fuse_reply_ioctl_iov(req, ret, NULL, 0);
}

static void hdmirx_cuse_ioctl(fuse_req_t req, int cmd, void *arg,
		       struct fuse_file_info *fi, unsigned int flags,
		       const void *in_buf, size_t in_bufsz, size_t out_bufsz)
{
	int ret = 0;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct hdmirx_obj_s *phdmirx_obj = (struct hdmirx_obj_s *)(pcuse_obj->pri);

	pthread_mutex_lock(&hdmirx_mutex);
	switch (cmd) {
		case VIDIOC_S_INPUT:
			if (!in_bufsz) {
				struct iovec iov_r = { arg, sizeof(int) };
				fuse_reply_ioctl_retry(req, &iov_r, 1, NULL, 0);
				break;
			}
			ret = hdmirx_drv_s_input(phdmirx_obj, *(int *)in_buf);
			fuse_reply_ioctl(req, ret, NULL, 0);
			break;
		case VIDIOC_G_INPUT:
			if (!out_bufsz) {
				struct iovec iov_w = { arg, sizeof(int) };
				fuse_reply_ioctl_retry(req, NULL, 0, &iov_w, 1);
				break;
			}
			ret = hdmirx_drv_g_input(phdmirx_obj);
			fuse_reply_ioctl(req, 0, &ret, sizeof(ret));
			break;
		case VIDIOC_S_CTRL:
			if (!in_bufsz || !out_bufsz) {
				struct iovec iov_r = { arg, sizeof(struct v4l2_control) };
				struct iovec iov_w = { arg, sizeof(struct v4l2_control) };
				fuse_reply_ioctl_retry(req, &iov_r, 1, &iov_w, 1);
			} else {
				hdmirx_cuse_s_ctrl(req, in_buf);
			}
			break;
		case VIDIOC_G_CTRL:
			if (!in_bufsz || !out_bufsz) {
				struct iovec iov_r = { arg, sizeof(struct v4l2_control) };
				struct iovec iov_w = { arg, sizeof(struct v4l2_control) };
				fuse_reply_ioctl_retry(req, &iov_r, 1, &iov_w, 1);
			} else {
				hdmirx_cuse_g_ctrl(req, in_buf);
			}
			break;
		case VIDIOC_S_EXT_CTRLS:
			hdmirx_cuse_s_ext_ctrls(req, arg, in_buf, in_bufsz);
			break;
		case VIDIOC_G_EXT_CTRLS:
			hdmirx_cuse_g_ext_ctrls(req, arg, in_buf, in_bufsz, out_bufsz);
			break;
		case VIDIOC_QUERYCAP:
			if (!in_bufsz || !out_bufsz) {
				struct iovec iov_r = { arg, sizeof(struct v4l2_capability) };
				struct iovec iov_w = { arg, sizeof(struct v4l2_capability) };
				fuse_reply_ioctl_retry(req, &iov_r, 1, &iov_w, 1);
				break;
			}
			struct v4l2_capability cap;
			memset(&cap, 0, sizeof(cap));
			ret = hdmirx_drv_querycap(phdmirx_obj, &cap);

			fuse_reply_ioctl(req, ret, &cap, sizeof(struct v4l2_capability));
			break;
		default:
			fuse_reply_err(req, EINVAL);
			break;
	}
	pthread_mutex_unlock(&hdmirx_mutex);
}

static struct cuse_lowlevel_ops hdmirx_cuse_ops = {
	.open		= hdmirx_cuse_open,
	.read		= hdmirx_cuse_read,
	.write		= hdmirx_cuse_write,
	.release	= hdmirx_cuse_release,
	.ioctl		= hdmirx_cuse_ioctl,
};

struct hdmirx_obj_s hdmirx_obj = {
	.obj_name   = "hdmirx",
	.hdmirx0_drv_fd = -1,
};

struct cuse_obj hdmirx_cuse = {
	.dev_name 	= "DEVNAME=video12",
	.p_cuse_ops = &hdmirx_cuse_ops,
	.pri        = &hdmirx_obj,
};

#ifdef __cplusplus
}
#endif
