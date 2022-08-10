/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_MOUDLE_TAG "TVIN-HDMIRX"
#define LOG_CLASS_TAG "HDMIRX_DRV"

#include "../common.h"
#include "hdmirx_fuse.h"
#include "hdmirx_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

int hdmirx_drv_open(struct hdmirx_obj_s *phdmirx_obj, int flags)
{
    //open vdin
    if (phdmirx_obj->mp_VdinDrvIf == NULL) {
        phdmirx_obj->mp_VdinDrvIf = VdinDrvIf::GetInstance();
    }

    //open hdmirx devices
    if (phdmirx_obj->hdmirx0_drv_fd == -1) {
        phdmirx_obj->hdmirx0_drv_fd = open(HDMI_DETECT_PATH, flags, 0);
        if (phdmirx_obj->hdmirx0_drv_fd < 0) {
            LOGE("Unable to open %s, err:%s\n", HDMI_DETECT_PATH,
                strerror(errno));
            return errno;
        }
    }
    phdmirx_obj->open_cnt++;

    LOGD("%s, open_cnt=%d\n",__func__, phdmirx_obj->open_cnt);

    return 0;
}

int hdmirx_drv_close(struct hdmirx_obj_s *phdmirx_obj)
{
    LOGD("%s, open_cnt:%d\n", __func__, phdmirx_obj->open_cnt);
    phdmirx_obj->open_cnt--;
    if (phdmirx_obj->open_cnt == 0) {
        if (phdmirx_obj->hdmirx0_drv_fd > 0) {
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

    LOGD("%s, hwport=%d\n", __func__, hwport);

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
    LOGD("%s,  hwport=%d, source_input=%d\n", __func__, hwport, source_input);
    return 0;
}

int hdmirx_drv_g_input(struct hdmirx_obj_s *phdmirx_obj)
{
    LOGD("%s, input=%d\n",__func__, phdmirx_obj->hwport);

    return phdmirx_obj->hwport;
}

int hdmirx_drv_querycap(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_capability *cap)
{
    memset(cap, 0, sizeof(struct v4l2_capability));

    strncpy((char *)&cap->driver[0], "other_dvb_eu", strlen("other_dvb_eu"));
    strncpy((char *)&cap->card[0], phdmirx_obj->obj_name, strlen(phdmirx_obj->obj_name));
    cap->version = 0x20220429;/* KERNEL_VERSION */
    cap->capabilities = (V4L2_CAP_VIDEO_CAPTURE_MPLANE | V4L2_CAP_STREAMING | V4L2_CAP_READWRITE);
    cap->device_caps = (V4L2_CAP_VIDEO_CAPTURE_MPLANE | V4L2_CAP_STREAMING | V4L2_CAP_READWRITE);

    LOGD("%s, version: %d\n", __func__, cap->version);

    return 0;
}

/* V4L2_CID_EXT_HDMI_POWER_OFF */
int hdmirx_drv_s_power_off(struct hdmirx_obj_s *phdmirx_obj, int onoff)
{
    int ret = 0;
    int power_off = onoff;

    LOGD("%s, power off:%d\n", __FUNCTION__, onoff);

    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_SET_HDMI_POWER_OFF, &power_off);
    if (ret < 0) {
        LOGE("%s error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    return 0;
}

/* V4L2_CID_EXT_HDMI_DISCONNECT */
int hdmirx_drv_s_disconnect(struct hdmirx_obj_s *phdmirx_obj, int port)
{
    int ret = 0;
    int hdmirx_port = port;

    LOGD("%s, port:%d\n", __FUNCTION__, hdmirx_port);

    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_SET_HDMI_DISCONNECT, &hdmirx_port);
    if (ret < 0) {
        LOGE("%s error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    return 0;
}

/* V4L2_CID_EXT_HDMI_TIMING_INFO */
int hdmirx_drv_g_ext_timing_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_timing_info *info)
{
    int ret = 0;
    struct tvin_info_s SignalInfo;
    struct tvin_format_s Format;

    if (!info) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, info->port);
    }

    memset(&SignalInfo, 0, sizeof(SignalInfo));
    memset(&Format, 0, sizeof(Format));

    ret = phdmirx_obj->mp_VdinDrvIf->DeviceIOCtl(TVIN_IOC_G_SIG_INFO, &SignalInfo);
    if (ret < 0) {
        LOGE("%s TVIN_IOC_G_SIG_INFO error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("trans_fmt:%d,fmt:%#x,status:%d,cfmt:%d,fps:%d,is_dvi:%d,hdr_info:%x,ratio:%d\n",
        SignalInfo.trans_fmt, SignalInfo.fmt, SignalInfo.status,
        SignalInfo.cfmt, SignalInfo.fps, SignalInfo.is_dvi,
        SignalInfo.hdr_info, SignalInfo.aspect_ratio);

    ret = phdmirx_obj->mp_VdinDrvIf->DeviceIOCtl(TVIN_IOC_G_INPUT_TIMING, &Format);
    if (ret < 0) {
        LOGE("%s TVIN_IOC_G_INPUT_TIMING error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("active:%dx%d,h_cnt:%d-%d-%d,hs_cnt:%d-%d,total:%dx%d,hs:%d %d %d,vs:%d %d %d\n",
        Format.h_active, Format.v_active, Format.h_cnt, Format.h_cnt_offset, Format.v_cnt_offset,
        Format.hs_cnt, Format.hs_cnt_offset,Format.h_total, Format.v_total,Format.hs_front,
        Format.hs_width,Format.hs_bp,Format.vs_front,Format.vs_width, Format.vs_bp);
    LOGD("pol:%d %d,scan:%d,clk:%d,vbi:%d %d,duration:%d\n",
        Format.hs_pol, Format.vs_pol, Format.scan_mode, Format.pixel_clk,
        Format.vbi_line_start, Format.vbi_line_end, Format.duration);

    struct v4l2_ext_hdmi_timing_info hdmi_timing_info;
    memset(&hdmi_timing_info, 0, sizeof(v4l2_ext_hdmi_timing_info));
    hdmi_timing_info.port = info->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_TIMING_INFO, &hdmi_timing_info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_TIMING_INFO error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }
    LOGD("%s, port:%d h_freq:%d v_vreq:%d "
        "h_total:%d v_total:%d h_porch:%d v_porch:%d "
        "active.x:%d active.y:%d active.h:%d active.w:%d "
        "scan_type:%d dvi_hdmi:%d color_depth:%d allm_mode:%d\n",
        __FUNCTION__, hdmi_timing_info.port,
        hdmi_timing_info.h_freq, hdmi_timing_info.v_vreq,
        hdmi_timing_info.h_total, hdmi_timing_info.v_total,
        hdmi_timing_info.h_porch, hdmi_timing_info.v_porch,
        hdmi_timing_info.active.x, hdmi_timing_info.active.y, hdmi_timing_info.active.h, hdmi_timing_info.active.w,
        hdmi_timing_info.scan_type, hdmi_timing_info.dvi_hdmi,
        hdmi_timing_info.color_depth, hdmi_timing_info.allm_mode);

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
    info->dvi_hdmi    = hdmi_timing_info.dvi_hdmi;
    info->color_depth = hdmi_timing_info.color_depth;
    info->allm_mode   = hdmi_timing_info.allm_mode;

    memcpy(&phdmirx_obj->timing_info, info, sizeof(struct v4l2_ext_hdmi_timing_info));

    return 0;
}

/* V4L2_CID_EXT_HDMI_DRM_INFO */
int hdmirx_drv_g_ext_drm_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_drm_info *info)
{
    if (!info) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, info->port);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_drm_info hdmi_drm_info;
    memset(&hdmi_drm_info, 0, sizeof(v4l2_ext_hdmi_drm_info));
    hdmi_drm_info.port = info->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_DRM_INFO, &hdmi_drm_info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_DRM_INFO error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d version:%d length:%d"
        "eotf_type:%d meta_desc:%d display_primaries_x0:%d display_primaries_y0:%d"
        "display_primaries_x1:%d display_primaries_y1:%d display_primaries_x2:%d display_primaries_y2:%d"
        "white_point_x:%d white_point_y:%d"
        "max_display_mastering_luminance:%d min_display_mastering_luminance:%d"
        "maximum_content_light_level:%d maximum_frame_average_light_level:%d\n",
        __FUNCTION__, hdmi_drm_info.port,
        hdmi_drm_info.version, hdmi_drm_info.length,
        hdmi_drm_info.eotf_type, hdmi_drm_info.meta_desc,
        hdmi_drm_info.display_primaries_x0, hdmi_drm_info.display_primaries_y0,
        hdmi_drm_info.display_primaries_x1, hdmi_drm_info.display_primaries_y1,
        hdmi_drm_info.display_primaries_x2, hdmi_drm_info.display_primaries_y2,
        hdmi_drm_info.white_point_x, hdmi_drm_info.white_point_y,
        hdmi_drm_info.max_display_mastering_luminance, hdmi_drm_info.min_display_mastering_luminance,
        hdmi_drm_info.maximum_content_light_level, hdmi_drm_info.maximum_frame_average_light_level);

    info->version = hdmi_drm_info.version;
    info->length  = hdmi_drm_info.length;
    info->eotf_type = hdmi_drm_info.eotf_type;
    info->meta_desc = hdmi_drm_info.meta_desc;
    info->display_primaries_x0 = hdmi_drm_info.display_primaries_x0;
    info->display_primaries_y0 = hdmi_drm_info.display_primaries_y0;
    info->display_primaries_x1 = hdmi_drm_info.display_primaries_x1;
    info->display_primaries_y1 = hdmi_drm_info.display_primaries_y1;
    info->display_primaries_x2 = hdmi_drm_info.display_primaries_y2;
    info->white_point_x        = hdmi_drm_info.white_point_x;
    info->white_point_y        = hdmi_drm_info.white_point_y;
    info->max_display_mastering_luminance = hdmi_drm_info.max_display_mastering_luminance;
    info->min_display_mastering_luminance = hdmi_drm_info.min_display_mastering_luminance;
    info->maximum_content_light_level     = hdmi_drm_info.maximum_content_light_level;
    info->maximum_frame_average_light_level = hdmi_drm_info.maximum_frame_average_light_level;

    memcpy(&phdmirx_obj->drm_info, info, sizeof(struct v4l2_ext_hdmi_drm_info));
    return 0;
}

