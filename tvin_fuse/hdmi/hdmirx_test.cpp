/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#include "../common.h"
#include "hdmirx_fuse.h"

#define VBE_V4L2_DEV_PATH   V4L2_EXT_DEV_PATH_BACKEND

int video_fd = -1;
int isRunning = 1;

class V4L2_HDMIRX_DRIVER
{
public:
    V4L2_HDMIRX_DRIVER();
    ~V4L2_HDMIRX_DRIVER();

    int v4l2_hdmirx_open();
    int v4l2_hdmirx_close();
private:
    int32_t fd = 0;
};

V4L2_HDMIRX_DRIVER::V4L2_HDMIRX_DRIVER()
{
    fd = 0;
}

V4L2_HDMIRX_DRIVER::~V4L2_HDMIRX_DRIVER()
{

}

int V4L2_HDMIRX_DRIVER::v4l2_hdmirx_open()
{
    if (fd == 0) {
        fd = open(V4L2_EXT_DEV_PATH_HDMI, O_RDWR);
        printf("%s:%d,video_fd:%d\n", __func__, __LINE__, fd);
    } else {
        printf("# already open\n");
    }
    return fd;
}

static int WriteSysfs(const char *path, const char *cmd)
{
    int fd,ret;
    fd = open(path, O_CREAT|O_RDWR | O_TRUNC, 0777);

    if (fd >= 0) {
        ret = write(fd, cmd, strlen(cmd));
        close(fd);
        return ret;
    }

    return -1;
}

static int WriteSysfs(const char *path, int value)
{
    char cmdbuf[32] = {0};
    sprintf(cmdbuf,"%d",value);
    return WriteSysfs(path, cmdbuf);
}

int hdmirx_v4l2_open(int flags)
{
    if (video_fd == -1) {
        video_fd = open(V4L2_EXT_DEV_PATH_HDMI, flags, 0);
        if (video_fd < 0) {
            printf("Unable to open %s,err:%s\n", V4L2_EXT_DEV_PATH_HDMI,
                strerror(errno));
            goto error;
        }
    }

    printf("%s,open %s OK!,video fd = %d;\n", __func__,
        V4L2_EXT_DEV_PATH_HDMI, video_fd);
error:
    return video_fd;
}

int hdmirx_v4l2_close(void)
{
    if (video_fd != -1) {
        close(video_fd);
        video_fd = -1;
    }
    return 0;
}

int hdmirx_v4l2_s_intput(int input)
{
    int input_index;
    int ret;

    input_index = input;
    ret = ioctl(video_fd, VIDIOC_S_INPUT, &input_index);
    if (ret) {
        printf("%d VIDIOC_S_INPUT[%d] failed with %d\n", __LINE__, ret, input_index);
        return ret;
    }

    printf("%s,set input=%d,ok\n", __func__, input_index);

    return 0;
}

int hdmirx_v4l2_g_intput(void)
{
    int input_index = 0;
    int ret;

    ret = ioctl(video_fd, VIDIOC_G_INPUT, &input_index);
    if (ret) {
        printf("%d VIDIOC_G_INPUT[%d] failed with %d\n", __LINE__, ret, input_index);
        return ret;
    }

    printf("%s,get input=%d,ok\n", __func__, input_index);

    return input_index;
}

int hdmirx_v4l2_s_std(void)
{
    int ret;
    v4l2_std_id std = V4L2_STD_PAL_N;

    ret = ioctl(video_fd, VIDIOC_S_STD, &std);
    if (ret) {
        printf("%d VIDIOC_S_STD[%lx] failed with %d\n", __LINE__, ret, std);
        return ret;
    }

    printf("%s,VIDIOC_S_STD(%lx),ok\n", __func__, std);

    return 0;
}

int hdmirx_v4l2_g_std(void)
{
    int ret;
    v4l2_std_id std = 0;

    ret = ioctl(video_fd, VIDIOC_G_STD, &std);
    if (ret) {
        printf("%d VIDIOC_G_STD failed with %d\n", __LINE__, ret);
        return ret;
    }

    printf("%s,VIDIOC_G_STD(%lx),ok\n", __func__, std);

    return 0;
}

