/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#include "../common.h"
#include "pq_fuse.h"

int video_fd = -1;

int pq_v4l2_open(int flags)
{
	if (video_fd == -1) {
		video_fd = open(V4L2_EXT_DEV_PATH_PQ, flags, 0);
		if (video_fd < 0) {
			printf("Unable to open %s, err:%s\n", V4L2_EXT_DEV_PATH_PQ,
				strerror(errno));
			goto error;
		}
	}

	printf("%s, open %s OK!, video fd = %d\n", __func__,
		V4L2_EXT_DEV_PATH_PQ, video_fd);

error:
	return video_fd;
}

int pq_v4l2_close(void)
{
	if (video_fd != -1) {
		close(video_fd);
		video_fd = -1;
	}

	return 0;
}

int pq_v4l2_s_ctrl(unsigned int cmd, int value)
{
	int ret;
	v4l2_control control;

	control.id = cmd;
	control.value = value;

	ret = ioctl(video_fd, VIDIOC_S_CTRL, &control);
	if (ret) {
		printf("%d VIDIOC_S_CTRL failed with %d, %s\n",
			__LINE__, ret, strerror(errno));
		return ret;
	}

	printf("%s, VIDIOC_S_CTRL, cmd = %#x(%x), ok\n",
		__func__, cmd, control.value);

	return 0;
}

int pq_v4l2_g_ctrl(unsigned int cmd)
{
	int ret;
	v4l2_control control;

	control.id = cmd;
	control.value = 0xee;

	ret = ioctl(video_fd, VIDIOC_G_CTRL, &control);
	if (ret) {
		printf("%d VIDIOC_G_CTRL failed with %d, %s\n",
			__LINE__, ret, strerror(errno));
		return ret;
	}

	printf("%s, VIDIOC_G_CTRL, cmd = %#x(%x), ok\n",
		__func__, cmd, control.value);

	return control.value;
}

int pq_v4l2_s_ext_ctrls(int cmd, void *param, unsigned int size)
{
	int ret = 0;
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
		printf("%d VIDIOC_S_EXT_CTRLS failed with %d, %s\n",
			__LINE__, ret, strerror(errno));
		return ret;
	}

	printf("%s, VIDIOC_S_EXT_CTRLS, cmd = %#x, ok\n", __func__, cmd);

	return 0;
}

int pq_v4l2_g_ext_ctrls(int cmd, void *param, unsigned int size)
{
	int ret = 0;
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
		printf("%d VIDIOC_G_EXT_CTRLS failed with %d, %s\n",
			__LINE__, ret, strerror(errno));
		return ret;
	}

	printf("%s, VIDIOC_G_EXT_CTRLS, cmd = %#x, ok\n", __func__, cmd);

	return 0;
}