/* V4L2_CID_EXT_HDMI_VSI_INFO */
int hdmirx_drv_g_ext_vsi_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_vsi_info *info)
{
    if (!info) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, info->port);
    }

    int ret = 0;
    int i = 0;
    struct v4l2_ext_hdmi_vsi_info hdmi_vsi_info;
    memset(&hdmi_vsi_info, 0, sizeof(v4l2_ext_hdmi_vsi_info));
    hdmi_vsi_info.port = info->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_VSI_INFO, &hdmi_vsi_info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_VSI_INFO error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d packet_status:%d"
        "video_format:%d st_3d:%d ext_data_3d:%d vic:%d\n",
        __FUNCTION__, hdmi_vsi_info.port, hdmi_vsi_info.packet_status,
        hdmi_vsi_info.video_format, hdmi_vsi_info.st_3d,
        hdmi_vsi_info.ext_data_3d, hdmi_vsi_info.vic);

    info->packet_status = hdmi_vsi_info.packet_status;
    info->video_format = hdmi_vsi_info.video_format;
    info->st_3d  = hdmi_vsi_info.st_3d;
    info->ext_data_3d = hdmi_vsi_info.ext_data_3d;
    info->vic = hdmi_vsi_info.vic;
    for (i = 0; i < V4L2_EXT_HDMI_VENDOR_SPECIFIC_REGID_LEN; i++) {
        LOGD("%s, regid[%d]:%d\n", __FUNCTION__, i, hdmi_vsi_info.regid[i]);
        info->regid[i] = hdmi_vsi_info.regid[i];
    }
    for (i = 0; i < V4L2_EXT_HDMI_VENDOR_SPECIFIC_PAYLOAD_LEN; i++) {
        LOGD("%s, payload[%d]:%d\n", __FUNCTION__, i, hdmi_vsi_info.payload[i]);
        info->payload[i] = hdmi_vsi_info.payload[i];
    }

    LOGD("%s, packet.type:%d packet.version:%d packet.length:%d\n",
        __FUNCTION__,
        hdmi_vsi_info.packet.type, hdmi_vsi_info.packet.version,
        hdmi_vsi_info.packet.length);

    info->packet.type    = hdmi_vsi_info.packet.type;
    info->packet.version = hdmi_vsi_info.packet.version;
    info->packet.length  = hdmi_vsi_info.packet.length;

    for (i = 0; i < V4L2_EXT_HDMI_PACKET_DATA_LENGTH; i++) {
        LOGD("%s, packet.data_bytes[%d]:%d\n", __FUNCTION__, i, hdmi_vsi_info.packet.data_bytes[i]);
        info->packet.data_bytes[i] = hdmi_vsi_info.packet.data_bytes[i];
    }

    memcpy(&phdmirx_obj->vsi_info, info, sizeof(struct v4l2_ext_hdmi_vsi_info));
    return 0;
}

/* V4L2_CID_EXT_HDMI_SPD_INFO */
int hdmirx_drv_g_ext_spd_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_spd_info *info)
{
    if (!info) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, info->port);
    }

    int ret = 0;
    int i = 0;
    struct v4l2_ext_hdmi_spd_info hdmi_spd_info;
    memset(&hdmi_spd_info, 0, sizeof(v4l2_ext_hdmi_spd_info));
    hdmi_spd_info.port = info->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_SPD_INFO, &hdmi_spd_info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_SPD_INFO error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d packet_status:%d source_device_info:%d\n",
        __FUNCTION__, hdmi_spd_info.port,
        hdmi_spd_info.packet_status, hdmi_spd_info.source_device_info);

    info->packet_status = hdmi_spd_info.packet_status;
    info->source_device_info  = hdmi_spd_info.source_device_info;
    for (i = 0; i < V4L2_EXT_HDMI_SPD_IF_VENDOR_LEN + 1; i++) {
        LOGD("%s, vendor_name[%d]:%d\n", __FUNCTION__, i, hdmi_spd_info.vendor_name[i]);
        info->vendor_name[i] = hdmi_spd_info.vendor_name[i];
    }
    for (i = 0; i < V4L2_EXT_HDMI_SPD_IF_DESC_LEN + 1; i++) {
        LOGD("%s, product_description[%d]:%d\n", __FUNCTION__, i, hdmi_spd_info.product_description[i]);
        info->product_description[i] = hdmi_spd_info.product_description[i];
    }

    LOGD("%s, packet.type:%d packet.version:%d packet.length:%d\n",
        __FUNCTION__,
        hdmi_spd_info.packet.type, hdmi_spd_info.packet.version,
        hdmi_spd_info.packet.length);

    info->packet.type    = hdmi_spd_info.packet.type;
    info->packet.version = hdmi_spd_info.packet.version;
    info->packet.length  = hdmi_spd_info.packet.length;

    for (i = 0; i < V4L2_EXT_HDMI_PACKET_DATA_LENGTH; i++) {
        LOGD("%s, packet.data_bytes[%d]:%d\n", __FUNCTION__, i, hdmi_spd_info.packet.data_bytes[i]);
        info->packet.data_bytes[i] = hdmi_spd_info.packet.data_bytes[i];
    }

    memcpy(&phdmirx_obj->spd_info, info, sizeof(struct v4l2_ext_hdmi_spd_info));
    return 0;
}