int hdmirx_v4l2_querycap()
{
    int ret;
    struct v4l2_capability cap;

    memset(&cap, 0, sizeof(cap));
    ret = ioctl(video_fd, VIDIOC_QUERYCAP, &cap);
    if (ret) {
        printf("%d VIDIOC_QUERYCAP fail :%d\n", __LINE__, ret);
        return -1;
    }
    // Print capability infomations
    printf("Capability Informations:\n");
    printf(" driver: %s\n",     cap.driver);
    printf(" card: %s\n",       cap.card);
    printf(" bus_info: %s\n",   cap.bus_info);
    printf(" version: %08X\n",  cap.version);
    printf(" version: %08X\n",  cap.device_caps);
    printf(" capabilities: %08X\n", cap.capabilities);

    return 0;
}

int hdmirx_v4l2_s_ctrl(unsigned int cmd, int value)
{
    int ret;
    v4l2_control control;

    control.id = cmd;
    control.value = value;

    ret = ioctl(video_fd, VIDIOC_S_CTRL, &control);
    if (ret) {
        printf("%d VIDIOC_G_STD failed with %d\n", __LINE__, ret);
        return ret;
    }

    printf("%s,VIDIOC_S_CTRL,,cmd=%#x(%x),ok\n",
        __func__, cmd, control.value);

    return 0;
}

int hdmirx_v4l2_g_ctrl(unsigned int cmd)
{
    int ret;
    v4l2_control control;

    control.id = cmd;

    ret = ioctl(video_fd, VIDIOC_G_CTRL, &control);
    if (ret) {
        printf("%d VIDIOC_G_STD failed with %d\n", __LINE__, ret);
        return ret;
    }

    printf("%s,VIDIOC_G_CTRL,cmd=%#x(%x),ok\n",
        __func__, cmd, control.value);

    return control.value;
}

int hdmirx_v4l2_s_ext_ctrls(int cmd, void *param, unsigned int size)
{
    int ret = 0;
    struct v4l2_ext_controls ext_controls;
    struct v4l2_ext_control ext_control;

    memset(&ext_controls, 0, sizeof(struct v4l2_ext_controls));
    memset(&ext_control, 0, sizeof(struct v4l2_ext_control));

    ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ext_controls.count = 1;
    ext_controls.controls = &ext_control;
    ext_controls.controls->id = cmd;
    ext_controls.controls->size = size;
    ext_controls.controls->ptr = (void *)param;

    ret = ioctl(video_fd, VIDIOC_S_EXT_CTRLS, &ext_controls);
    if (ret < 0) {
        printf("%s VIDIOC_S_EXT_CTRLS failed with error %s\n", __FUNCTION__, strerror(errno));
    } else {
        printf("%s VIDIOC_S_EXT_CTRLS cmd 0x%x sucess\n", __FUNCTION__, cmd);
    }

    return ret;
}

int hdmirx_v4l2_g_ext_ctrls(int cmd, void *param, unsigned int size)
{
    int ret;
    struct v4l2_ext_controls ext_controls;
    struct v4l2_ext_control ext_control;

    memset(&ext_controls, 0, sizeof(v4l2_ext_controls));
    memset(&ext_control, 0, sizeof(v4l2_ext_control));

    ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ext_controls.count = 1;
    ext_controls.controls = &ext_control;
    ext_controls.controls->id = cmd;
    ext_controls.controls->size = size;
    ext_controls.controls->ptr = (void *)param;

    ret = ioctl(video_fd, VIDIOC_G_EXT_CTRLS, &ext_controls);
    if (ret < 0) {
        printf("%s VIDIOC_G_EXT_CTRLS failed with error %s\n", __FUNCTION__, strerror(errno));
    } else {
        printf("%s VIDIOC_G_EXT_CTRLS cmd 0x%x sucess\n", __FUNCTION__, cmd);
    }

    return ret;
}

