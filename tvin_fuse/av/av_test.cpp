/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#include "../common.h"
#include "av_fuse.h"

#define AV_V4L2_DEV_PATH	V4L2_EXT_DEV_PATH_AV

int video_fd = -1;
int isRunning = 1;

class V4L2_AVD_DRIVER
{
public:
    V4L2_AVD_DRIVER();
    ~V4L2_AVD_DRIVER();

    int v4l2_avd_open();
    int v4l2_avd_close();
private:
    int32_t fd = 0;
};

V4L2_AVD_DRIVER::V4L2_AVD_DRIVER()
{
	fd = 0;
}

V4L2_AVD_DRIVER::~V4L2_AVD_DRIVER()
{

}

int V4L2_AVD_DRIVER::v4l2_avd_open()
{
    if (fd == 0) {
        fd = open(AV_V4L2_DEV_PATH, O_RDWR);
		printf("%s:%d,video_fd:%d\n", __func__, __LINE__, fd);
    } else {
        printf("# already open\n");
    }
    return fd;
}

static int WriteSysfs(const char *path, const char *cmd)
{
    int fd, ret;
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

int av_v4l2_open(int flags)
{
	if (video_fd == -1) {
		video_fd = open(AV_V4L2_DEV_PATH, flags, 0);
		if (video_fd < 0) {
			printf("Unable to open %s,err:%s\n", AV_V4L2_DEV_PATH,
				strerror(errno));
			goto error;
		}
	}

	printf("%s,open %s OK!,video fd = %d;\n", __func__,
		AV_V4L2_DEV_PATH, video_fd);
error:
	return video_fd;
}

int av_v4l2_close(void)
{
	if (video_fd != -1) {
		close(video_fd);
		video_fd = -1;
	}
	return 0;
}

int av_v4l2_s_intput(int input)
{
	int input_index;
	int ret;

	input_index = input;
	ret = ioctl(video_fd, VIDIOC_S_INPUT, &input_index);
	if (ret) {
		printf("%d VIDIOC_S_INPUT[%d] failed with %d\n", __LINE__, input_index, ret);
		return ret;
	}

	printf("%s,set input=%d,ok\n", __func__, input_index);

	return 0;
}

int av_v4l2_g_intput(enum v4l2_ext_avd_input_src *input_index)
{
	//enum v4l2_ext_avd_input_src input_index = 0;
	int ret;

	ret = ioctl(video_fd, VIDIOC_G_INPUT, input_index);
	if (ret) {
		printf("%d VIDIOC_G_INPUT[%d] failed with %d\n", __LINE__, ret, *input_index);
		return ret;
	}

	printf("%s,get input=%d,ok\n", __func__, *input_index);

	return ret;
}

int av_v4l2_s_std(v4l2_std_id std)
{
	int ret;
	//v4l2_std_id std = V4L2_STD_PAL_N;

	ret = ioctl(video_fd, VIDIOC_S_STD, &std);
	if (ret) {
		printf("%d VIDIOC_S_STD[%lx] failed with %d\n", __LINE__, ret, std);
		return ret;
	}

	printf("%s,VIDIOC_S_STD(%lx),ok\n", __func__, std);

	return 0;
}

v4l2_std_id av_v4l2_g_std(void)
{
	int ret;
	v4l2_std_id std = 0;

	ret = ioctl(video_fd, VIDIOC_G_STD, &std);
	if (ret) {
		printf("%d VIDIOC_G_STD failed with %d\n", __LINE__, ret);
		return ret;
	}

	printf("%s,VIDIOC_G_STD(%lx),ok\n", __func__, std);

	return std;
}

int av_v4l2_querycap()
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

int av_v4l2_s_ctrl(unsigned int cmd, int value)
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

int av_v4l2_g_ctrl(unsigned int cmd)
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

int av_v4l2_g_avd_timing_info(struct v4l2_ext_avd_timing_info *info)
{
	int ret;
	// Get Video Timing Info
	v4l2_ext_controls ext_controls;
	v4l2_ext_control ext_control;

	memset(&ext_controls, 0, sizeof(v4l2_ext_controls));
	memset(&ext_control, 0, sizeof(v4l2_ext_control));
	memset(info, 0, sizeof(struct v4l2_ext_avd_timing_info));

	ext_controls.count = 1;
	ext_controls.controls = &ext_control;
	ext_controls.controls->id = V4L2_CID_EXT_AVD_TIMING_INFO;
	ext_controls.controls->size = sizeof(struct v4l2_ext_avd_timing_info);
	ext_controls.controls->ptr = (void *)info;

	ret = ioctl(video_fd, VIDIOC_G_EXT_CTRLS, &ext_controls);
	if (ret) {
		printf("%d VIDIOC_G_STD failed with %d\n", __LINE__, ret);
		return ret;
	}

	printf("h_f:%d,v_f:%d,h_p:%d,v_p:%d,[%d,%d,%d,%d],lock:%d,%d,%d\n",
		info->h_freq,   info->v_freq,
		info->h_porch,  info->v_porch,
		info->active.x, info->active.y,
		info->active.w, info->active.h,
		info->vd_lock,  info->h_lock,
		info->v_lock);

	return 0;
}

void avd_testcase_202(void)
{
    int s_ret = 0;
    int g_ret = 0;
    int c_ret = 0;
    enum v4l2_ext_avd_input_src s_input;
    enum v4l2_ext_avd_input_src g_input;
    int cnt = 0;
	enum v4l2_ext_avd_input_src input_source = V4L2_EXT_AVD_INPUT_SRC_AV;
	struct timeval before, after;
	unsigned long delta_time;

    // Connect (VIDIOC_S_INPUT, VIDIOC_G_INPUT)
    do {
        if ((cnt % 2) == 0)
            s_input = input_source;
        else
            s_input = V4L2_EXT_AVD_INPUT_SRC_NONE;
		gettimeofday(&before, NULL);
		s_ret = av_v4l2_s_intput(s_input);
		gettimeofday(&after, NULL);
		delta_time = 1000 * 1000 * (after.tv_sec - before.tv_sec)
			+ (after.tv_usec - before.tv_usec);
		printf("cnt=%d,%d,time:%d us\n", cnt, s_input, delta_time);

		av_v4l2_g_intput(&g_input);
		//s_ret = driver.set_avd_input(s_input);
		//g_ret = driver.get_avd_input(g_input);

        if (s_input == g_input)
            c_ret = true;
        else
            c_ret = false;

        if ((s_ret != 0) && (g_ret != 0) && (c_ret == false))
            break;

        cnt++;
        usleep(1000 * 50);

        if (cnt >= 50) {
            //driver.set_avd_input(input_source);
            av_v4l2_s_intput(V4L2_EXT_AVD_INPUT_SRC_AV);
            break;
        }
    } while (1);
    usleep(1000);
}

static void sig_handler(int signo)
{
	isRunning = 0;
}

int main(int argc, char **argv)
{
	int video_fd = -1;
	int ret,wait_cnt;
	int flags = O_RDWR;
	v4l2_std_id std;
	unsigned long delta_time;
	struct timeval before, after;
	struct v4l2_ext_avd_timing_info timing_info;
	V4L2_AVD_DRIVER driver	= V4L2_AVD_DRIVER();
	enum v4l2_ext_avd_input_src input_source = V4L2_EXT_AVD_INPUT_SRC_AV;

	signal(SIGCHLD, (__sighandler_t)sig_handler);
	signal(SIGINT,  (__sighandler_t)sig_handler);
	signal(SIGKILL, (__sighandler_t)sig_handler);
	signal(SIGQUIT, (__sighandler_t)sig_handler);

	av_v4l2_open(flags);
	//open device twice
	video_fd = driver.v4l2_avd_open();
	if (video_fd < 0) {
		printf("2.Unable to open %s,err:%s\n", AV_V4L2_DEV_PATH,
			strerror(errno));
		printf("exited,%s:%d\n", __func__, __LINE__);
	}
	close(video_fd);

	gettimeofday(&before, NULL);
	av_v4l2_s_ctrl(V4L2_CID_EXT_AVD_PORT, 2);
	gettimeofday(&after, NULL);
	delta_time = 1000 * 1000 * (after.tv_sec - before.tv_sec)
		+ (after.tv_usec - before.tv_usec);
	printf("ioctl takes delta_time:%d us\n", delta_time);
	av_v4l2_g_ctrl(V4L2_CID_EXT_AVD_PORT);

	avd_testcase_202();

	gettimeofday(&before, NULL);
	av_v4l2_s_intput(V4L2_EXT_AVD_INPUT_SRC_AV);
	gettimeofday(&after, NULL);
	delta_time = 1000 * 1000 * (after.tv_sec - before.tv_sec)
			+ (after.tv_usec - before.tv_usec);
	printf("av_v4l2_s_intput takes delta_time:%d us\n", delta_time);

	av_v4l2_g_intput(&input_source);
	WriteSysfs("/sys/class/graphics/fb0/blank", 0);
	//wait signal stable....
	wait_cnt = 100;
	while (isRunning && wait_cnt--)
	{
		ret = av_v4l2_g_avd_timing_info(&timing_info);
		if (ret < 0)
		{
			printf("%s:%d,av_v4l2_g_avd_timing_info failed\n", __func__, __LINE__);
			break;
		}

		if (timing_info.active.w != 0 && timing_info.active.h != 0)
			break;
		usleep(1000*1000);//1s
	}
	printf("freq[h=%d,v=%d],porch[h=%d,v=%d],active[%d %d %d %d],lock[vd:%d,h:%d,v:%d]\n",
		timing_info.h_freq, timing_info.v_freq, timing_info.h_porch, timing_info.v_porch,
		timing_info.active.x, timing_info.active.y, timing_info.active.w, timing_info.active.h,
		timing_info.vd_lock, timing_info.h_lock, timing_info.v_lock);
	av_v4l2_s_ctrl(V4L2_CID_EXT_AVD_AUTO_TUNING_MODE, 3);
	av_v4l2_g_ctrl(V4L2_CID_EXT_AVD_AUTO_TUNING_MODE);
	av_v4l2_g_ctrl(V4L2_CID_EXT_AVD_VIDEO_SYNC);
	av_v4l2_s_std(V4L2_STD_NTSC);
	std = av_v4l2_g_std();
	getchar();

	av_v4l2_s_intput(V4L2_EXT_AVD_INPUT_SRC_NONE);
	av_v4l2_g_intput(&input_source);
	av_v4l2_querycap();
	av_v4l2_close();

	printf("exited,%s:%d,std=%d\n", __func__, __LINE__, std);

	return 0;
}