/* V4L2_CID_EXT_HDMI_AVI_INFO */
int hdmirx_drv_g_ext_avi_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_avi_info *info)
{
    if (!info) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, info->port);
    }

    int ret = 0;
    int i = 0;
    struct v4l2_ext_hdmi_avi_info hdmi_avi_info;
    memset(&hdmi_avi_info, 0, sizeof(v4l2_ext_hdmi_avi_info));
    hdmi_avi_info.port = info->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_AVI_INFO, &hdmi_avi_info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_AVI_INFO error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d"
         "packet_status:%d mode:%d pixel_encoding:%d"
         "active_info:%d bar_info:%d scan_info:%d"
         "colorimetry:%d picture_aspect_ratio:%d scaling:%d"
         "vic:%d pixel_repeat:%d it_content:%d extended_colorimetry:%d"
         "rgb_quantization_range:%d ycc_quantization_range:%d"
         "content_type:%d bar_info:%d additional_colorimetry:%d"
         "top_bar_end_line_number:%d bottom_bar_start_line_number:%d"
         "left_bar_end_pixel_number:%d bar_info:%d right_bar_end_pixel_number:%d",
        __FUNCTION__, hdmi_avi_info.port,
        hdmi_avi_info.packet_status, hdmi_avi_info.mode, hdmi_avi_info.pixel_encoding,
        hdmi_avi_info.active_info, hdmi_avi_info.bar_info, hdmi_avi_info.scan_info,
        hdmi_avi_info.colorimetry, hdmi_avi_info.picture_aspect_ratio, hdmi_avi_info.scaling,
        hdmi_avi_info.vic, hdmi_avi_info.pixel_repeat, hdmi_avi_info.it_content, hdmi_avi_info.extended_colorimetry,
        hdmi_avi_info.rgb_quantization_range, hdmi_avi_info.ycc_quantization_range,
        hdmi_avi_info.content_type, hdmi_avi_info.additional_colorimetry,
        hdmi_avi_info.top_bar_end_line_number, hdmi_avi_info.bottom_bar_start_line_number,
        hdmi_avi_info.left_bar_end_pixel_number, hdmi_avi_info.right_bar_end_pixel_number);

    info->packet_status  = hdmi_avi_info.packet_status;
    info->mode           = hdmi_avi_info.mode;
    info->pixel_encoding = hdmi_avi_info.pixel_encoding;
    info->active_info = hdmi_avi_info.active_info;
    info->bar_info    = hdmi_avi_info.bar_info;
    info->scan_info   = hdmi_avi_info.scan_info;
    info->colorimetry = hdmi_avi_info.colorimetry;
    info->picture_aspect_ratio  = hdmi_avi_info.picture_aspect_ratio;
    info->scaling      = hdmi_avi_info.scaling;
    info->vic          = hdmi_avi_info.vic;
    info->pixel_repeat = hdmi_avi_info.pixel_repeat;
    info->it_content   = hdmi_avi_info.it_content;
    info->extended_colorimetry   = hdmi_avi_info.extended_colorimetry;
    info->rgb_quantization_range = hdmi_avi_info.rgb_quantization_range;
    info->ycc_quantization_range = hdmi_avi_info.ycc_quantization_range;
    info->content_type           = hdmi_avi_info.content_type;
    info->additional_colorimetry = hdmi_avi_info.additional_colorimetry;
    info->top_bar_end_line_number      = hdmi_avi_info.top_bar_end_line_number;
    info->bottom_bar_start_line_number = hdmi_avi_info.bottom_bar_start_line_number;
    info->left_bar_end_pixel_number    = hdmi_avi_info.left_bar_end_pixel_number;
    info->right_bar_end_pixel_number   = hdmi_avi_info.right_bar_end_pixel_number;

    LOGD("%s, packet.type:%d packet.version:%d packet.length:%d\n",
        __FUNCTION__,
        hdmi_avi_info.packet.type, hdmi_avi_info.packet.version,
        hdmi_avi_info.packet.length);

    info->packet.type    = hdmi_avi_info.packet.type;
    info->packet.version = hdmi_avi_info.packet.version;
    info->packet.length  = hdmi_avi_info.packet.length;

    for (i = 0; i < V4L2_EXT_HDMI_PACKET_DATA_LENGTH; i++) {
        LOGD("%s, packet.data_bytes[%d]:%d\n", __FUNCTION__, i, hdmi_avi_info.packet.data_bytes[i]);
        info->packet.data_bytes[i] = hdmi_avi_info.packet.data_bytes[i];
    }

    memcpy(&phdmirx_obj->spd_info, info, sizeof(struct v4l2_ext_hdmi_avi_info));
    return 0;
}

/* V4L2_CID_EXT_HDMI_PACKET_INFO */
int hdmirx_drv_g_ext_packet_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_packet_info *info)
{
    if (!info) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, info->port);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_packet_info hdmi_packet_info;
    memset(&hdmi_packet_info, 0, sizeof(v4l2_ext_hdmi_packet_info));
    hdmi_packet_info.port = info->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_PACKET_INFO, &hdmi_packet_info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_PACKET_INFO error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    memcpy(info, &hdmi_packet_info, sizeof(struct v4l2_ext_hdmi_packet_info));
    memcpy(&phdmirx_obj->spd_info, info, sizeof(struct v4l2_ext_hdmi_packet_info));
    return 0;
}

/* V4L2_CID_EXT_HDMI_DOLBY_HDR */
int hdmirx_drv_g_ext_dolby_hdr(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_dolby_hdr *dolby_hdr)
{
    if (!dolby_hdr) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, dolby_hdr->port);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_dolby_hdr info;
    memset(&info, 0, sizeof(v4l2_ext_hdmi_dolby_hdr));
    info.port = dolby_hdr->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_DOLBY_HDR_INFO, &info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_DOLBY_HDR_INFO error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, type:%d\n", __FUNCTION__, info.type);

    dolby_hdr->type = info.type;

    memcpy(&phdmirx_obj->dolby_hdr, dolby_hdr, sizeof(struct v4l2_ext_hdmi_dolby_hdr));
    return 0;
}

/* V4L2_CID_EXT_HDMI_EDID */
int hdmirx_drv_g_ext_edid(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_edid *edid)
{
    if (!edid) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d size:%d\n", __FUNCTION__, edid->port, edid->size);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_edid info;
    memset(&info, 0, sizeof(v4l2_ext_hdmi_edid));
    info.port  = edid->port;
    info.size  = edid->size;
    info.pData = new unsigned char[0x200];
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_EDID, &info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_EDID error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    } else {
        edid->pData = info.pData;
    }

    return 0;
}

/* V4L2_CID_EXT_HDMI_CONNECTION_STATE */
int hdmirx_drv_g_ext_connection_state(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_connection_state *state)
{
    if (!state) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, state->port);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_connection_state info;
    memset(&info, 0, sizeof(v4l2_ext_hdmi_connection_state));
    info.port = state->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_CONNETION_STATE, &info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_CONNETION_STATE error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, state:%d\n", __FUNCTION__, info.state);

    state->state = info.state;

    memcpy(&phdmirx_obj->connection_state, state, sizeof(struct v4l2_ext_hdmi_connection_state));
    return 0;
}