int av_v4l2_g_hdmirx_timing_info(struct v4l2_ext_hdmi_timing_info *info)
{
    int ret = 0;
    // Get Video Timing Info
    struct v4l2_ext_controls ext_controls;
    struct v4l2_ext_control ext_control;

    memset(&ext_controls, 0, sizeof(v4l2_ext_controls));
    memset(&ext_control, 0, sizeof(v4l2_ext_control));

    ext_controls.count = 1;
    ext_controls.controls = &ext_control;
    ext_controls.controls->id = V4L2_CID_EXT_HDMI_TIMING_INFO;
    ext_controls.controls->size = sizeof(struct v4l2_ext_hdmi_timing_info);
    ext_controls.controls->ptr = (void *)info;

    ret = ioctl(video_fd, VIDIOC_G_EXT_CTRLS, &ext_controls);
    if (ret < 0) {
        printf("%d VIDIOC_G_EXT_CTRLS failed with %d\n", __LINE__, ret);
    } else {
        printf("h_f:%d,v_f:%d,h_p:%d,v_p:%d,[%d,%d,%d,%d],%d,%d,%d,%d\n",
            info->h_freq,   info->v_vreq,
            info->h_porch,  info->v_porch,
            info->active.x, info->active.y,
            info->active.w, info->active.h,
            info->scan_type,info->dvi_hdmi,
            info->color_depth, info->allm_mode);
    }

    return ret;
}

static void sig_handler(int signo)
{
    isRunning = 0;
    printf("%s,%d,signo=%d\n",__func__, __LINE__, signo);
}

