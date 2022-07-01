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
    struct iovec iov_r[3], iov_w[2], iov, iov_ret[2];
    struct v4l2_ext_controls *ext_controls;
    struct v4l2_ext_control *ext_control;
    struct v4l2_ext_avd_timing_info avd_timing_info;
    struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
    struct hdmirx_obj_s *phdmirx_obj = (struct hdmirx_obj_s *)(pcuse_obj->pri);

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

    //fetch write address
    iov_w[0].iov_base = ext_control->ptr;

    switch (ext_control->id) {
		case V4L2_CID_EXT_HDMI_TIMING_INFO:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_timing_info)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_timing_info));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_drm_info
			struct v4l2_ext_hdmi_timing_info *in_timing;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_timing_info);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_timing = (struct v4l2_ext_hdmi_timing_info *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_timing_info);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_timing_info);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_timing_info);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_timing_info info;
			memset(&info, 0, sizeof(info));
			info.port = in_timing->port;
			ret = hdmirx_drv_g_ext_timing_info(phdmirx_obj, &info);
			//reply result
			iov.iov_base = &info;
			iov.iov_len  = sizeof(info);
			break;
		case V4L2_CID_EXT_HDMI_DRM_INFO:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_drm_info)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_drm_info));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_drm_info
			struct v4l2_ext_hdmi_drm_info *in_drm;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_drm_info);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_drm = (struct v4l2_ext_hdmi_drm_info *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_drm_info);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_drm_info);
			//fetch write address
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_drm_info);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_drm_info drm_info;
			memset(&drm_info, 0, sizeof(drm_info));
			drm_info.port = in_drm->port;
			ret = hdmirx_drv_g_ext_drm_info(phdmirx_obj, &drm_info);
			//reply result
			iov.iov_base = &drm_info;
			iov.iov_len = sizeof(drm_info);
			break;
		case V4L2_CID_EXT_HDMI_VSI_INFO:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_vsi_info)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_vsi_info));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_vsi_info
			struct v4l2_ext_hdmi_vsi_info *in_vsi;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_vsi_info);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_vsi = (struct v4l2_ext_hdmi_vsi_info *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_vsi_info);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_vsi_info);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_vsi_info);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_vsi_info vsi_info;
			memset(&vsi_info, 0, sizeof(vsi_info));
			vsi_info.port = in_vsi->port;
			ret = hdmirx_drv_g_ext_vsi_info(phdmirx_obj, &vsi_info);
			//reply result
			iov.iov_base = &vsi_info;
			iov.iov_len = sizeof(vsi_info);
			break;
		case V4L2_CID_EXT_HDMI_SPD_INFO:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_spd_info)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_spd_info));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_spd_info
			struct v4l2_ext_hdmi_spd_info *in_spd;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_spd_info);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_spd = (struct v4l2_ext_hdmi_spd_info *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_spd_info);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_spd_info);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_spd_info);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_spd_info spd_info;
			memset(&spd_info, 0, sizeof(spd_info));
			spd_info.port = in_spd->port;
			ret = hdmirx_drv_g_ext_spd_info(phdmirx_obj, &spd_info);
			//reply result
			iov.iov_base = &spd_info;
			iov.iov_len = sizeof(spd_info);
			break;
		case V4L2_CID_EXT_HDMI_AVI_INFO:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_avi_info)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_avi_info));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_avi_info
			struct v4l2_ext_hdmi_avi_info *in_avi;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_avi_info);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_avi = (struct v4l2_ext_hdmi_avi_info *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_avi_info);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_avi_info);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_avi_info);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_avi_info avi_info;
			memset(&avi_info, 0, sizeof(avi_info));
			avi_info.port = in_avi->port;
			ret = hdmirx_drv_g_ext_avi_info(phdmirx_obj, &avi_info);
			//reply result
			iov.iov_base = &avi_info;
			iov.iov_len = sizeof(avi_info);
			break;
		case V4L2_CID_EXT_HDMI_PACKET_INFO:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_packet_info)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_packet_info));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_packet_info
			struct v4l2_ext_hdmi_packet_info *in_packet;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_packet_info);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_packet = (struct v4l2_ext_hdmi_packet_info *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_packet_info);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_packet_info);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_packet_info);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_packet_info packet_info;
			memset(&packet_info, 0, sizeof(packet_info));
			packet_info.port = in_packet->port;
			ret = hdmirx_drv_g_ext_packet_info(phdmirx_obj, &packet_info);
			//reply result
			iov.iov_base = &packet_info;
			iov.iov_len = sizeof(packet_info);
			break;
		case V4L2_CID_EXT_HDMI_DOLBY_HDR:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_dolby_hdr)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_dolby_hdr));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_dolby_hdr
			struct v4l2_ext_hdmi_dolby_hdr *in_hdmi_dolby_hdr;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_dolby_hdr);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_hdmi_dolby_hdr = (struct v4l2_ext_hdmi_dolby_hdr *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_dolby_hdr);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_dolby_hdr);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_dolby_hdr);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_dolby_hdr dolby_hdr;
			memset(&dolby_hdr, 0, sizeof(dolby_hdr));
			dolby_hdr.port = in_hdmi_dolby_hdr->port;
			ret = hdmirx_drv_g_ext_dolby_hdr(phdmirx_obj, &dolby_hdr);
			//reply result
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
			LOGD("%d, out_bufsz:%d in_bufsz:%d port:%d size:%d\n", __LINE__, out_bufsz, in_bufsz, pedid->port, pedid->size);
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
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_connection_state)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_connection_state));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_hpd
			struct v4l2_ext_hdmi_connection_state *in_connect_state;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_connection_state);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_connect_state = (struct v4l2_ext_hdmi_connection_state *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_connection_state);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_connection_state);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_connection_state);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_connection_state connection_state;
			memset(&connection_state, 0, sizeof(connection_state));
			connection_state.port = in_connect_state->port;
			ret = hdmirx_drv_g_ext_connection_state(phdmirx_obj, &connection_state);
			//reply result
			iov.iov_base = &connection_state;
			iov.iov_len = sizeof(connection_state);
			break;
		case V4L2_CID_EXT_HDMI_HPD:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_hpd)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_hpd));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_hpd
			struct v4l2_ext_hdmi_hpd *in_hpd;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_hpd);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_hpd = (struct v4l2_ext_hdmi_hpd *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_hpd);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_hpd);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_hpd);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_hpd hpd;
			memset(&hpd, 0, sizeof(hpd));
			hpd.port = in_hpd->port;
			ret = hdmirx_drv_g_ext_hpd(phdmirx_obj, &hpd);
			//reply result
			iov.iov_base = &hpd;
			iov.iov_len = sizeof(hpd);
			break;
		case V4L2_CID_EXT_HDMI_VRR_FREQUENCY:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_vrr_frequency)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_vrr_frequency));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_vrr_frequency
			struct v4l2_ext_hdmi_vrr_frequency *in_vrr;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_vrr_frequency);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_vrr = (struct v4l2_ext_hdmi_vrr_frequency *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_vrr_frequency);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_vrr_frequency);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_vrr_frequency);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_vrr_frequency vrr_frequency;
			memset(&vrr_frequency, 0, sizeof(vrr_frequency));
			vrr_frequency.port = in_vrr->port;
			ret = hdmirx_drv_g_ext_vrr_frequency(phdmirx_obj, &vrr_frequency);
			//reply result
			iov.iov_base = &vrr_frequency;
			iov.iov_len = sizeof(vrr_frequency);
			break;
		case V4L2_CID_EXT_HDMI_EMP_INFO:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_emp_info)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_emp_info));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_emp_info
			struct v4l2_ext_hdmi_emp_info *in_emp;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_emp_info);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_emp = (struct v4l2_ext_hdmi_emp_info *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_emp_info);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_emp_info);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_emp_info);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_emp_info emp_info;
			memset(&emp_info, 0, sizeof(emp_info));
			emp_info.port = in_emp->port;
			emp_info.type = in_emp->type;
			emp_info.current_packet_index = in_emp->current_packet_index;
			ret = hdmirx_drv_g_ext_emp_info(phdmirx_obj, &emp_info);
			//reply result
			iov.iov_base = &emp_info;
			iov.iov_len = sizeof(emp_info);
			break;
		case V4L2_CID_EXT_HDMI_HDCP_REPEATER:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_hdcp_repeater)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_hdcp_repeater));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_hdcp_repeater
			struct v4l2_ext_hdmi_hdcp_repeater *in_hdcp_repeater;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_hdcp_repeater);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_hdcp_repeater = (struct v4l2_ext_hdmi_hdcp_repeater *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_hdcp_repeater);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_hdcp_repeater);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_hdcp_repeater);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_hdcp_repeater hdcp_repeater;
			memset(&hdcp_repeater, 0, sizeof(hdcp_repeater));
			hdcp_repeater.port = in_hdcp_repeater->port;
			ret = hdmirx_drv_g_ext_hdcp_repeater(phdmirx_obj, &hdcp_repeater);
			//reply result
			iov.iov_base = &hdcp_repeater;
			iov.iov_len = sizeof(hdcp_repeater);
			break;
		case V4L2_CID_EXT_HDMI_HDCP_REPEATER_STREAM_MANAGE:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_hdcp_repeater_stream_manage)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_hdcp_repeater_stream_manage));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_hdcp_repeater_stream_manage
			struct v4l2_ext_hdmi_hdcp_repeater_stream_manage *in_stream_manage;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_hdcp_repeater_stream_manage);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_stream_manage = (struct v4l2_ext_hdmi_hdcp_repeater_stream_manage *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_hdcp_repeater_stream_manage);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_hdcp_repeater_stream_manage);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_hdcp_repeater_stream_manage);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_hdcp_repeater_stream_manage stream_manage;
			memset(&stream_manage, 0, sizeof(stream_manage));
			stream_manage.port = in_stream_manage->port;
			ret = hdmirx_drv_g_ext_stream_manage(phdmirx_obj, &stream_manage);
			//reply result
			iov.iov_base = &stream_manage;
			iov.iov_len = sizeof(stream_manage);
			break;
		case V4L2_CID_EXT_HDMI_QUERYCAP:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_querycap)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_querycap));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_hdcp_repeater_stream_manage
			struct v4l2_ext_hdmi_querycap *in_querycap;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_querycap);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_querycap = (struct v4l2_ext_hdmi_querycap *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_querycap);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_querycap);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_querycap);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_querycap querycap;
			memset(&querycap, 0, sizeof(querycap));
			querycap.port = in_querycap->port;
			ret = hdmirx_drv_g_ext_querycap(phdmirx_obj, &querycap);
			//reply result
			iov.iov_base = &querycap;
			iov.iov_len = sizeof(querycap);
			break;
		case V4L2_CID_EXT_HDMI_HPD_LOW_DURATION_DC_ON:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_hpd_low_duration_dc_on
			struct v4l2_ext_hdmi_hpd_low_duration_dc_on *in_low_duration;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_low_duration = (struct v4l2_ext_hdmi_hpd_low_duration_dc_on *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_hpd_low_duration_dc_on dc_on;
			memset(&dc_on, 0, sizeof(dc_on));
			dc_on.port = in_low_duration->port;
			ret = hdmirx_drv_g_ext_dc_on(phdmirx_obj, &dc_on);
			//reply result
			iov.iov_base = &dc_on;
			iov.iov_len = sizeof(dc_on);
			break;
		case V4L2_CID_EXT_HDMI_SLEEP:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_sleep)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_sleep));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_sleep
			struct v4l2_ext_hdmi_sleep *in_sleep;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_sleep);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_sleep = (struct v4l2_ext_hdmi_sleep *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_sleep);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_sleep);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_sleep);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_sleep sleep_mode;
			memset(&sleep_mode, 0, sizeof(sleep_mode));
			sleep_mode.port = in_sleep->port;
			ret = hdmirx_drv_g_ext_sleep_mode(phdmirx_obj, &sleep_mode);
			//reply result
			iov.iov_base = &sleep_mode;
			iov.iov_len = sizeof(sleep_mode);
			break;
		case V4L2_CID_EXT_HDMI_DIAGNOSTICS_STATUS:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_diagnostics_status)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_diagnostics_status));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_diagnostics_status
			struct v4l2_ext_hdmi_diagnostics_status *in_diagnostics;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_diagnostics_status);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_diagnostics = (struct v4l2_ext_hdmi_diagnostics_status *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_diagnostics_status);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_diagnostics_status);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_diagnostics_status);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_diagnostics_status status;
			memset(&status, 0, sizeof(status));
			status.port = in_diagnostics->port;
			ret = hdmirx_drv_g_ext_diag_status(phdmirx_obj, &status);
			//reply result
			iov.iov_base = &status;
			iov.iov_len = sizeof(status);
			break;
		case V4L2_CID_EXT_HDMI_PHY_STATUS:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_phy_status)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_phy_status));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_phy_status
			struct v4l2_ext_hdmi_phy_status *in_phy_status;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_phy_status);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_phy_status = (struct v4l2_ext_hdmi_phy_status *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_phy_status);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_phy_status);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_phy_status);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_phy_status phy_status;
			memset(&phy_status, 0, sizeof(phy_status));
			phy_status.port = in_phy_status->port;
			ret = hdmirx_drv_g_ext_phy_status(phdmirx_obj, &phy_status);
			//reply result
			iov.iov_base = &phy_status;
			iov.iov_len = sizeof(phy_status);
			break;
		case V4L2_CID_EXT_HDMI_LINK_STATUS:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_link_status)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_link_status));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_link_status
			struct v4l2_ext_hdmi_link_status *in_link;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_link_status);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_link = (struct v4l2_ext_hdmi_link_status *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_link_status);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_link_status);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_link_status);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_link_status link_status;
			memset(&link_status, 0, sizeof(link_status));
			link_status.port = in_link->port;
			ret = hdmirx_drv_g_ext_link_status(phdmirx_obj, &link_status);
			//reply result
			iov.iov_base = &link_status;
			iov.iov_len = sizeof(link_status);
			break;
		case V4L2_CID_EXT_HDMI_VIDEO_STATUS:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_video_status)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_video_status));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_video_status
			struct v4l2_ext_hdmi_video_status *in_video;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_video_status);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_video = (struct v4l2_ext_hdmi_video_status *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_video_status);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_video_status);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_video_status);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_video_status video_status;
			memset(&video_status, 0, sizeof(video_status));
			video_status.port = in_video->port;
			ret = hdmirx_drv_g_ext_video_status(phdmirx_obj, &video_status);
			//reply result
			iov.iov_base = &video_status;
			iov.iov_len = sizeof(video_status);
			break;
		case V4L2_CID_EXT_HDMI_AUDIO_STATUS:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_audio_status)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_audio_status));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_audio_status
			struct v4l2_ext_hdmi_audio_status *in_audio;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_audio_status);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_audio = (struct v4l2_ext_hdmi_audio_status *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_audio_status);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_audio_status);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_audio_status);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_audio_status audio_status;
			memset(&audio_status, 0, sizeof(audio_status));
			audio_status.port = in_audio->port;
			ret = hdmirx_drv_g_ext_audio_status(phdmirx_obj, &audio_status);
			//reply result
			iov.iov_base = &audio_status;
			iov.iov_len = sizeof(audio_status);
			break;
		case V4L2_CID_EXT_HDMI_HDCP_STATUS:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_hdcp_status)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_hdcp_status));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_hdcp_status
			struct v4l2_ext_hdmi_hdcp_status *in_hdcp_status;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_hdcp_status);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_hdcp_status = (struct v4l2_ext_hdmi_hdcp_status *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_hdcp_status);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_hdcp_status);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_hdcp_status);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_hdcp_status hdcp_status;
			memset(&hdcp_status, 0, sizeof(hdcp_status));
			hdcp_status.port = in_hdcp_status->port;
			ret = hdmirx_drv_g_ext_hdcp_status(phdmirx_obj, &hdcp_status);
			//reply result
			iov.iov_base = &hdcp_status;
			iov.iov_len = sizeof(hdcp_status);
			break;
		case V4L2_CID_EXT_HDMI_SCDC_STATUS:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_scdc_status)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_scdc_status));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_scdc_status
			struct v4l2_ext_hdmi_scdc_status *in_scdc;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_scdc_status);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_scdc = (struct v4l2_ext_hdmi_scdc_status *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_scdc_status);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_scdc_status);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_scdc_status);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_scdc_status scdc_status;
			memset(&scdc_status, 0, sizeof(scdc_status));
			scdc_status.port = in_scdc->port;
			ret = hdmirx_drv_g_ext_scdc_status(phdmirx_obj, &scdc_status);
			//reply result
			iov.iov_base = &scdc_status;
			iov.iov_len = sizeof(scdc_status);
			break;
		case V4L2_CID_EXT_HDMI_ERROR_STATUS:
			//check size is right or not
			if (ext_control->size != sizeof(struct v4l2_ext_hdmi_error_status)) {
				LOGE("%d, ext_control->size:%d size:%d\n", __LINE__, ext_control->size, sizeof(struct v4l2_ext_hdmi_error_status));
				fuse_reply_err(req, EINVAL);
				return;
			}
			//fetch v4l2_ext_hdmi_error_status
			struct v4l2_ext_hdmi_error_status *in_error;
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len  = sizeof(struct v4l2_ext_hdmi_error_status);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			in_error = (struct v4l2_ext_hdmi_error_status *)in_buf;
			in_buf   += sizeof(struct v4l2_ext_hdmi_error_status);
			in_bufsz -= sizeof(struct v4l2_ext_hdmi_error_status);
			//fetch write size
			iov_w[0].iov_len = sizeof(struct v4l2_ext_hdmi_error_status);
			if (!out_bufsz) {
			    fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
			    return;
			}
			//get info from driver
			struct v4l2_ext_hdmi_error_status error_status;
			memset(&error_status, 0, sizeof(error_status));
			error_status.port = in_error->port;
			ret = hdmirx_drv_g_ext_error_status(phdmirx_obj, &error_status);
			//reply result
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