int main(int argc, char **argv)
{
	int flags = O_RDWR;
	struct v4l2_ext_vpq_cmn_data pqData;
	struct v4l2_ext_vpq_picture_ctrl_data pictureCtrl;
	struct v4l2_ext_led_ldim_demo_info leddemoinfo;
	struct v4l2_ext_led_spi_ctrl_info ledcontrolspi;
	int value;

	pq_v4l2_open(flags);

	pq_v4l2_s_ctrl(V4L2_CID_EXT_VPQ_LOW_DELAY_MODE, 1);
	pq_v4l2_g_ctrl(V4L2_CID_EXT_VPQ_LOW_DELAY_MODE);
	pq_v4l2_s_ctrl(V4L2_CID_EXT_VPQ_LOW_DELAY_MODE, 0);
	pq_v4l2_g_ctrl(V4L2_CID_EXT_VPQ_LOW_DELAY_MODE);

	printf("Please enter any key to continue %s...\n",
		__func__);
	getchar();

	printf("test V4L2_CID_EXT_LED_EN\n");
	value = pq_v4l2_g_ctrl(V4L2_CID_EXT_LED_EN);
	printf("V4L2_CID_EXT_LED_EN =  %d\n",value);
	pq_v4l2_s_ctrl(V4L2_CID_EXT_LED_EN, 1);

	printf("Please enter any key to continue %s...\n",
		__func__);
	getchar();

	printf("test V4L2_CID_EXT_LED_DB_IDX\n");
	value = pq_v4l2_g_ctrl(V4L2_CID_EXT_LED_DB_IDX);
	printf("V4L2_CID_EXT_LED_DB_IDX =  %d\n",value);
	pq_v4l2_s_ctrl(V4L2_CID_EXT_LED_DB_IDX, 1);

	printf("Please enter any key to continue %s...\n",
		__func__);
	getchar();

	printf("test V4L2_CID_EXT_LED_DEMOMODE\n");
	memset(&pqData, 0, sizeof(v4l2_ext_vpq_cmn_data));
	memset(&leddemoinfo, 0, sizeof(v4l2_ext_led_ldim_demo_info));
	leddemoinfo.eType = v4l2_ext_led_ldim_demo_type_topbottom;
	leddemoinfo.bOnOff = 1;

	pqData.version = 1;
	pqData.length = sizeof(v4l2_ext_led_ldim_demo_info);
	pqData.wid = 0;
	pqData.p_data = (unsigned char*)&leddemoinfo;

	pq_v4l2_s_ext_ctrls(V4L2_CID_EXT_LED_DEMOMODE,
		&pqData, sizeof(v4l2_ext_vpq_cmn_data));

	printf("now begin to read!\n");
	pq_v4l2_g_ext_ctrls(V4L2_CID_EXT_LED_DEMOMODE,
		&pqData, sizeof(v4l2_ext_vpq_cmn_data));

	printf("bOnOff = %d, eType = %d\n",
		leddemoinfo.bOnOff,
		leddemoinfo.eType);

	printf("Please enter any key to continue %s...\n",
		__func__);
	getchar();

	printf("test V4L2_CID_EXT_LED_CONTROL_SPI\n");
	memset(&pqData, 0, sizeof(v4l2_ext_vpq_cmn_data));
	memset(&ledcontrolspi, 0, sizeof(v4l2_ext_led_spi_ctrl_info));
	ledcontrolspi.bitMask = 3;
	ledcontrolspi.ctrlValue  = 1;

	pqData.version = 1;
	pqData.length = sizeof(v4l2_ext_led_spi_ctrl_info);
	pqData.wid = 0;
	pqData.p_data = (unsigned char*)&ledcontrolspi;

	pq_v4l2_s_ext_ctrls(V4L2_CID_EXT_LED_CONTROL_SPI,
		&pqData, sizeof(v4l2_ext_vpq_cmn_data));

	printf("now begin to read!\n");
	pq_v4l2_g_ext_ctrls(V4L2_CID_EXT_LED_CONTROL_SPI,
		&pqData, sizeof(v4l2_ext_vpq_cmn_data));
	printf("bitmask = %d, ctrlvalue = %d\n",
		ledcontrolspi.bitMask,
		ledcontrolspi.ctrlValue);

	printf("Please enter any key to continue %s...\n",
		__func__);
	getchar();

	printf("test V4L2_CID_EXT_VPQ_PICTURE_CTRL\n");
	memset(&pqData, 0, sizeof(v4l2_ext_vpq_cmn_data));
	memset(&pictureCtrl, 0, sizeof(v4l2_ext_vpq_picture_ctrl_data));

	pictureCtrl.picture_ui_value[V4L2_EXT_VPQ_PICTURE_CTRL_CONTRAST] = 10;
	pictureCtrl.picture_ui_value[V4L2_EXT_VPQ_PICTURE_CTRL_BRIGHTNESS] = 11;
	pictureCtrl.picture_ui_value[V4L2_EXT_VPQ_PICTURE_CTRL_SATURATION] = 12;
	pictureCtrl.picture_ui_value[V4L2_EXT_VPQ_PICTURE_CTRL_HUE] = 13;
	pictureCtrl.chipData_contrast = -500;
	pictureCtrl.chipData_brightness = 500;
	pictureCtrl.chipData_saturation = 100;
	pictureCtrl.chipData_hue = 20;

	pqData.version = 1;
	pqData.length = sizeof(v4l2_ext_vpq_picture_ctrl_data);
	pqData.wid = 0;
	pqData.p_data = (unsigned char*)&pictureCtrl;

	pq_v4l2_s_ext_ctrls(V4L2_CID_EXT_VPQ_PICTURE_CTRL,
		&pqData, sizeof(v4l2_ext_vpq_cmn_data));

	printf("Please enter any key to continue %s...\n",
		__func__);
	getchar();

	memset(&pqData, 0, sizeof(v4l2_ext_vpq_cmn_data));
	memset(&pictureCtrl, 0, sizeof(v4l2_ext_vpq_picture_ctrl_data));

	pqData.version = 1;
	pqData.length = sizeof(v4l2_ext_vpq_picture_ctrl_data);
	pqData.p_data = (unsigned char*)&pictureCtrl;

	pq_v4l2_g_ext_ctrls(V4L2_CID_EXT_VPQ_PICTURE_CTRL,
		&pqData, sizeof(v4l2_ext_vpq_cmn_data));
	printf("V4L2_CID_EXT_VPQ_PICTURE_CTRL, %s:%d picture_ui_value = %d/%d/%d/%d\n",
		__func__, __LINE__,
		pictureCtrl.picture_ui_value[0], pictureCtrl.picture_ui_value[1],
		pictureCtrl.picture_ui_value[2], pictureCtrl.picture_ui_value[3]);
	printf("V4L2_CID_EXT_VPQ_PICTURE_CTRL, %s:%d C/B/S/H = %d/%d/%d/%d\n",
		__func__, __LINE__,
		pictureCtrl.chipData_contrast, pictureCtrl.chipData_brightness,
		pictureCtrl.chipData_saturation, pictureCtrl.chipData_hue);
	printf("V4L2_CID_EXT_VPQ_PICTURE_CTRL,%s, %d, wid: %d\n",
		__func__, __LINE__, pqData.wid);

	pq_v4l2_close();
	printf("exited, %s:%d\n", __func__, __LINE__);

	return 0;
}

