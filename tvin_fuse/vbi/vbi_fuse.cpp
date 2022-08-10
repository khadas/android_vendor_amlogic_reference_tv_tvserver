/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_MOUDLE_TAG "TVIN-CVBS"
#define LOG_CLASS_TAG "TVIN-CUSE"

#include "../common.h"
#include "vbi_fuse.h"

#ifdef __cplusplus
extern "C" {
#endif

pthread_mutex_t vbi_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_lock(&vbi_mutex);
//pthread_mutex_unlock(&vbi_mutex);

static void vbi_cuse_open(fuse_req_t req, struct fuse_file_info *fi)
{
	int ret = 0;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct vbi_obj_s *pvbi_obj = (struct vbi_obj_s *)(pcuse_obj->pri);

	ret = vbi_drv_open(pvbi_obj, fi->flags);
	if (ret == 0)
		fuse_reply_open(req, fi);
	else
		fuse_reply_err(req, ret);
}

static void vbi_cuse_release(fuse_req_t req, struct fuse_file_info *fi)
{
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct vbi_obj_s *pvbi_obj = (struct vbi_obj_s *)(pcuse_obj->pri);

	vbi_drv_close(pvbi_obj);

	fuse_reply_buf(req, NULL, 0);
}

static void vbi_cuse_read(fuse_req_t req, size_t size, off_t off,
		      struct fuse_file_info *fi)
{
	char *buf = NULL;
	size_t out_size = 0;

	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct vbi_obj_s *pvbi_obj = (struct vbi_obj_s *)(pcuse_obj->pri);

	printf("%s,%d\n",__func__, __LINE__);
	vbi_drv_read(pvbi_obj, size, off, buf, out_size);
	fuse_reply_buf(req, buf, out_size);
}

static void vbi_cuse_write(fuse_req_t req, const char *buf, size_t size, off_t off,
		       struct fuse_file_info *fi)
{
	printf("%s,%d\n",__func__, __LINE__);
    fuse_reply_write(req, size);
}

static void vbi_cuse_g_ctrls(fuse_req_t req, const void *in_buf)
{
	int ret = 0;
	struct v4l2_control *control = (struct v4l2_control *)in_buf;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct vbi_obj_s *pvbi_obj = (struct vbi_obj_s *)(pcuse_obj->pri);

	printf("%s,%d\n",__func__, __LINE__);

    if (!control) {
        fuse_reply_err(req, EINVAL);
        return;
    }

    switch (control->id) {
        default:
            break;
    }
    fuse_reply_ioctl(req, ret, control, sizeof(*control));
}

static void vbi_cuse_s_ctrls(fuse_req_t req, const void *in_buf)
{
	int ret = 0;
	struct v4l2_control *control = (struct v4l2_control *)in_buf;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct vbi_obj_s *pvbi_obj = (struct vbi_obj_s *)(pcuse_obj->pri);

	if (!control) {
		fuse_reply_err(req, EINVAL);
		return;
	}

	switch (control->id) {
		case V4L2_CID_EXT_VBI_FLUSH:
			ret = vbi_drv_s_ext_vbi_flush(pvbi_obj, control->value);
			break;
		default:
			break;
	}
	fuse_reply_ioctl(req, ret, control, sizeof(*control));
}

static void vbi_cuse_g_ext_ctrls(fuse_req_t req, void *arg, const void *in_buf,
    size_t in_bufsz, size_t out_bufsz)
{
    int ret = 0;
    struct iovec iov_r[2], iov_w, iov;
    struct v4l2_ext_controls *ext_controls;
    struct v4l2_ext_control *ext_control;
	struct v4l2_ext_avd_timing_info avd_timing_info;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct vbi_obj_s *pvbi_obj = (struct vbi_obj_s *)(pcuse_obj->pri);

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
        case V4L2_CID_EXT_VBI_COPY_PROTECTION_INFO:
            iov_w.iov_len = sizeof(struct v4l2_ext_vbi_copy_protection);
            if (!out_bufsz) {
                fuse_reply_ioctl_retry(req, iov_r, 2, &iov_w, 1);
                return;
            }
			struct v4l2_ext_vbi_copy_protection info;
			printf("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_vbi_copy_protection), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_vbi_copy_protection))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			ret = vbi_drv_g_ext_vbi_copy_protection_info(pvbi_obj, &info);
			iov.iov_base = &info;
			iov.iov_len = sizeof(info);
			break;
		default:
			break;
	}
	fuse_reply_ioctl_iov(req, ret, &iov, 1);
}

static void vbi_cuse_s_ext_ctrls(fuse_req_t req, void *arg,
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
        default:
            break;
    }
    fuse_reply_ioctl_iov(req, ret, NULL, 0);
}

