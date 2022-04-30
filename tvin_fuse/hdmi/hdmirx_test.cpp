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

#define VBE_V4L2_DEV_PATH	V4L2_EXT_DEV_PATH_BACKEND

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
	printf(" driver: %s\n", 	cap.driver);
	printf(" card: %s\n", 		cap.card);
	printf(" bus_info: %s\n", 	cap.bus_info);
	printf(" version: %08X\n", 	cap.version);
	printf(" version: %08X\n", 	cap.device_caps);
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
	int ret;
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
	if (ret) {
		printf("%d VIDIOC_S_EXT_CTRLS failed with %d\n", __LINE__, ret);
		return ret;
	}

	printf("%s,VIDIOC_S_EXT_CTRLS,cmd=%#x,ok\n", __func__, cmd);

	return 0;
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
	if (ret) {
		printf("%d VIDIOC_G_EXT_CTRLS failed with %d\n", __LINE__, ret);
		return ret;
	}

	printf("%s,VIDIOC_G_EXT_CTRLS,cmd=%#x,ok\n", __func__, cmd);

	return 0;
}

int av_v4l2_g_hdmirx_timing_info(struct v4l2_ext_hdmi_timing_info *info)
{
	int ret;
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
	//printf("i am here %s,%d\n",__func__, __LINE__);

	ret = ioctl(video_fd, VIDIOC_G_EXT_CTRLS, &ext_controls);
	if (ret) {
		printf("%d VIDIOC_G_EXT_CTRLS failed with %d\n", __LINE__, ret);
		return ret;
	}

	printf("h_f:%d,v_f:%d,h_p:%d,v_p:%d,[%d,%d,%d,%d],%d,%d,%d,%d\n",
		info->h_freq,   info->v_vreq,
		info->h_porch,  info->v_porch,
		info->active.x, info->active.y,
		info->active.w, info->active.h,
		info->scan_type,info->dvi_hdmi,
		info->color_depth, info->allm_mode);

	return 0;
}

static void sig_handler(int signo)
{
	isRunning = 0;
	printf("%s,%d,signo=%d\n",__func__, __LINE__, signo);
}

int main(int argc, char **argv)
{
	int video_fd = -1;
	int i, ret, wait_cnt;
	int flags = O_RDWR;
	V4L2_HDMIRX_DRIVER driver	= V4L2_HDMIRX_DRIVER();

	//signal(SIGCHLD, (__sighandler_t)sig_handler);
	signal(SIGINT,  (__sighandler_t)sig_handler);
	signal(SIGKILL, (__sighandler_t)sig_handler);
	signal(SIGQUIT, (__sighandler_t)sig_handler);

	ret = system("echo edid_mode 1 > /sys/class/hdmirx/hdmirx0/param");
	WriteSysfs("/sys/class/graphics/fb0/blank", 0);
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

    struct v4l2_ext_hdmi_hdcp_key hdcp_key = {};
	unsigned char *key = NULL;

	key = new unsigned char[0x100];
	for (i = 0;i < 0x100;i++)
		key[i] = i;//fake data
	hdcp_key.port 		= V4L2_EXT_HDMI_INPUT_PORT_1;
	hdcp_key.version 	= V4L2_EXT_HDMI_HDCP_VERSION_14;
	hdcp_key.key_size 	= HDCP14_KEY_SIZE;//TODO
	hdcp_key.pData      = key;
	hdmirx_v4l2_s_ext_ctrls(V4L2_CID_EXT_HDMI_HDCP_KEY, &hdcp_key, sizeof(hdcp_key));
	delete[] key;

	struct v4l2_ext_hdmi_edid edid;
	unsigned char *data = NULL;

	data = new unsigned char[0x100];
	for (i = 0;i < 0x100;i++)
		data[i] = i;//fake data
	edid.port	= V4L2_EXT_HDMI_INPUT_PORT_1;
	edid.size	= V4L2_EXT_HDMI_EDID_SIZE_256;
	edid.pData	= data;
	hdmirx_v4l2_s_ext_ctrls(V4L2_CID_EXT_HDMI_EDID, &edid, sizeof(edid));

	memset(data, 0, 0x100);
	edid.pData	= data;
	edid.port = V4L2_EXT_HDMI_INPUT_PORT_1;
	edid.size = V4L2_EXT_HDMI_EDID_SIZE_256;
	hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_EDID, &edid, sizeof(edid));
	printf("port:%d,size:%d\n", edid.port, edid.size);
	printf("edid:%x %x %x\n", edid.pData[0], edid.pData[1], edid.pData[2]);
	delete[] data;

	struct v4l2_ext_hdmi_hpd hpd;
	hpd.port = V4L2_EXT_HDMI_INPUT_PORT_1;
	hpd.hpd_state = V4L2_EXT_HDMI_HPD_ENABLE;
	hdmirx_v4l2_s_ext_ctrls(V4L2_CID_EXT_HDMI_HPD, &hpd, sizeof(hpd));

	hpd.port = V4L2_EXT_HDMI_INPUT_PORT_NONE;
	hpd.hpd_state = V4L2_EXT_HDMI_HPD_DISABLE;
	hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_HPD, &hpd, sizeof(hpd));

	hdmirx_v4l2_s_intput(V4L2_EXT_HDMI_INPUT_PORT_1);

	//wait signal stable....
	struct v4l2_ext_hdmi_timing_info timing_info;
	memset(&timing_info, 0, sizeof(struct v4l2_ext_hdmi_timing_info));
	timing_info.port = V4L2_EXT_HDMI_INPUT_PORT_1;
	wait_cnt = 100;
	while (isRunning && wait_cnt--)
	{
		ret = av_v4l2_g_hdmirx_timing_info(&timing_info);
		if (ret < 0)
		{
			printf("%s:%d,av_v4l2_g_avd_timing_info failed\n", __func__, __LINE__);
			break;
		}

		if (timing_info.active.w != 0 && timing_info.active.h != 0)
			break;
		usleep(1000*1000);//1s
	}

	printf("port=%d,freq[h=%d,v=%d],total[h=%d,v=%d],porch[h=%d,v=%d]active[%d %d %d %d],scan_type:%d,dvi_hdmi:%d,depth:%d,allm:%d\n",
		timing_info.port,
		timing_info.h_freq, timing_info.v_vreq, timing_info.h_total, timing_info.v_total,
		timing_info.h_porch, timing_info.v_porch,
		timing_info.active.x, timing_info.active.y, timing_info.active.w, timing_info.active.h,
		timing_info.scan_type, timing_info.dvi_hdmi, timing_info.color_depth, timing_info.allm_mode);

	struct v4l2_ext_hdmi_avi_info avi_info;
	struct v4l2_ext_hdmi_drm_info drm_info = {};
	struct v4l2_ext_hdmi_vsi_info vsi_info = {};
	struct v4l2_ext_hdmi_spd_info spd_info = {};

	hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_AVI_INFO, &avi_info, sizeof(avi_info));
	hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_DRM_INFO, &drm_info, sizeof(drm_info));
	hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_VSI_INFO, &vsi_info, sizeof(vsi_info));
	hdmirx_v4l2_g_ext_ctrls(V4L2_CID_EXT_HDMI_SPD_INFO, &spd_info, sizeof(spd_info));

	getchar();
	hdmirx_v4l2_close();
	printf("exited,%s:%d\n", __func__, __LINE__);

	return 0;
}