/* V4L2_CID_EXT_HDMI_HPD */
int hdmirx_drv_g_ext_hpd(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hpd *hpd)
{
    int ret = 0;
    struct v4l2_ext_hdmi_hpd info;

    if (!hpd) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, hpd->port);
    }

    if ((hpd->port >= V4L2_EXT_HDMI_INPUT_PORT_1) && (hpd->port <= V4L2_EXT_HDMI_INPUT_PORT_4)) {
        memset(&info, 0, sizeof(struct v4l2_ext_hdmi_hpd));
        info.port = hpd->port;
        ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_HPD, &info);
        if (ret < 0) {
            LOGE("%s error: %s\n", __FUNCTION__, strerror(errno));
            return ret;
        }
        LOGD("%s, port:%d hpd:%d\n", __FUNCTION__, info.port, info.hpd_state);
        hpd->hpd_state = info.hpd_state;
        memcpy(&phdmirx_obj->hpd, hpd, sizeof(struct v4l2_ext_hdmi_hpd));
    } else {
        LOGE("%s, input port:%d is invalid\n", __FUNCTION__, hpd->port);
    }

    return 0;
}

/* V4L2_CID_EXT_HDMI_VRR_FREQUENCY */
int hdmirx_drv_g_ext_vrr_frequency(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_vrr_frequency *vrr_frequency)
{
    if (!vrr_frequency) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, vrr_frequency->port);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_vrr_frequency info;
    memset(&info, 0, sizeof(v4l2_ext_hdmi_vrr_frequency));
    info.port = vrr_frequency->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_VRR_FREQUENCY, &info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_VRR_FREQUENCY error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, frequency:%d\n", __FUNCTION__, info.frequency);

    vrr_frequency->frequency = info.frequency;

    memcpy(&phdmirx_obj->vrr_frequency, vrr_frequency, sizeof(struct v4l2_ext_hdmi_vrr_frequency));
    return 0;

}

/* V4L2_CID_EXT_HDMI_EMP_INFO */
int hdmirx_drv_g_ext_emp_info(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_emp_info *emp_info)
{
    if (!emp_info) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, emp_info->port);
    }

    int ret = 0;
    int i = 0;
    struct v4l2_ext_hdmi_emp_info hdmi_emp_info;
    memset(&hdmi_emp_info, 0, sizeof(v4l2_ext_hdmi_emp_info));
    hdmi_emp_info.port = emp_info->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_EMP_INFO, &hdmi_emp_info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_EMP_INFO error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d type:%d current_packet_index:%d total_packet_number:%d\n",
        __FUNCTION__, hdmi_emp_info.port,
        hdmi_emp_info.type, hdmi_emp_info.current_packet_index, hdmi_emp_info.total_packet_number);

    emp_info->type = hdmi_emp_info.type;
    emp_info->current_packet_index = hdmi_emp_info.current_packet_index;
    emp_info->total_packet_number  = hdmi_emp_info.total_packet_number;

    for (i = 0; i < 31; i++) {
        LOGD("%s, data[%d]:%d\n", __FUNCTION__, i, hdmi_emp_info.data[i]);
        emp_info->data[i] = hdmi_emp_info.data[i];
    }

    memcpy(&phdmirx_obj->emp_info, emp_info, sizeof(struct v4l2_ext_hdmi_emp_info));
    return 0;
}

/* V4L2_CID_EXT_HDMI_HDCP_REPEATER */
int hdmirx_drv_g_ext_hdcp_repeater(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_repeater *hdcp_repeater)
{
    if (!hdcp_repeater) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, hdcp_repeater->port);
    }

    int ret = 0;
    int i = 0;
    struct v4l2_ext_hdmi_hdcp_repeater info;
    memset(&info, 0, sizeof(v4l2_ext_hdmi_hdcp_repeater));
    info.port = hdcp_repeater->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_HDCP_REPEATER, &info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_HDCP_REPEATER error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d repeater_mode:%d repeater_hpd:%d\n",
        __FUNCTION__, info.port,
        info.repeater_mode, info.repeater_hpd);

    hdcp_repeater->repeater_mode = info.repeater_mode;
    hdcp_repeater->repeater_hpd = info.repeater_hpd;

    for (i = 0; i < 5; i++) {
        LOGD("%s, receiver_id[%d]:%d\n", __FUNCTION__, i, info.receiver_id[i]);
        hdcp_repeater->receiver_id[i] = info.receiver_id[i];
    }

    memcpy(&phdmirx_obj->hdcp_repeater, hdcp_repeater, sizeof(struct v4l2_ext_hdmi_hdcp_repeater));
    return 0;
}

/* V4L2_CID_EXT_HDMI_HDCP_REPEATER_STREAM_MANAGE */
int hdmirx_drv_g_ext_stream_manage(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_repeater_stream_manage *stream_manage)
{
    if (!stream_manage) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, stream_manage->port);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_hdcp_repeater_stream_manage info;
    memset(&info, 0, sizeof(v4l2_ext_hdmi_hdcp_repeater_stream_manage));
    info.port = stream_manage->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_HDCP_REPEATER_STREAM_MANAGE, &info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_HDCP_REPEATER_STREAM_MANAGE error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d repeater_mode:%d repeater_hpd:%d\n",
        __FUNCTION__, info.port, info.value);

    stream_manage->value = info.value;

    memcpy(&phdmirx_obj->stream_manage, stream_manage, sizeof(struct v4l2_ext_hdmi_hdcp_repeater_stream_manage));
    return 0;
}

/* V4L2_CID_EXT_HDMI_QUERYCAP */
int hdmirx_drv_g_ext_querycap(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_querycap *querycap)
{
    if (!querycap) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, querycap->port);
    }

    int ret = 0;
    int i = 0;
    struct v4l2_ext_hdmi_querycap info;
    memset(&info, 0, sizeof(v4l2_ext_hdmi_querycap));
    info.port = querycap->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_QUERYCAP, &info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_QUERYCAP error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d version:%d\n",
        __FUNCTION__, info.port,
        info.hdmi_capability.version);

    querycap->hdmi_capability.version = info.hdmi_capability.version;

    memcpy(&phdmirx_obj->querycap, querycap, sizeof(struct v4l2_ext_hdmi_querycap));
    return 0;
}

/* V4L2_CID_EXT_HDMI_HPD_LOW_DURATION_DC_ON */
int hdmirx_drv_g_ext_dc_on(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hpd_low_duration_dc_on *dc_on)
{
    if (!dc_on) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, dc_on->port);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_hpd_low_duration_dc_on info;
    memset(&info, 0, sizeof(v4l2_ext_hdmi_hpd_low_duration_dc_on));
    info.port = dc_on->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_HPD_LOW_DURATION_DC_ON, &info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_HPD_LOW_DURATION_DC_ON error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d hpd_low_duration:%d\n",
        __FUNCTION__, info.port, info.hpd_low_duration);

    dc_on->hpd_low_duration = info.hpd_low_duration;

    memcpy(&phdmirx_obj->dc_on, dc_on, sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on));
    return 0;
}

/* V4L2_CID_EXT_HDMI_SLEEP */
int hdmirx_drv_g_ext_sleep_mode(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_sleep *sleep_mode)
{
    if (!sleep_mode) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, sleep_mode->port);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_sleep info;
    memset(&info, 0, sizeof(v4l2_ext_hdmi_sleep));
    info.port = sleep_mode->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_SLEEP, &info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_SLEEP error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d mode:%d\n", __FUNCTION__, info.port, info.mode);

    sleep_mode->mode = info.mode;

    memcpy(&phdmirx_obj->sleep_mode, sleep_mode, sizeof(struct v4l2_ext_hdmi_sleep));
    return 0;

}