static void vbi_cuse_ioctl(fuse_req_t req, int cmd, void *arg,
		       struct fuse_file_info *fi, unsigned int flags,
		       const void *in_buf, size_t in_bufsz, size_t out_bufsz)
{
	int ret = 0;
	struct iovec in_iov, out_iov;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct vbi_obj_s *pvbi_obj = (struct vbi_obj_s *)(pcuse_obj->pri);
	struct v4l2_format fmt;

	switch (cmd) {
		case VIDIOC_S_FMT:
			printf("line %d,in_bufsz:%d,out_bufsz:%d\n", __LINE__, in_bufsz, out_bufsz);
			in_iov.iov_base = (void *)arg;
			in_iov.iov_len = sizeof(fmt);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, &in_iov, 1, NULL, 0);
				return;
			}
			ret = vbi_drv_s_fmt(pvbi_obj, (struct v4l2_format *)in_buf);
			fuse_reply_ioctl(req, ret, NULL, 0);
			break;
		case VIDIOC_G_FMT:
			printf("line %d,in_bufsz:%d,out_bufsz:%d\n", __LINE__, in_bufsz, out_bufsz);
			/* in_buf=NULL,in_bufsz=0 */
			in_iov.iov_base = (void *)arg;
			in_iov.iov_len = sizeof(fmt);
			out_iov.iov_base = (void *)arg;
			out_iov.iov_len = sizeof(fmt);
			if (!in_bufsz || !out_bufsz) {
				fuse_reply_ioctl_retry(req, &in_iov, 1, &out_iov, 1);
				return;
			}

			memcpy(&fmt, (void *)in_buf, sizeof(fmt));
			ret = vbi_drv_g_fmt(pvbi_obj, &fmt);
			fuse_reply_ioctl(req, ret, &fmt, sizeof(fmt));

			break;
		case VIDIOC_STREAMON:
			in_iov.iov_base = (void *)arg;
			in_iov.iov_len = sizeof(int);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, &in_iov, 1, NULL, 0);
				return;
			}
			ret = vbi_drv_streamon(pvbi_obj, *(int *)in_buf);
			fuse_reply_ioctl(req, ret, NULL, 0);
			break;
		case VIDIOC_STREAMOFF:
			in_iov.iov_base = (void *)arg;
			in_iov.iov_len = sizeof(int);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, &in_iov, 1, NULL, 0);
				return;
			}
			ret = vbi_drv_streamoff(pvbi_obj, *(int *)in_buf);
			fuse_reply_ioctl(req, ret, NULL, 0);
			break;
		case VIDIOC_S_CTRL:
			if (!in_bufsz || !out_bufsz) {
				struct iovec iov_r = { arg, sizeof(struct v4l2_control) };
				struct iovec iov_w = { arg, sizeof(struct v4l2_control) };
				fuse_reply_ioctl_retry(req, &iov_r, 1, &iov_w, 1);
			} else {
				vbi_cuse_s_ctrls(req, in_buf);
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
				vbi_cuse_g_ctrls(req, in_buf);
			}
			break;
		case VIDIOC_S_EXT_CTRLS:
			vbi_cuse_s_ext_ctrls(req, arg, in_buf, in_bufsz);
			break;
		case VIDIOC_G_EXT_CTRLS:
			vbi_cuse_g_ext_ctrls(req, arg, in_buf, in_bufsz, out_bufsz);
			break;
		default:
			fuse_reply_err(req, EINVAL);
			break;
	}
}

static struct cuse_lowlevel_ops vbi_cuse_ops = {
	.open		= vbi_cuse_open,
	.read		= vbi_cuse_read,
	.write		= vbi_cuse_write,
	.release	= vbi_cuse_release,
	.ioctl		= vbi_cuse_ioctl,
};

struct vbi_obj_s vbi_obj = {
	.obj_name   = "vbi",
	.video_fd = -1,
};

struct cuse_obj vbi_cuse = {
	.dev_name 	= "DEVNAME=vbi1",
	.p_cuse_ops = &vbi_cuse_ops,
	.pri        = &vbi_obj,
};

#ifdef __cplusplus
}
#endif
