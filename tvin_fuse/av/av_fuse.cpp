/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_MOUDLE_TAG "TVIN-FUSE"
#define LOG_CLASS_TAG "AV_FUSE"

#include "../common.h"
#include "av_fuse.h"

#ifdef __cplusplus
extern "C" {
#endif

pthread_mutex_t av_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_lock(&av_mutex);
//pthread_mutex_unlock(&av_mutex);

static void av_cuse_open(fuse_req_t req, struct fuse_file_info *fi)
{
	int ret = 0;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct av_obj_s *pav_obj = (struct av_obj_s *)(pcuse_obj->pri);

	pthread_mutex_lock(&av_mutex);
	ret = av_drv_open(pav_obj, fi->flags);
	if (ret == 0)
		fuse_reply_open(req, fi);
	else
		fuse_reply_err(req, ret);
	pthread_mutex_unlock(&av_mutex);
}

static void av_cuse_release(fuse_req_t req, struct fuse_file_info *fi)
{
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct av_obj_s *pav_obj = (struct av_obj_s *)(pcuse_obj->pri);

	pthread_mutex_lock(&av_mutex);
	av_drv_close(pav_obj);
	fuse_reply_buf(req, NULL, 0);
	pthread_mutex_unlock(&av_mutex);
}

static void av_cuse_read(fuse_req_t req, size_t size, off_t off,
		      struct fuse_file_info *fi)
{
	LOGD("%s,%d\n",__func__, __LINE__);
	fuse_reply_buf(req, NULL, 0);
}

static void av_cuse_write(fuse_req_t req, const char *buf, size_t size, off_t off,
		       struct fuse_file_info *fi)
{
	LOGD("%s,%d\n",__func__, __LINE__);
    fuse_reply_write(req, size);
}

static void av_cuse_g_ctrls(fuse_req_t req, const void *in_buf)
{
	int ret = 0;
	struct v4l2_control *control = (struct v4l2_control *)in_buf;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct av_obj_s *pav_obj = (struct av_obj_s *)(pcuse_obj->pri);

	LOGD("%s,%d\n",__func__, __LINE__);

    if (!control) {
        fuse_reply_err(req, EINVAL);
        return;
    }

    switch (control->id) {
        case V4L2_CID_EXT_AVD_PORT:
			control->value = av_drv_g_ext_avd_port(pav_obj);
            break;
		case V4L2_CID_EXT_AVD_AUTO_TUNING_MODE:
			control->value = av_drv_g_ext_avd_auto_tuning_mode(pav_obj);
			break;
		case V4L2_CID_EXT_AVD_VIDEO_SYNC:
			control->value = av_drv_g_ext_avd_video_sync(pav_obj);
			break;
        default:
            break;
    }
    fuse_reply_ioctl(req, ret, control, sizeof(*control));
}

static void av_cuse_s_ctrls(fuse_req_t req, const void *in_buf)
{
	int ret = 0;
	struct v4l2_control *control = (struct v4l2_control *)in_buf;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct av_obj_s *pav_obj = (struct av_obj_s *)(pcuse_obj->pri);

	if (!control) {
		fuse_reply_err(req, EINVAL);
		return;
	}

	switch (control->id) {
		case V4L2_CID_EXT_AVD_PORT:
			ret = av_drv_s_ext_avd_port(pav_obj, control->value);
			break;
		case V4L2_CID_EXT_AVD_AUTO_TUNING_MODE:
			ret = av_drv_s_ext_avd_auto_tuning_mode(pav_obj, control->value);
			break;
		case V4L2_CID_EXT_AVD_CHANNEL_CHANGE:
			ret = av_drv_s_ext_avd_channel_change(pav_obj, control->value);
			break;
		default:
			break;
	}
	fuse_reply_ioctl(req, ret, control, sizeof(*control));
}

static void av_cuse_g_ext_ctrls(fuse_req_t req, void *arg, const void *in_buf,
    size_t in_bufsz, size_t out_bufsz)
{
    int ret = 0;
    struct iovec iov_r[2], iov_w, iov;
    struct v4l2_ext_controls *ext_controls;
    struct v4l2_ext_control *ext_control;
	struct v4l2_ext_avd_timing_info avd_timing_info;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct av_obj_s *pav_obj = (struct av_obj_s *)(pcuse_obj->pri);

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

    iov_w.iov_base = ext_control->ptr;

    switch (ext_control->id) {
        case V4L2_CID_EXT_AVD_TIMING_INFO:
            iov_w.iov_len = sizeof(struct v4l2_ext_avd_timing_info);
            if (!out_bufsz) {
                fuse_reply_ioctl_retry(req, iov_r, 2, &iov_w, 1);
                return;
            }
			/* negative_test,out_bufsz:14,info:14,ctl_size:15 */
			LOGD("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_avd_timing_info), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_avd_timing_info))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			ret = av_drv_g_ext_avd_timing_info(pav_obj, &avd_timing_info);
            iov.iov_base = &avd_timing_info;
            iov.iov_len = sizeof(struct v4l2_ext_avd_timing_info);
            break;
        default:
            break;
    }
    fuse_reply_ioctl_iov(req, ret, &iov, 1);
}

static void av_cuse_s_ext_ctrls(fuse_req_t req, void *arg,
    const void *in_buf, size_t in_bufsz)
{
    struct iovec iov_r[3];
    struct v4l2_ext_controls *ext_controls;
    struct v4l2_ext_control *ext_control;
    int ret = 0;

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
        case V4L2_CID_EXT_AVD_TIMING_INFO://for test
            iov_r[2].iov_base = ext_control->ptr;
            iov_r[2].iov_len = sizeof(struct v4l2_ext_avd_timing_info);
            if (!in_bufsz) {
                fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
                return;
            }
            //ret = av_g_ext_avd_timing_info(req, in_buf);
            break;
        default:
            break;
    }
    fuse_reply_ioctl_iov(req, ret, NULL, 0);
}

static void av_cuse_ioctl(fuse_req_t req, int cmd, void *arg,
		       struct fuse_file_info *fi, unsigned int flags,
		       const void *in_buf, size_t in_bufsz, size_t out_bufsz)
{
	int ret = 0;
	struct iovec in_iov, out_iov;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct av_obj_s *pav_obj = (struct av_obj_s *)(pcuse_obj->pri);

	pthread_mutex_lock(&av_mutex);
	switch (cmd) {
		case VIDIOC_S_INPUT:
			in_iov.iov_base = (void *)arg;
			in_iov.iov_len = sizeof(int);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, &in_iov, 1, NULL, 0);
				break;
			}
			ret = av_drv_s_input(pav_obj, *(enum v4l2_ext_avd_input_src *)in_buf);
			fuse_reply_ioctl(req, ret, NULL, 0);
			break;
		case VIDIOC_G_INPUT:
			if (!out_bufsz) {
				out_iov.iov_base = (void *)arg;
				out_iov.iov_len = sizeof(int);
				fuse_reply_ioctl_retry(req, NULL, 0, &out_iov, 1);
				break;
			}
			ret = av_drv_g_input(pav_obj);
			fuse_reply_ioctl(req, 0, &ret, sizeof(ret));
			break;
		case VIDIOC_S_STD:
			in_iov.iov_base = (void *)arg;
			in_iov.iov_len = sizeof(v4l2_std_id);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, &in_iov, 1, NULL, 0);
				break;
			}

			ret = av_drv_s_std(pav_obj, (*(v4l2_std_id *)in_buf));
			fuse_reply_ioctl(req, ret, NULL, 0);
			break;
		case VIDIOC_G_STD:
			if (!out_bufsz) {
				out_iov.iov_base = (void *)arg;
				out_iov.iov_len = sizeof(v4l2_std_id);
				fuse_reply_ioctl_retry(req, NULL, 0, &out_iov, 1);
				break;
			}
			v4l2_std_id id;
			id = av_drv_g_std(pav_obj);
			fuse_reply_ioctl(req, ret, &id, sizeof(id));
			break;
		case VIDIOC_S_CTRL:
			if (!in_bufsz || !out_bufsz) {
				struct iovec iov_r = { arg, sizeof(struct v4l2_control) };
				struct iovec iov_w = { arg, sizeof(struct v4l2_control) };
				fuse_reply_ioctl_retry(req, &iov_r, 1, &iov_w, 1);
			} else {
				av_cuse_s_ctrls(req, in_buf);
			}
			break;
		case VIDIOC_G_CTRL:
			if (!in_bufsz || !out_bufsz) {
				in_iov.iov_base = (void *)arg;
				in_iov.iov_len = sizeof(struct v4l2_control);
				out_iov.iov_base = (void *)arg;
				out_iov.iov_len = sizeof(struct v4l2_control);
				fuse_reply_ioctl_retry(req, &in_iov, 1, &out_iov, 1);
			} else {
				av_cuse_g_ctrls(req, in_buf);
			}
			break;
		case VIDIOC_S_EXT_CTRLS:
			av_cuse_s_ext_ctrls(req, arg, in_buf, in_bufsz);
			break;
		case VIDIOC_G_EXT_CTRLS:
			av_cuse_g_ext_ctrls(req, arg, in_buf, in_bufsz, out_bufsz);
			break;
		case VIDIOC_QUERYCAP:
			if (!in_bufsz || !out_bufsz) {
				in_iov.iov_base = (void *)arg;
				in_iov.iov_len = sizeof(struct v4l2_capability);
				out_iov.iov_base = (void *)arg;
				out_iov.iov_len = sizeof(struct v4l2_capability);
				fuse_reply_ioctl_retry(req, &in_iov, 1, &out_iov, 1);
				break;
			}
			struct v4l2_capability cap;
			memset(&cap, 0, sizeof(cap));
			ret = av_drv_querycap(pav_obj, &cap);

			fuse_reply_ioctl(req, ret, &cap, sizeof(struct v4l2_capability));
			break;
		default:
			fuse_reply_err(req, EINVAL);
			break;
	}
	pthread_mutex_unlock(&av_mutex);
}

static struct cuse_lowlevel_ops av_cuse_ops = {
	.open		= av_cuse_open,
	.read		= av_cuse_read,
	.write		= av_cuse_write,
	.release    = av_cuse_release,
	.ioctl		= av_cuse_ioctl,
};

struct av_obj_s av_obj = {
	.obj_name   = "av",
	.av_drv_fd = -1,
	.source_input = SOURCE_INVALID,
	.mp_VdinDrvIf = NULL,
};

struct cuse_obj av_cuse = {
	.dev_name 	= "DEVNAME=video10",
	.p_cuse_ops = &av_cuse_ops,
	.pri        = &av_obj,
};

#ifdef __cplusplus
}
#endif