/* V4L2_CID_EXT_HDMI_DIAGNOSTICS_STATUS */
int hdmirx_drv_g_ext_diag_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_diagnostics_status *status)
{
    if (!status) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, status->port);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_diagnostics_status diag_status;
    memset(&diag_status, 0, sizeof(v4l2_ext_hdmi_diagnostics_status));
    diag_status.port = status->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_DIAGNOSTICS_STATUS, &diag_status);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_DIAGNOSTICS_STATUS error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    return 0;
}

/* V4L2_CID_EXT_HDMI_PHY_STATUS */
int hdmirx_drv_g_ext_phy_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_phy_status *status)
{
    if (!status) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, status->port);
    }

    int ret = 0;
    int i = 0;
    struct v4l2_ext_hdmi_phy_status hdmi_phy_info;
    memset(&hdmi_phy_info, 0, sizeof(v4l2_ext_hdmi_phy_status));
    hdmi_phy_info.port = status->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_PHY_STATUS, &hdmi_phy_info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_PHY_STATUS error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d lock_status:%d tmds_clk_khz:%d link_type:%d link_lane:%d link_rate:%d\n",
        __FUNCTION__, hdmi_phy_info.port,
        hdmi_phy_info.lock_status, hdmi_phy_info.tmds_clk_khz,
        hdmi_phy_info.link_type, hdmi_phy_info.link_lane, hdmi_phy_info.link_rate);

    status->lock_status  = hdmi_phy_info.lock_status;
    status->tmds_clk_khz = hdmi_phy_info.tmds_clk_khz;
    status->link_type = hdmi_phy_info.link_type;
    status->link_lane = hdmi_phy_info.link_lane;
    status->link_rate = hdmi_phy_info.link_rate;

    for (i = 0; i < V4L2_EXT_HDMI_TMDS_CH_NUM; i++) {
        LOGD("%s, ctle_eq_min_range[%d]:%d\n", __FUNCTION__, i, hdmi_phy_info.ctle_eq_min_range[i]);
        status->ctle_eq_min_range[i] = hdmi_phy_info.ctle_eq_min_range[i];
    }

    for (i = 0; i < V4L2_EXT_HDMI_TMDS_CH_NUM; i++) {
        LOGD("%s, ctle_eq_max_range[%d]:%d\n", __FUNCTION__, i, hdmi_phy_info.ctle_eq_max_range[i]);
        status->ctle_eq_max_range[i] = hdmi_phy_info.ctle_eq_max_range[i];
    }

    for (i = 0; i < V4L2_EXT_HDMI_TMDS_CH_NUM; i++) {
        LOGD("%s, ctle_eq_result[%d]:%d\n", __FUNCTION__, i, hdmi_phy_info.ctle_eq_result[i]);
        status->ctle_eq_result[i] = hdmi_phy_info.ctle_eq_result[i];
    }

    for (i = 0; i < V4L2_EXT_HDMI_TMDS_CH_NUM; i++) {
        LOGD("%s, error[%d]:%d\n", __FUNCTION__, i, hdmi_phy_info.error[i]);
        status->error[i] = hdmi_phy_info.error[i];
    }

    memcpy(&phdmirx_obj->phy_status, status, sizeof(struct v4l2_ext_hdmi_phy_status));
    return 0;
}

/* V4L2_CID_EXT_HDMI_LINK_STATUS */
int hdmirx_drv_g_ext_link_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_link_status *status)
{
    if (!status) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, status->port);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_link_status hdmi_link_info;
    memset(&hdmi_link_info, 0, sizeof(v4l2_ext_hdmi_link_status));
    hdmi_link_info.port = status->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_LINK_STATUS, &hdmi_link_info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_LINK_STATUS error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d hpd:%d hdmi_5v:%d rx_sense:%d frame_rate_x100_hz:%d dvi_hdmi_mode:%d"
        "video_width:%d video_height:%d color_space:%d color_depth:%d"
        "colorimetry:%d ext_colorimetry:%d additional_colorimetry:%d hdr_type:%d"
        "audio_format:%d audio_sampling_freq:%d audio_channel_number:%d\n",
        __FUNCTION__, hdmi_link_info.port,
        hdmi_link_info.hpd, hdmi_link_info.hdmi_5v,
        hdmi_link_info.rx_sense, hdmi_link_info.frame_rate_x100_hz, hdmi_link_info.dvi_hdmi_mode,
        hdmi_link_info.video_width, hdmi_link_info.video_height, hdmi_link_info.color_space,
        hdmi_link_info.color_depth, hdmi_link_info.colorimetry, hdmi_link_info.ext_colorimetry,
        hdmi_link_info.additional_colorimetry, hdmi_link_info.hdr_type,
        hdmi_link_info.audio_format, hdmi_link_info.audio_sampling_freq, hdmi_link_info.audio_channel_number);

    status->hpd      = hdmi_link_info.hpd;
    status->hdmi_5v  = hdmi_link_info.hdmi_5v;
    status->rx_sense = hdmi_link_info.rx_sense;
    status->frame_rate_x100_hz = hdmi_link_info.frame_rate_x100_hz;
    status->dvi_hdmi_mode = hdmi_link_info.dvi_hdmi_mode;
    status->video_width  = hdmi_link_info.video_width;
    status->video_height = hdmi_link_info.video_height;
    status->color_space  = hdmi_link_info.color_space;
    status->color_depth  = hdmi_link_info.color_depth;
    status->colorimetry  = hdmi_link_info.colorimetry;
    status->ext_colorimetry        = hdmi_link_info.ext_colorimetry;
    status->additional_colorimetry = hdmi_link_info.additional_colorimetry;
    status->hdr_type               = hdmi_link_info.hdr_type;
    status->audio_format          = hdmi_link_info.audio_format;
    status->audio_sampling_freq  = hdmi_link_info.audio_sampling_freq;
    status->audio_channel_number = hdmi_link_info.audio_channel_number;

    memcpy(&phdmirx_obj->link_status, status, sizeof(struct v4l2_ext_hdmi_link_status));
    return 0;
}

/* V4L2_CID_EXT_HDMI_VIDEO_STATUS */
int hdmirx_drv_g_ext_video_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_video_status *status)
{
    if (!status) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, status->port);
    }

    int ret = 0;
    int i = 0;
    struct v4l2_ext_hdmi_video_status hdmi_video_info;
    memset(&hdmi_video_info, 0, sizeof(v4l2_ext_hdmi_video_status));
    hdmi_video_info.port = status->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_VIDEO_STATUS, &hdmi_video_info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_VIDEO_STATUS error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d video_width_real:%d video_htotal_real:%d video_height_real:%d video_vtotal_real:%d"
        "pixel_clock_khz:%d current_vrr_refresh_rate:%d\n",
        __FUNCTION__, hdmi_video_info.port,
        hdmi_video_info.video_width_real, hdmi_video_info.video_htotal_real,
        hdmi_video_info.video_height_real, hdmi_video_info.video_vtotal_real,
        hdmi_video_info.pixel_clock_khz, hdmi_video_info.current_vrr_refresh_rate);

    status->video_width_real      = hdmi_video_info.video_width_real;
    status->video_htotal_real  = hdmi_video_info.video_htotal_real;
    status->video_height_real = hdmi_video_info.video_height_real;
    status->video_vtotal_real = hdmi_video_info.video_vtotal_real;
    status->pixel_clock_khz = hdmi_video_info.pixel_clock_khz;
    status->current_vrr_refresh_rate  = hdmi_video_info.current_vrr_refresh_rate;

    memcpy(&phdmirx_obj->video_status, status, sizeof(struct v4l2_ext_hdmi_video_status));
    return 0;
}

