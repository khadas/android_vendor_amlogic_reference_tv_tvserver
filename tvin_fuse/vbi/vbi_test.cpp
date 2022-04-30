/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#include "../common.h"
#include "vbi_fuse.h"

int video_fd = -1;

class V4L2_VBI_DRIVER
{
public:
    V4L2_VBI_DRIVER();
    ~V4L2_VBI_DRIVER();

    int v4l2_vbi_open();
    int v4l2_vbi_close();
private:
    int32_t fd = 0;
};

V4L2_VBI_DRIVER::V4L2_VBI_DRIVER()
{
	fd = 0;
}

V4L2_VBI_DRIVER::~V4L2_VBI_DRIVER()
{

}

int V4L2_VBI_DRIVER::v4l2_vbi_open()
{
    if (fd == 0) {
        fd = open(V4L2_EXT_DEV_PATH_VBI, O_RDWR);
		printf("%s:%d,video_fd:%d\n", __func__, __LINE__, fd);
    } else {
        printf("# already open\n");
    }
    return fd;
}

int vbi_v4l2_open(int flags)
{
	if (video_fd == -1) {
		video_fd = open(V4L2_EXT_DEV_PATH_VBI, flags, 0);
		if (video_fd < 0) {
			printf("Unable to open %s,err:%s\n", V4L2_EXT_DEV_PATH_VBI,
				strerror(errno));
			goto error;
		}
	}

	printf("%s,open %s OK!,video fd = %d;\n", __func__,
		V4L2_EXT_DEV_PATH_VBI, video_fd);
error:
	return video_fd;
}

int vbi_v4l2_close(void)
{
	if (video_fd != -1) {
		close(video_fd);
		video_fd = -1;
	}
	return 0;
}

int vbi_v4l2_s_intput(int input)
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

int vbi_v4l2_g_intput(void)
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

int vbi_v4l2_s_std(void)
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

int vbi_v4l2_g_std(void)
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

int vbi_v4l2_querycap()
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

int vbi_v4l2_s_ctrl(unsigned int cmd, int value)
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

int vbi_v4l2_g_ctrl(unsigned int cmd)
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

int vbi_v4l2_g_ext_ctrls(void)
{
	int ret;
	// Get Video Timing Info
	v4l2_ext_controls ext_controls;
	v4l2_ext_control ext_control;
	struct v4l2_ext_avd_timing_info avd_timing_info;

	memset(&ext_controls, 0, sizeof(v4l2_ext_controls));
	memset(&ext_control, 0, sizeof(v4l2_ext_control));
	memset(&avd_timing_info, 0, sizeof(struct v4l2_ext_avd_timing_info));

	ext_controls.count = 1;
	ext_controls.controls = &ext_control;
	ext_controls.controls->id = V4L2_CID_EXT_AVD_TIMING_INFO;
	ext_controls.controls->size = sizeof(struct v4l2_ext_avd_timing_info);
	ext_controls.controls->ptr = (void *)&avd_timing_info;

	ret = ioctl(video_fd, VIDIOC_G_EXT_CTRLS, &ext_controls);
	if (ret) {
		printf("%d VIDIOC_G_STD failed with %d\n", __LINE__, ret);
		return ret;
	}

	printf("h_f:%d,v_f:%d,h_p:%d,v_p:%d,[%d,%d,%d,%d],lock:%d,%d,%d\n",
		avd_timing_info.h_freq, avd_timing_info.v_freq,
		avd_timing_info.h_porch,avd_timing_info.v_porch,
		avd_timing_info.active.x, avd_timing_info.active.y,
		avd_timing_info.active.w, avd_timing_info.active.h,
		avd_timing_info.vd_lock, avd_timing_info.h_lock,
		avd_timing_info.v_lock);

	printf("%s,VIDIOC_G_EXT_CTRLS,V4L2_CID_EXT_AVD_TIMING_INFO,ok\n",
		__func__);

	return 0;
}

int main(int argc, char **argv)
{
	int video_fd = -1;
	//int i, ret;
	int flags = O_RDWR;

	V4L2_VBI_DRIVER driver	= V4L2_VBI_DRIVER();

	vbi_v4l2_open(flags);
	//open the same file twice
	video_fd = driver.v4l2_vbi_open();
	if (video_fd < 0) {
		printf("2.Unable to open %s,err:%s\n", V4L2_EXT_DEV_PATH_VBI,
			strerror(errno));
		printf("exited,%s:%d\n", __func__, __LINE__);
	}
	printf("OK,%s:%d,video_fd:%d\n", __func__, __LINE__, video_fd);
	close(video_fd);
#if 0
	vbi_v4l2_s_ctrl(V4L2_CID_EXT_AVD_PORT, 2);
	vbi_v4l2_g_ctrl(V4L2_CID_EXT_AVD_PORT);
	vbi_v4l2_s_intput(V4L2_EXT_AVD_INPUT_SRC_AV);
	vbi_v4l2_g_intput();
	vbi_v4l2_g_ext_ctrls();
	vbi_v4l2_s_ctrl(V4L2_CID_EXT_AVD_AUTO_TUNING_MODE, 3);
	vbi_v4l2_g_ctrl(V4L2_CID_EXT_AVD_AUTO_TUNING_MODE);
	vbi_v4l2_g_ctrl(V4L2_CID_EXT_AVD_VIDEO_SYNC);
	vbi_v4l2_s_std();
	vbi_v4l2_g_std();
	vbi_v4l2_querycap();
#endif
	vbi_v4l2_close();
	printf("exited,%s:%d\n", __func__, __LINE__);

	return 0;
}