int main(int argc, char **argv)
{
    int video_fd = -1;
    int i, ret;
    int flags = O_RDWR;
    V4L2_HDMIRX_DRIVER driver   = V4L2_HDMIRX_DRIVER();

    //signal(SIGCHLD, (__sighandler_t)sig_handler);
    signal(SIGINT,  (__sighandler_t)sig_handler);
    signal(SIGKILL, (__sighandler_t)sig_handler);
    signal(SIGQUIT, (__sighandler_t)sig_handler);

    //update edid
    ret = system("echo edid_mode 1 > /sys/class/hdmirx/hdmirx0/param");
    //disable osd
    WriteSysfs("/sys/class/graphics/fb0/blank", 0);
    //open hdmi devices
    hdmirx_v4l2_open(flags);
    //open the same file twice
    video_fd = driver.v4l2_hdmirx_open();
    if (video_fd < 0) {
        printf("2.Unable to open %s,err:%s\n", V4L2_EXT_DEV_PATH_HDMI,
            strerror(errno));
        printf("exited,%s:%d\n", __func__, __LINE__);
    }
    printf("OK,%s:%d,video_fd:%d,ret:%d\n", __func__, __LINE__, video_fd, ret);
    close(video_fd);

    //start hdmi devices
    hdmirx_v4l2_s_intput(V4L2_EXT_HDMI_INPUT_PORT_1);

    printf("#### please select cmd####\n");
    printf("#### select 299 exit ####\n");
    printf("#### select 0 to get hdmi timing info ####\n");
    printf("#### select 1 to set hdmi hdcp key    ####\n");
    printf("#### select 2 to set hdmi edid        ####\n");
    printf("#### select 3 to get hdmi edid        ####\n");
    printf("#### select 4 to set hdmi hpd         ####\n");
    printf("#### select 5 to get hdmi hpd         ####\n");
    printf("#### select 6 to get hdmi avi info    ####\n");
    printf("#### select 7 to get hdmi drm info    ####\n");
    printf("#### select 8 to get hdmi vsi info    ####\n");
    printf("#### select 9 to get hdmi spd info    ####\n");
    printf("#### select 10 to get hdmi packet info ####\n");
    printf("#### select 11 to get hdmi dolby hdr   ####\n");
    printf("#### select 12 to get hdmi connect state ####\n");
    printf("#### select 13 to set hdmi power        ####\n");
    printf("#### select 14 to set hdmi disconnect   ####\n");
    printf("#### select 15 to get hdmi vrr         ####\n");
    printf("#### select 16 to get hdmi emp info    ####\n");
    printf("#### select 17 to get hdcp repeater    ####\n");
    printf("#### select 18 to set hdcp repeater    ####\n");
    printf("#### select 19 to set hdmi drm info    ####\n");
    printf("#### select 20 to get hdmi query cap   ####\n");
    printf("#### select 21 to set hdmi low duration    ####\n");
    printf("#### select 22 to get hdmi low duration    ####\n");
    printf("#### select 23 to set hdcp topology        ####\n");
    printf("#### select 24 to get stream manage        ####\n");
    printf("#### select 25 to get hdmi sleep         ####\n");
    printf("#### select 26 to set hdmi sleep         ####\n");
    printf("#### select 27 to get hdmi diagnostics   ####\n");
    printf("#### select 28 to get hdmi phy status    ####\n");
    printf("#### select 29 to get hdmi link status    ####\n");
    printf("#### select 30 to get hdmi video status   ####\n");
    printf("#### select 31 to get hdmi audio status   ####\n");
    printf("#### select 32 to get hdmi hdcp status    ####\n");
    printf("#### select 33 to get hdmi scdc status   ####\n");
    printf("#### select 34 to get hdmi erro status   ####\n");
    printf("#### select 35 to set expert setting     ####\n");

    while (isRunning) {
        char Command[10];
        int conmmand;
        int ret;

        ret = scanf("%s", Command);
        conmmand = atoi(Command);
        printf("#### Command: %s %d\n", Command, conmmand);
        switch (conmmand) {
            case 0: {
                struct v4l2_ext_hdmi_timing_info timing_info;
                memset(&timing_info, 0, sizeof(struct v4l2_ext_hdmi_timing_info));
                timing_info.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                ret = av_v4l2_g_hdmirx_timing_info(&timing_info);
                if (ret < 0) {
                    printf("%s:%d,av_v4l2_g_avd_timing_info failed\n", __func__, __LINE__);
                } else {
                    printf("port=%d,freq[h=%d,v=%d],total[h=%d,v=%d],porch[h=%d,v=%d]active[%d %d %d %d],scan_type:%d,dvi_hdmi:%d,depth:%d,allm:%d\n",
                        timing_info.port,
                        timing_info.h_freq, timing_info.v_vreq, timing_info.h_total, timing_info.v_total,
                        timing_info.h_porch, timing_info.v_porch,
                        timing_info.active.x, timing_info.active.y, timing_info.active.w, timing_info.active.h,
                        timing_info.scan_type, timing_info.dvi_hdmi, timing_info.color_depth, timing_info.allm_mode);
                }
                break;
            }
            case 1: {
                struct v4l2_ext_hdmi_hdcp_key hdcp_key = {};
                unsigned char key[HDCP14_KEY_SIZE];

                for (i = 0; i < HDCP14_KEY_SIZE; i++)
                    key[i] = i;//fake data
                hdcp_key.port       = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdcp_key.version    = V4L2_EXT_HDMI_HDCP_VERSION_14;
                hdcp_key.key_size   = HDCP14_KEY_SIZE;
                hdcp_key.pData      = (unsigned char *)&key;
                hdmirx_v4l2_s_ext_ctrls(V4L2_CID_EXT_HDMI_HDCP_KEY, &hdcp_key, sizeof(struct v4l2_ext_hdmi_hdcp_key));
                break;
            }
            case 2: {
                struct v4l2_ext_hdmi_edid edid;
                unsigned char edid_data[256];

                for (i = 0;  i < 256; i++)
                    edid_data[i] = i;//fake data
                edid.port   = V4L2_EXT_HDMI_INPUT_PORT_1;
                edid.size   = V4L2_EXT_HDMI_EDID_SIZE_256;
                edid.pData  = (unsigned char *)&edid_data;
                hdmirx_v4l2_s_ext_ctrls(V4L2_CID_EXT_HDMI_EDID, &edid, sizeof(struct v4l2_ext_hdmi_edid));
                break;
            }
            case 3: {
                struct v4l2_ext_hdmi_edid edid;
                unsigned char edid_data[256] = {0};

                edid.port   = V4L2_EXT_HDMI_INPUT_PORT_1;
                edid.size   = V4L2_EXT_HDMI_EDID_SIZE_256;
                edid.pData  = edid_data;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_EDID, &edid, sizeof(struct v4l2_ext_hdmi_edid));

                int size = 256;
                if (edid.size == V4L2_EXT_HDMI_EDID_SIZE_128) {
                   size = 128;
                } else if (edid.size == V4L2_EXT_HDMI_EDID_SIZE_256) {
                   size = 256;
                } else if (edid.size == V4L2_EXT_HDMI_EDID_SIZE_512) {
                   size = 512;
                } else {
                   printf("%s get edid size:%d is invalid\n", __FUNCTION__, edid.size);
                   break;
                }

                int i = 0;
                for (i = 0; i < size; i++) {
                    printf("%s get edid.pData[%d]:%d\n", __FUNCTION__, i, edid.pData[i]);
                }
                break;
            }
            case 4: {
                struct v4l2_ext_hdmi_hpd hpd;
                hpd.port      = V4L2_EXT_HDMI_INPUT_PORT_1;
                hpd.hpd_state = V4L2_EXT_HDMI_HPD_ENABLE;
                hdmirx_v4l2_s_ext_ctrls(V4L2_CID_EXT_HDMI_HPD, &hpd, sizeof(struct v4l2_ext_hdmi_hpd));
                break;
            }
            case 5: {
                struct v4l2_ext_hdmi_hpd hpd;
                hpd.port      = V4L2_EXT_HDMI_INPUT_PORT_1;
                hpd.hpd_state = V4L2_EXT_HDMI_HPD_DISABLE;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_HPD, &hpd, sizeof(struct v4l2_ext_hdmi_hpd));
                break;
            }
            case 6: {
                struct v4l2_ext_hdmi_avi_info avi_info;
                memset(&avi_info, 0, sizeof(struct v4l2_ext_hdmi_avi_info));
                avi_info.port      = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_AVI_INFO, &avi_info, sizeof(struct v4l2_ext_hdmi_avi_info));
                break;
            }
            case 7: {
                struct v4l2_ext_hdmi_drm_info drm_info = {};
                memset(&drm_info, 0, sizeof(struct v4l2_ext_hdmi_drm_info));
                drm_info.port      = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_DRM_INFO, &drm_info, sizeof(struct v4l2_ext_hdmi_drm_info));
                break;
            }
            case 8: {
                struct v4l2_ext_hdmi_vsi_info vsi_info = {};
                memset(&vsi_info, 0, sizeof(struct v4l2_ext_hdmi_vsi_info));
                vsi_info.port      = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_VSI_INFO, &vsi_info, sizeof(struct v4l2_ext_hdmi_vsi_info));
                break;
            }
            case 9: {
                struct v4l2_ext_hdmi_spd_info spd_info = {};
                memset(&spd_info, 0, sizeof(struct v4l2_ext_hdmi_spd_info));
                spd_info.port      = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_SPD_INFO, &spd_info, sizeof(struct v4l2_ext_hdmi_spd_info));
                break;
            }
            case 10: {
                struct v4l2_ext_hdmi_packet_info packet_info = {};
                memset(&packet_info, 0, sizeof(struct v4l2_ext_hdmi_packet_info));
                packet_info.port      = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_PACKET_INFO, &packet_info, sizeof(struct v4l2_ext_hdmi_packet_info));
                break;
            }
            case 11: {
                struct v4l2_ext_hdmi_dolby_hdr dolby_hdr = {};
                memset(&dolby_hdr, 0, sizeof(struct v4l2_ext_hdmi_dolby_hdr));
                dolby_hdr.port      = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_DOLBY_HDR, &dolby_hdr, sizeof(struct v4l2_ext_hdmi_dolby_hdr));
                break;
            }
            case 12: {
                struct v4l2_ext_hdmi_connection_state hdmi_port_state = {};
                memset(&hdmi_port_state, 0, sizeof(struct v4l2_ext_hdmi_connection_state));
                hdmi_port_state.port      = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_CONNECTION_STATE, &hdmi_port_state, sizeof(struct v4l2_ext_hdmi_connection_state));
                break;
            }
            case 13: {
                int value = 1;
                hdmirx_v4l2_s_ctrl(V4L2_CID_EXT_HDMI_POWER_OFF, value);
                break;
            }
            case 14: {
                int value = 1;
                hdmirx_v4l2_s_ctrl(V4L2_CID_EXT_HDMI_DISCONNECT, value);
                break;
            }
            case 15: {
                struct v4l2_ext_hdmi_vrr_frequency freq = {};
                memset(&freq, 0, sizeof(struct v4l2_ext_hdmi_vrr_frequency));
                freq.port      = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_VRR_FREQUENCY, &freq, sizeof(struct v4l2_ext_hdmi_vrr_frequency));
                break;
            }
            case 16: {
                struct v4l2_ext_hdmi_emp_info emp = {};
                memset(&emp, 0, sizeof(struct v4l2_ext_hdmi_emp_info));
                emp.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                emp.type = V4L2_EXT_HDMI_EMP_TYPE_VTEM;
                emp.current_packet_index = 1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_EMP_INFO, &emp, sizeof(struct v4l2_ext_hdmi_emp_info));
                break;
            }
            case 17: {
                struct v4l2_ext_hdmi_hdcp_repeater repeater = {};
                memset(&repeater, 0, sizeof(struct v4l2_ext_hdmi_hdcp_repeater));
                repeater.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_HDCP_REPEATER, &repeater, sizeof(struct v4l2_ext_hdmi_hdcp_repeater));
                break;
            }
            case 18: {
                struct v4l2_ext_hdmi_hdcp_repeater repeater = {};
                memset(&repeater, 0, sizeof(struct v4l2_ext_hdmi_hdcp_repeater));
                repeater.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                repeater.repeater_mode = 1;
                repeater.receiver_id[0] = 1;
                repeater.repeater_hpd =1;
                hdmirx_v4l2_s_ext_ctrls(V4L2_CID_EXT_HDMI_HDCP_REPEATER, &repeater, sizeof(struct v4l2_ext_hdmi_hdcp_repeater));
                break;
            }
            case 19: {
                struct v4l2_ext_hdmi_override_drm_info status = {};
                memset(&status, 0, sizeof(struct v4l2_ext_hdmi_override_drm_info));
                status.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                status.override_eotf = V4L2_EXT_HDMI_OVERRIDE_EOTF_HLG;
                hdmirx_v4l2_s_ext_ctrls(V4L2_CID_EXT_HDMI_OVERRIDE_EOTF, &status, sizeof(struct v4l2_ext_hdmi_override_drm_info));
                break;
            }
            case 20: {
                struct v4l2_ext_hdmi_querycap hdmi_querycap = {};
                memset(&hdmi_querycap, 0, sizeof(struct v4l2_ext_hdmi_querycap));
                hdmi_querycap.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_QUERYCAP, &hdmi_querycap, sizeof(struct v4l2_ext_hdmi_querycap));
                break;
            }
            case 21: {
                struct v4l2_ext_hdmi_hpd_low_duration_dc_on status = {};
                memset(&status, 0, sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on));
                status.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                status.hpd_low_duration = 200;
                hdmirx_v4l2_s_ext_ctrls(V4L2_CID_EXT_HDMI_HPD_LOW_DURATION_DC_ON, &status, sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on));
                break;
            }
            case 22: {
                struct v4l2_ext_hdmi_hpd_low_duration_dc_on status = {};
                memset(&status, 0, sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on));
                status.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_HPD_LOW_DURATION_DC_ON, &status, sizeof(struct v4l2_ext_hdmi_hpd_low_duration_dc_on));
                break;
            }
            case 23: {
                struct v4l2_ext_hdmi_hdcp_repeater_topology topology = {};
                memset(&topology, 0, sizeof(struct v4l2_ext_hdmi_hdcp_repeater_topology));
                topology.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_s_ext_ctrls(V4L2_CID_EXT_HDMI_HDCP_REPEATER_TOPOLOGY, &topology, sizeof(struct v4l2_ext_hdmi_hdcp_repeater_topology));
                break;
            }
            case 24: {
                struct v4l2_ext_hdmi_hdcp_repeater_stream_manage stream_manage = {};
                memset(&stream_manage, 0, sizeof(struct v4l2_ext_hdmi_hdcp_repeater_stream_manage));
                stream_manage.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_HDCP_REPEATER_STREAM_MANAGE, &stream_manage, sizeof(struct v4l2_ext_hdmi_hdcp_repeater_stream_manage));
                break;
            }
            case 25: {
                struct v4l2_ext_hdmi_sleep sleep_mode = {};
                memset(&sleep_mode, 0, sizeof(struct v4l2_ext_hdmi_sleep));
                sleep_mode.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_SLEEP, &sleep_mode, sizeof(struct v4l2_ext_hdmi_sleep));
                break;
            }
            case 26: {
                struct v4l2_ext_hdmi_sleep sleep_mode = {};
                memset(&sleep_mode, 0, sizeof(struct v4l2_ext_hdmi_sleep));
                sleep_mode.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                sleep_mode.mode = V4L2_EXT_HDMI_SLEEP_MODE;
                hdmirx_v4l2_s_ext_ctrls(V4L2_CID_EXT_HDMI_SLEEP, &sleep_mode, sizeof(struct v4l2_ext_hdmi_sleep));
                break;
            }
            case 27: {
                struct v4l2_ext_hdmi_diagnostics_status status = {};
                memset(&status, 0, sizeof(struct v4l2_ext_hdmi_diagnostics_status));
                status.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_DIAGNOSTICS_STATUS, &status, sizeof(struct v4l2_ext_hdmi_diagnostics_status));
                break;
            }
            case 28: {
                struct v4l2_ext_hdmi_phy_status status = {};
                memset(&status, 0, sizeof(struct v4l2_ext_hdmi_phy_status));
                status.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_PHY_STATUS, &status, sizeof(struct v4l2_ext_hdmi_phy_status));
                break;
            }
            case 29: {
                struct v4l2_ext_hdmi_link_status status = {};
                memset(&status, 0, sizeof(struct v4l2_ext_hdmi_link_status));
                status.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_LINK_STATUS, &status, sizeof(struct v4l2_ext_hdmi_link_status));
                break;
            }
            case 30: {
                struct v4l2_ext_hdmi_video_status status = {};
                memset(&status, 0, sizeof(struct v4l2_ext_hdmi_video_status));
                status.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_VIDEO_STATUS, &status, sizeof(struct v4l2_ext_hdmi_video_status));
                break;
            }
            case 31: {
                struct v4l2_ext_hdmi_audio_status status = {};
                memset(&status, 0, sizeof(struct v4l2_ext_hdmi_audio_status));
                status.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_AUDIO_STATUS, &status, sizeof(struct v4l2_ext_hdmi_audio_status));
                break;
            }
            case 32: {
                struct v4l2_ext_hdmi_hdcp_status status = {};
                memset(&status, 0, sizeof(struct v4l2_ext_hdmi_hdcp_status));
                status.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_HDCP_STATUS, &status, sizeof(struct v4l2_ext_hdmi_hdcp_status));
                break;
            }
            case 33: {
                struct v4l2_ext_hdmi_scdc_status status = {};
                memset(&status, 0, sizeof(struct v4l2_ext_hdmi_scdc_status));
                status.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_SCDC_STATUS, &status, sizeof(struct v4l2_ext_hdmi_scdc_status));
                break;
            }
            case 34: {
                struct v4l2_ext_hdmi_error_status status = {};
                memset(&status, 0, sizeof(struct v4l2_ext_hdmi_error_status));
                status.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_ERROR_STATUS, &status, sizeof(struct v4l2_ext_hdmi_error_status));
                break;
            }
            case 35: {
                struct v4l2_ext_hdmi_expert_setting setting = {};
                memset(&setting, 0, sizeof(struct v4l2_ext_hdmi_expert_setting));
                setting.port = V4L2_EXT_HDMI_INPUT_PORT_1;
                setting.type = V4L2_EXT_HDMI_EXPERT_SETTING_TYPE_TMDS_MANUAL_EQ_CH0;
                setting.param1 = 7;
                hdmirx_v4l2_s_ext_ctrls(V4L2_CID_EXT_HDMI_EXPERT_SETTING, &setting, sizeof(struct v4l2_ext_hdmi_expert_setting));
                break;
            }
            case 299: {
                isRunning = 0;
                break;
            }
            default: {
                isRunning = 0;
                break;
            }
        }
        printf("#### please input cmd again####\n");
        //usleep(1000*1000);//1s
    }

    hdmirx_v4l2_close();
    printf("exited,%s:%d\n", __func__, __LINE__);

    return 0;
}