/* V4L2_CID_EXT_HDMI_AUDIO_STATUS */
int hdmirx_drv_g_ext_audio_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_audio_status *status)
{
    if (!status) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, status->port);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_audio_status hdmi_audio_info;
    memset(&hdmi_audio_info, 0, sizeof(v4l2_ext_hdmi_audio_status));
    hdmi_audio_info.port = status->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_AUDIO_STATUS, &hdmi_audio_info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_AUDIO_STATUS error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d pcm_N:%d pcm_CTS:%d LayoutBitValue:%d ChannelStatusBits:%d\n",
        __FUNCTION__, hdmi_audio_info.port,
        hdmi_audio_info.pcm_N, hdmi_audio_info.pcm_CTS,
        hdmi_audio_info.LayoutBitValue, hdmi_audio_info.ChannelStatusBits);

    status->pcm_N             = hdmi_audio_info.pcm_N;
    status->pcm_CTS           = hdmi_audio_info.pcm_CTS;
    status->LayoutBitValue    = hdmi_audio_info.LayoutBitValue;
    status->ChannelStatusBits = hdmi_audio_info.ChannelStatusBits;

    //memcpy(&phdmirx_obj->audio_status, status, sizeof(struct v4l2_ext_hdmi_audio_status));
    return 0;
}

/* V4L2_CID_EXT_HDMI_HDCP_STATUS */
int hdmirx_drv_g_ext_hdcp_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_status *status)
{
    if (!status) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, status->port);
    }

    int ret = 0;
    int i = 0;
    struct v4l2_ext_hdmi_hdcp_status hdmi_hdcp_info;
    memset(&hdmi_hdcp_info, 0, sizeof(v4l2_ext_hdmi_hdcp_status));
    hdmi_hdcp_info.port = status->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_HDCP_STATUS, &hdmi_hdcp_info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_HDCP_STATUS error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d hdcp_version:%d auth_status:%d encEn:%d\n",
        __FUNCTION__, hdmi_hdcp_info.port,
        hdmi_hdcp_info.hdcp_version, hdmi_hdcp_info.auth_status, hdmi_hdcp_info.encEn);

    status->hdcp_version = hdmi_hdcp_info.hdcp_version;
    status->auth_status  = hdmi_hdcp_info.auth_status;
    status->encEn        = hdmi_hdcp_info.encEn;

    LOGD("%s, hdcp14_status.port:%d hdcp14_status.Ri[0]:%d hdmi_hdcp_info.hdcp14_status.Ri[1]:%d"
        "hdmi_hdcp_info.hdcp14_status.Bcaps:%d hdmi_hdcp_info.hdcp14_status.Bstatus[0]:%d hdmi_hdcp_info.hdcp14_status.Bstatus[1]:%d\n",
        __FUNCTION__, hdmi_hdcp_info.hdcp14_status.port,
        hdmi_hdcp_info.hdcp14_status.Ri[0], hdmi_hdcp_info.hdcp14_status.Ri[1],
        hdmi_hdcp_info.hdcp14_status.Bcaps, hdmi_hdcp_info.hdcp14_status.Bstatus[0], hdmi_hdcp_info.hdcp14_status.Bstatus[1]);

    status->hdcp14_status.port = hdmi_hdcp_info.hdcp14_status.port;
    status->hdcp14_status.Ri[0] = hdmi_hdcp_info.hdcp14_status.Ri[0];
    status->hdcp14_status.Ri[1] = hdmi_hdcp_info.hdcp14_status.Ri[1];
    status->hdcp14_status.Bcaps = hdmi_hdcp_info.hdcp14_status.Bcaps;
    status->hdcp14_status.Bstatus[0] = hdmi_hdcp_info.hdcp14_status.Bstatus[0];
    status->hdcp14_status.Bstatus[1] = hdmi_hdcp_info.hdcp14_status.Bstatus[1];
    for (i = 0; i < 8; i++) {
        LOGD("%s, An[%d]:%d\n", __FUNCTION__, i, hdmi_hdcp_info.hdcp14_status.An[i]);
        status->hdcp14_status.An[i] = hdmi_hdcp_info.hdcp14_status.An[i];
    }
    for (i = 0; i < 5; i++) {
        LOGD("%s, Aksv[%d]:%d\n", __FUNCTION__, i, hdmi_hdcp_info.hdcp14_status.Aksv[i]);
        status->hdcp14_status.Aksv[i] = hdmi_hdcp_info.hdcp14_status.Aksv[i];
    }
    for (i = 0; i < 5; i++) {
        LOGD("%s, Bksv[%d]:%d\n", __FUNCTION__, i, hdmi_hdcp_info.hdcp14_status.Bksv[i]);
        status->hdcp14_status.Bksv[i] = hdmi_hdcp_info.hdcp14_status.Bksv[i];
    }

    memcpy(&phdmirx_obj->hdcp_status, status, sizeof(struct v4l2_ext_hdmi_hdcp_status));
    return 0;
}

