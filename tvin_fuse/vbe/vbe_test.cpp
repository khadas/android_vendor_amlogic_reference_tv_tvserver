/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#include "../common.h"
#include "vbe_fuse.h"

#define VBE_V4L2_DEV_PATH	V4L2_EXT_DEV_PATH_BACKEND

int video_fd = -1;

class V4L2_VBE_DRIVER
{
public:
    V4L2_VBE_DRIVER();
    ~V4L2_VBE_DRIVER();

    int v4l2_vbe_open();
    int v4l2_vbe_close();
private:
    int32_t fd = 0;
};

V4L2_VBE_DRIVER::V4L2_VBE_DRIVER()
{
	fd = 0;
}

V4L2_VBE_DRIVER::~V4L2_VBE_DRIVER()
{

}

int V4L2_VBE_DRIVER::v4l2_vbe_open()
{
    if (fd == 0) {
        fd = open(VBE_V4L2_DEV_PATH, O_RDWR);
		printf("%s:%d,video_fd:%d\n", __func__, __LINE__, fd);
    } else {
        printf("# already open\n");
    }
    return fd;
}

int vbe_v4l2_open(int flags)
{
	if (video_fd == -1) {
		video_fd = open(VBE_V4L2_DEV_PATH, flags, 0);
		if (video_fd < 0) {
			printf("Unable to open %s,err:%s\n", VBE_V4L2_DEV_PATH,
				strerror(errno));
			goto error;
		}
	}

	printf("%s,open %s OK!,video fd = %d;\n", __func__,
		VBE_V4L2_DEV_PATH, video_fd);
error:
	return video_fd;
}

int vbe_v4l2_close(void)
{
	if (video_fd != -1) {
		close(video_fd);
		video_fd = -1;
	}
	return 0;
}

int vbe_v4l2_s_intput(int input)
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

int vbe_v4l2_g_intput(void)
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

int vbe_v4l2_s_std(void)
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

int vbe_v4l2_g_std(void)
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

int vbe_v4l2_querycap()
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

int vbe_v4l2_s_ctrl(unsigned int cmd, int value)
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

int vbe_v4l2_g_ctrl(unsigned int cmd)
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

int vbe_v4l2_s_ext_ctrls(int cmd, void *param, unsigned int size)
{
	int ret;
	v4l2_ext_controls ext_controls;
	v4l2_ext_control ext_control;

	memset(&ext_controls, 0, sizeof(v4l2_ext_controls));
	memset(&ext_control, 0, sizeof(v4l2_ext_control));

	ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	ext_controls.count = 1;
	ext_controls.controls = &ext_control;
	ext_controls.controls->id = cmd;
	ext_controls.controls->size = size;
	ext_controls.controls->ptr = (void *)param;

	ret = ioctl(video_fd, VIDIOC_S_EXT_CTRLS, &ext_controls);
	if (ret) {
		printf("%d VIDIOC_G_STD failed with %d\n", __LINE__, ret);
		return ret;
	}

	printf("%s,VIDIOC_S_EXT_CTRLS,cmd=%#x,ok\n", __func__, cmd);

	return 0;
}

int vbe_v4l2_g_ext_ctrls(int cmd, void *param, unsigned int size)
{
	int ret;
	v4l2_ext_controls ext_controls;
	v4l2_ext_control ext_control;

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
		printf("%d VIDIOC_G_STD failed with %d\n", __LINE__, ret);
		return ret;
	}

	printf("%s,VIDIOC_G_EXT_CTRLS,cmd=%#x,ok\n", __func__, cmd);

	return 0;
}

int main(int argc, char **argv)
{
	int video_fd = -1;
	//int i, ret;
	int flags = O_RDWR;
	struct v4l2_ext_vbe_mplus_param mplus_param;
	V4L2_VBE_DRIVER driver	= V4L2_VBE_DRIVER();

	vbe_v4l2_open(flags);
	//open the same file twice
	video_fd = driver.v4l2_vbe_open();
	if (video_fd < 0) {
		printf("2.Unable to open %s,err:%s\n", VBE_V4L2_DEV_PATH,
			strerror(errno));
		printf("exited,%s:%d\n", __func__, __LINE__);
	}
	printf("OK,%s:%d,video_fd:%d\n", __func__, __LINE__, video_fd);
	close(video_fd);

	vbe_v4l2_s_ctrl(V4L2_CID_EXT_VBE_MUTE, 1);
	vbe_v4l2_g_ctrl(V4L2_CID_EXT_VBE_GSR_STATUS);

	memset(&mplus_param, 0, sizeof(mplus_param));
	mplus_param.nFrameGainLimit = 0x2000;
	mplus_param.nPixelGainLimit = 0x0800;
	vbe_v4l2_s_ext_ctrls(V4L2_CID_EXT_VBE_MPLUS_PARAM, &mplus_param, sizeof(mplus_param));

	memset(&mplus_param, 0, sizeof(mplus_param));
	vbe_v4l2_g_ext_ctrls(V4L2_CID_EXT_VBE_MPLUS_PARAM, &mplus_param, sizeof(mplus_param));
	printf("V4L2_CID_EXT_VBE_MPLUS_PARAM,%s:%d;%x %x\n", __func__, __LINE__,
		mplus_param.nFrameGainLimit,mplus_param.nPixelGainLimit);

	vbe_v4l2_close();
	printf("exited,%s:%d\n", __func__, __LINE__);

	return 0;
}

