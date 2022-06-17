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

#define HDCP14_KEY_SIZE 288
#define HDCP22_KEY_SIZE 288 //i am not sure

struct hdmirx_obj_s {
	const char		*obj_name;
	int 			hdmirx0_drv_fd;
	unsigned char 	open_cnt;
	int 			hwport;
	VdinDrvIf *mp_VdinDrvIf;
	tv_source_input_t source_input;
	struct v4l2_ext_hdmi_timing_info timing_info;
	struct v4l2_ext_hdmi_drm_info drm_info;
	struct v4l2_ext_hdmi_vsi_info vsi_info;
	struct v4l2_ext_hdmi_spd_info spd_info;
	struct v4l2_ext_hdmi_avi_info avi_info;
	struct v4l2_ext_hdmi_packet_info packet_info;
	struct v4l2_ext_hdmi_dolby_hdr dolby_hdr;
	struct v4l2_ext_hdmi_edid edid;
	struct v4l2_ext_hdmi_connection_state connection_state;
	struct v4l2_ext_hdmi_hpd hpd;
	struct v4l2_ext_hdmi_hdcp_key hdcp_key;
	struct v4l2_ext_hdmi_vrr_frequency vrr_frequency;
	struct v4l2_ext_hdmi_emp_info emp_info;
	struct v4l2_ext_hdmi_hdcp_repeater hdcp_repeater;
	struct v4l2_ext_hdmi_querycap querycap;
	struct v4l2_ext_hdmi_hpd_low_duration_dc_on dc_on;
	struct v4l2_ext_hdmi_sleep sleep_mode;
	struct v4l2_ext_hdmi_diagnostics_status diag_status;
	struct v4l2_ext_hdmi_phy_status phy_status;
	struct v4l2_ext_hdmi_link_status link_status;
	struct v4l2_ext_hdmi_video_status video_status;
	//struct v4l2_ext_hdmi_audio_status audio_status;
	struct v4l2_ext_hdmi_hdcp_status hdcp_status;
	struct v4l2_ext_hdmi_scdc_status scdc_status;
	struct v4l2_ext_hdmi_error_status error_status;
	v4l2_ext_hdmi_override_eotf eotf;
	struct v4l2_ext_hdmi_hdcp_repeater_topology topology;
	struct v4l2_ext_hdmi_hdcp_repeater_stream_manage stream_manage;
	struct v4l2_ext_hdmi_expert_setting expert_setting;
};

int hdmirx_drv_open(struct hdmirx_obj_s *phdmirx_obj, int flags);
int hdmirx_drv_close(struct hdmirx_obj_s *phdmirx_obj);
int hdmirx_drv_querycap(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_capability *cap);
int hdmirx_drv_g_input(struct hdmirx_obj_s *phdmirx_obj);
int hdmirx_drv_s_input(struct hdmirx_obj_s *phdmirx_obj, int hwport);

int hdmirx_drv_s_power_off(struct hdmirx_obj_s *pvbe_obj, int onoff);
int hdmirx_drv_s_disconnect(struct hdmirx_obj_s *pvbe_obj, int port);

int hdmirx_drv_g_ext_timing_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_timing_info *info);
int hdmirx_drv_g_ext_drm_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_drm_info *info);
int hdmirx_drv_g_ext_vsi_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_vsi_info *info);
int hdmirx_drv_g_ext_spd_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_spd_info *info);
int hdmirx_drv_g_ext_avi_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_avi_info *info);
int hdmirx_drv_g_ext_packet_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_packet_info *info);
int hdmirx_drv_g_ext_dolby_hdr(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_dolby_hdr *dolby_hdr);
int hdmirx_drv_g_ext_edid(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_edid *edid);
int hdmirx_drv_g_ext_connection_state(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_connection_state *state);
int hdmirx_drv_g_ext_hpd(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hpd *hpd);
int hdmirx_drv_g_ext_vrr_frequency(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_vrr_frequency *vrr_frequency);
int hdmirx_drv_g_ext_emp_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_emp_info *emp_info);
int hdmirx_drv_g_ext_hdcp_repeater(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_repeater *hdcp_repeater);
int hdmirx_drv_g_ext_stream_manage(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_repeater_stream_manage *stream_manage);
int hdmirx_drv_g_ext_error_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_error_status *status);
int hdmirx_drv_g_ext_scdc_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_scdc_status *status);
int hdmirx_drv_g_ext_hdcp_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_status *status);
int hdmirx_drv_g_ext_audio_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_audio_status *status);
int hdmirx_drv_g_ext_link_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_link_status *status);
int hdmirx_drv_g_ext_phy_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_phy_status *status);
int hdmirx_drv_g_ext_diag_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_diagnostics_status *status);
int hdmirx_drv_g_ext_sleep_mode(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_sleep *sleep_mode);
int hdmirx_drv_g_ext_querycap(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_querycap *querycap);
int hdmirx_drv_g_ext_video_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_video_status *status);
int hdmirx_drv_g_ext_dc_on(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hpd_low_duration_dc_on *dc_on);

int hdmirx_drv_s_ext_edid(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_edid *edid);
int hdmirx_drv_s_ext_hpd(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hpd *hpd);
int hdmirx_drv_s_ext_hdcp_key(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_key *hdcp_key);
int hdmirx_drv_s_ext_hdcp_repeater(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_repeater *hdcp_repeater);
int hdmirx_drv_s_ext_dc_on(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hpd_low_duration_dc_on *dc_on);
int hdmirx_drv_s_ext_sleep(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_sleep *sleep_mode);
int hdmirx_drv_s_ext_expert_setting(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_expert_setting *expert_setting);
int hdmirx_drv_s_ext_override_eotf(struct hdmirx_obj_s *phdmirx_obj, v4l2_ext_hdmi_override_eotf eotf);
int hdmirx_drv_s_ext_repeater_topology(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_repeater_topology *topology);

#ifdef __cplusplus
}
#endif

#endif