/* V4L2_CID_EXT_HDMI_SCDC_STATUS */
int hdmirx_drv_g_ext_scdc_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_scdc_status *status)
{
    if (!status) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, status->port);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_scdc_status hdmi_scdc_info;
    memset(&hdmi_scdc_info, 0, sizeof(v4l2_ext_hdmi_scdc_status));
    hdmi_scdc_info.port = status->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_SCDC_STATUS, &hdmi_scdc_info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_SCDC_STATUS error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d source_version:%d sink_version:%d"
        "rsed_update:%d flt_update:%d frl_start:%d source_test_update:%d"
        "rr_test:%d ced_update:%d status_update:%d tmds_bit_clock_ratio:%d"
        "scrambling_enable:%d tmds_scrambler_status:%d dsc_decode_fail:%d"
        "rr_enable:%d ffe_levels:%d frl_rate:%d dsc_decode_fail:%d"
        "flt_ready:%d clk_detect:%d"
        "ch0_locked:%d ch1_locked:%d ch2_locked:%d ch3_locked:%d"
        "lane0_ltp_request:%d lane1_ltp_request:%d lane2_ltp_request:%d lane3_ltp_request:%d"
        "ch0_ced_valid:%d ch1_ced_valid:%d ch2_ced_valid:%d ch3_ced_valid:%d"
        "ch0_ced:%d ch1_ced:%d ch2_ced:%d ch3_ced:%d"
        "rs_correction_valid:%d rs_correcton_count:%d",
        __FUNCTION__, hdmi_scdc_info.port,
        hdmi_scdc_info.source_version, hdmi_scdc_info.sink_version,
        hdmi_scdc_info.rsed_update, hdmi_scdc_info.flt_update, hdmi_scdc_info.frl_start, hdmi_scdc_info.source_test_update,
        hdmi_scdc_info.rr_test, hdmi_scdc_info.ced_update, hdmi_scdc_info.status_update, hdmi_scdc_info.tmds_bit_clock_ratio,
        hdmi_scdc_info.scrambling_enable,  hdmi_scdc_info.tmds_scrambler_status, hdmi_scdc_info.flt_no_retrain,
        hdmi_scdc_info.rr_enable, hdmi_scdc_info.ffe_levels, hdmi_scdc_info.frl_rate, hdmi_scdc_info.dsc_decode_fail,
        hdmi_scdc_info.flt_ready, hdmi_scdc_info.clk_detect,
        hdmi_scdc_info.ch0_locked, hdmi_scdc_info.ch1_locked, hdmi_scdc_info.ch2_locked, hdmi_scdc_info.ch3_locked,
        hdmi_scdc_info.lane0_ltp_request, hdmi_scdc_info.lane1_ltp_request, hdmi_scdc_info.lane2_ltp_request, hdmi_scdc_info.lane3_ltp_request,
        hdmi_scdc_info.ch0_ced_valid, hdmi_scdc_info.ch1_ced_valid, hdmi_scdc_info.ch2_ced_valid, hdmi_scdc_info.ch3_ced_valid,
        hdmi_scdc_info.ch0_ced, hdmi_scdc_info.ch1_ced, hdmi_scdc_info.ch2_ced, hdmi_scdc_info.ch3_ced,
        hdmi_scdc_info.rs_correction_valid, hdmi_scdc_info.rs_correcton_count);

    status->source_version = hdmi_scdc_info.source_version;
    status->sink_version   = hdmi_scdc_info.sink_version;
    status->rsed_update    = hdmi_scdc_info.rsed_update;
    status->flt_update     = hdmi_scdc_info.flt_update;
    status->frl_start      = hdmi_scdc_info.frl_start;
    status->source_test_update    = hdmi_scdc_info.source_test_update;
    status->rr_test               = hdmi_scdc_info.rr_test;
    status->ced_update            = hdmi_scdc_info.ced_update;
    status->status_update         = hdmi_scdc_info.status_update;
    status->tmds_bit_clock_ratio  = hdmi_scdc_info.tmds_bit_clock_ratio;
    status->scrambling_enable     = hdmi_scdc_info.scrambling_enable;
    status->tmds_scrambler_status = hdmi_scdc_info.tmds_scrambler_status;
    status->flt_no_retrain        = hdmi_scdc_info.flt_no_retrain;
    status->rr_enable             = hdmi_scdc_info.rr_enable;
    status->ffe_levels            = hdmi_scdc_info.ffe_levels;
    status->frl_rate              = hdmi_scdc_info.frl_rate;
    status->dsc_decode_fail       = hdmi_scdc_info.dsc_decode_fail;
    status->flt_ready  = hdmi_scdc_info.flt_ready;
    status->clk_detect = hdmi_scdc_info.clk_detect;
    status->ch0_locked = hdmi_scdc_info.ch0_locked;
    status->ch1_locked = hdmi_scdc_info.ch1_locked;
    status->ch2_locked = hdmi_scdc_info.ch2_locked;
    status->ch3_locked = hdmi_scdc_info.ch3_locked;
    status->lane0_ltp_request = hdmi_scdc_info.lane0_ltp_request;
    status->lane1_ltp_request = hdmi_scdc_info.lane1_ltp_request;
    status->lane2_ltp_request = hdmi_scdc_info.lane2_ltp_request;
    status->lane3_ltp_request = hdmi_scdc_info.lane3_ltp_request;
    status->ch0_ced_valid = hdmi_scdc_info.ch0_ced_valid;
    status->ch1_ced_valid = hdmi_scdc_info.ch1_ced_valid;
    status->ch2_ced_valid = hdmi_scdc_info.ch2_ced_valid;
    status->ch3_ced_valid = hdmi_scdc_info.ch3_ced_valid;
    status->ch0_ced = hdmi_scdc_info.ch0_ced;
    status->ch1_ced = hdmi_scdc_info.ch1_ced;
    status->ch2_ced = hdmi_scdc_info.ch2_ced;
    status->ch3_ced = hdmi_scdc_info.ch3_ced;
    status->rs_correction_valid = hdmi_scdc_info.rs_correction_valid;
    status->rs_correcton_count  = hdmi_scdc_info.rs_correcton_count;

    memcpy(&phdmirx_obj->scdc_status, status, sizeof(struct v4l2_ext_hdmi_scdc_status));
    return 0;
}

/* V4L2_CID_EXT_HDMI_ERROR_STATUS */
int hdmirx_drv_g_ext_error_status(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_error_status *status)
{
    if (!status) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d\n", __FUNCTION__, status->port);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_error_status hdmi_error_info;
    memset(&hdmi_error_info, 0, sizeof(v4l2_ext_hdmi_error_status));
    hdmi_error_info.port = status->port;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_GET_HDMI_ERROR_STATUS, &hdmi_error_info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_GET_HDMI_ERROR_STATUS error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    LOGD("%s, port:%d error:%d param1:%d param2:%d\n",
        __FUNCTION__, hdmi_error_info.port,
        hdmi_error_info.error, hdmi_error_info.param1, hdmi_error_info.param2);

    status->error  = hdmi_error_info.error;
    status->param1 = hdmi_error_info.param1;
    status->param2 = hdmi_error_info.param2;

    memcpy(&phdmirx_obj->error_status, status, sizeof(struct v4l2_ext_hdmi_error_status));
    return 0;
}

/* V4L2_CID_EXT_HDMI_EDID */
int hdmirx_drv_s_ext_edid(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_edid *edid)
{
    if (!edid) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d size:%d\n", __FUNCTION__, edid->port, edid->size);
    }

    phdmirx_obj->edid.port = edid->port;
    phdmirx_obj->edid.size = edid->size;
    if (!(edid->pData)) {
        LOGE("%s input edid->pData is NULL\n", __FUNCTION__);
    } else {
        //to do
        int size = 256;
        if (edid->size == V4L2_EXT_HDMI_EDID_SIZE_128) {
           size = 128;
        } else if (edid->size == V4L2_EXT_HDMI_EDID_SIZE_256) {
           size = 256;
        } else if (edid->size == V4L2_EXT_HDMI_EDID_SIZE_512) {
           size = 512;
        } else {
           LOGE("%s input edid->size:%d is invalid\n", __FUNCTION__, edid->size);
           return 0;
        }

        //int i = 0;
        //for (i = 0; i < size; i++) {
            //LOGD("%s input edid->pData[%d]:%d\n", __FUNCTION__, i, edid->pData[i]);
        //}

        int ret = 0;
        ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_SET_HDMI_EDID, edid);
        if (ret < 0) {
            LOGE("%s HDMI_IOC_SET_HDMI_EDID error: %s\n", __FUNCTION__, strerror(errno));
            return ret;
        }
    }

    return 0;
}

/* V4L2_CID_EXT_HDMI_HPD */
int hdmirx_drv_s_ext_hpd(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hpd *hpd)
{
    if (!hpd) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d hpd:%d\n", __FUNCTION__, hpd->port, hpd->hpd_state);
    }

    if ((hpd->port >= V4L2_EXT_HDMI_INPUT_PORT_1) && (hpd->port <= V4L2_EXT_HDMI_INPUT_PORT_4)) {
        int ret = 0;
        struct v4l2_ext_hdmi_hpd info;

        memset(&info, 0, sizeof(struct v4l2_ext_hdmi_hpd));
        info.port      = hpd->port;
        info.hpd_state = hpd->hpd_state;

        ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_SET_HDMI_HPD, &info);
        if (ret < 0) {
            LOGE("%s HDMI_IOC_SET_HDMI_HPD error: %s\n", __FUNCTION__, strerror(errno));
            return ret;
        }
        memcpy(&phdmirx_obj->hpd, hpd, sizeof(struct v4l2_ext_hdmi_hpd));
    } else {
        LOGE("%s, input port:%d is invalid\n", __FUNCTION__, hpd->port);
    }

    return 0;
}

/* V4L2_CID_EXT_HDMI_HDCP_KEY */
int hdmirx_drv_s_ext_hdcp_key(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_key *hdcp_key)
{
    if (!hdcp_key) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d version:%d key_size:%d\n",
            __FUNCTION__, hdcp_key->port, hdcp_key->version, hdcp_key->key_size);
    }

    if (hdcp_key->version >= V4L2_EXT_HDMI_HDCP_VERSION_RESERVED ||
        (hdcp_key->version == V4L2_EXT_HDMI_HDCP_VERSION_14 &&
         hdcp_key->key_size != HDCP14_KEY_SIZE) ||
        (hdcp_key->version == V4L2_EXT_HDMI_HDCP_VERSION_22 &&
         hdcp_key->key_size != HDCP22_KEY_SIZE))
        return -EINVAL;

    phdmirx_obj->hdcp_key.port     = hdcp_key->port;
    phdmirx_obj->hdcp_key.version  = hdcp_key->version;
    phdmirx_obj->hdcp_key.key_size = hdcp_key->key_size;
    if (!(hdcp_key->pData)) {
        LOGE("%s input hdcp_key->pData is NULL\n", __FUNCTION__);
    } else {
        //to do
        //int i = 0;
        //for (i = 0; i < hdcp_key->key_size; i++)
            //LOGD("%s input hdcp_key->pData[%d]:%d\n", __FUNCTION__, i, hdcp_key->pData[i]);

        int ret = 0;
        ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_SET_HDMI_HDCP_KEY, hdcp_key);
        if (ret < 0) {
            LOGE("%s HDMI_IOC_SET_HDMI_HDCP_KEY error: %s\n", __FUNCTION__, strerror(errno));
            return ret;
        }
    }

    return 0;
}

/* V4L2_CID_EXT_HDMI_HDCP_REPEATER */
int hdmirx_drv_s_ext_hdcp_repeater(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_repeater *hdcp_repeater)
{
    if (!hdcp_repeater) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d mode:%d hpd:%d\n",
            __FUNCTION__, hdcp_repeater->port, hdcp_repeater->repeater_mode, hdcp_repeater->repeater_hpd);
    }

    int ret = 0;
    int i = 0;
    struct v4l2_ext_hdmi_hdcp_repeater info;

    memset(&info, 0, sizeof(struct v4l2_ext_hdmi_hdcp_repeater));
    info.port          = hdcp_repeater->port;
    info.repeater_mode = hdcp_repeater->repeater_mode;
    info.repeater_hpd  = hdcp_repeater->repeater_hpd;

    for (i = 0; i < 5; i++) {
        LOGD("%s, receiver_id[%d]:%d\n", __FUNCTION__, i, hdcp_repeater->receiver_id[i]);
        info.receiver_id[i] = hdcp_repeater->receiver_id[i];
    }

    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_SET_HDMI_HDCP_REPEATER, &info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_SET_HDMI_HDCP_REPEATER error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    memcpy(&phdmirx_obj->hdcp_repeater, hdcp_repeater, sizeof(struct v4l2_ext_hdmi_hdcp_repeater));
    return 0;
}

/* V4L2_CID_EXT_HDMI_OVERRIDE_EOTF */
int hdmirx_drv_s_ext_override_eotf(struct hdmirx_obj_s *phdmirx_obj, v4l2_ext_hdmi_override_eotf eotf)
{
    int ret = 0;
    enum v4l2_ext_hdmi_override_eotf info;

    LOGD("%s eotf:%d\n", __FUNCTION__, eotf);

    info = eotf;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_SET_HDMI_OVERRIDE_EOTF, &info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_SET_HDMI_OVERRIDE_EOTF error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    memcpy(&phdmirx_obj->eotf, &eotf, sizeof(enum v4l2_ext_hdmi_override_eotf));
    return 0;
}

/* V4L2_CID_EXT_HDMI_HPD_LOW_DURATION_DC_ON */
int hdmirx_drv_s_ext_dc_on(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hpd_low_duration_dc_on *dc_on)
{
    if (!dc_on) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d hpd_low_duration:%d\n", __FUNCTION__, dc_on->port, dc_on->hpd_low_duration);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_hpd_low_duration_dc_on info;

    memset(&info, 0, sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on));
    info.port             = dc_on->port;
    info.hpd_low_duration = dc_on->hpd_low_duration;

    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_SET_HDMI_HPD_LOW_DURATION_DC_ON, &info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_SET_HDMI_HPD_LOW_DURATION_DC_ON error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    memcpy(&phdmirx_obj->dc_on, dc_on, sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on));
    return 0;
}

/* V4L2_CID_EXT_HDMI_HDCP_REPEATER_TOPOLOGY */
int hdmirx_drv_s_ext_repeater_topology(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_hdcp_repeater_topology *topology)
{
    if (!topology) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d repeater_mode:%d count:%d depth:%d repeater_hpd:%d msg_id:%d\n",
        __FUNCTION__, topology->port, topology->repeater_mode, topology->count, topology->depth, topology->repeater_hpd, topology->msg_id);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_hdcp_repeater_topology info;

    memset(&info, 0, sizeof(struct v4l2_ext_hdmi_hdcp_repeater_topology));
    info.port          = topology->port;
    info.repeater_mode = topology->repeater_mode;
    info.count         = topology->count;
    info.depth         = topology->depth;
    info.repeater_hpd  = topology->repeater_hpd;
    info.msg_id        = topology->msg_id;
    int i = 0, j= 0;
    for (j = 0; j < 5; j++) {
        for (i = 0; i < 32; i++) {
            LOGD("%s, topology->receiver_id[%d][%d]:%d\n", __FUNCTION__, i, j, topology->receiver_id[i][j]);
            info.receiver_id[i][j] = topology->receiver_id[i][j];
        }
    }
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_SET_HDMI_HDCP_REPEATER_TOPOLOGY, &info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_SET_HDMI_HDCP_REPEATER_TOPOLOGY error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    memcpy(&phdmirx_obj->topology, topology, sizeof(struct v4l2_ext_hdmi_hdcp_repeater_topology));
    return 0;
}

/* V4L2_CID_EXT_HDMI_SLEEP */
int hdmirx_drv_s_ext_sleep(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_sleep *sleep_mode)
{
    if (!sleep_mode) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d mode:%d\n", __FUNCTION__, sleep_mode->port, sleep_mode->mode);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_sleep info;

    memset(&info, 0, sizeof(struct v4l2_ext_hdmi_sleep));
    info.port = sleep_mode->port;
    info.mode = sleep_mode->mode;
    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_SET_HDMI_SLEEP, &info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_SET_HDMI_SLEEP error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    memcpy(&phdmirx_obj->sleep_mode, sleep_mode, sizeof(struct v4l2_ext_hdmi_sleep));
    return 0;
}
/* V4L2_CID_EXT_HDMI_EXPERT_SETTING */
int hdmirx_drv_s_ext_expert_setting(struct hdmirx_obj_s *phdmirx_obj, struct v4l2_ext_hdmi_expert_setting *expert_setting)
{
    if (!expert_setting) {
        LOGE("%s input is NULL\n", __FUNCTION__);
        return -1;
    } else {
        LOGD("%s, port:%d type:%d param1:%d param2:%d param3:%d\n",
            __FUNCTION__, expert_setting->port, expert_setting->type, expert_setting->param1, expert_setting->param1, expert_setting->param1);
    }

    int ret = 0;
    struct v4l2_ext_hdmi_expert_setting info;

    memset(&info, 0, sizeof(struct v4l2_ext_hdmi_expert_setting));
    info.port   = expert_setting->port;
    info.type   = expert_setting->type;
    info.param1 = expert_setting->param1;
    info.param2 = expert_setting->param2;
    info.param3 = expert_setting->param3;

    ret = ioctl(phdmirx_obj->hdmirx0_drv_fd, HDMI_IOC_SET_HDMI_EXPERT_SETTING, &info);
    if (ret < 0) {
        LOGE("%s HDMI_IOC_SET_HDMI_EXPERT_SETTING error: %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }

    memcpy(&phdmirx_obj->expert_setting, expert_setting, sizeof(struct v4l2_ext_hdmi_expert_setting));
    return 0;
}

#ifdef __cplusplus
}
#endif
