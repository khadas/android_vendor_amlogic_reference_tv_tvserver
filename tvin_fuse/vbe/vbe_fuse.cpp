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
#include "vbe_fuse.h"

#ifdef __cplusplus
extern "C" {
#endif

pthread_mutex_t vbe_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_lock(&vbe_mutex);
//pthread_mutex_unlock(&vbe_mutex);

static void vbe_cuse_open(fuse_req_t req, struct fuse_file_info *fi)
{
	int ret = 0;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct vbe_obj_s *pvbe_obj = (struct vbe_obj_s *)(pcuse_obj->pri);

	ret = vbe_drv_open(pvbe_obj, fi->flags);
	if (ret == 0)
		fuse_reply_open(req, fi);
	else
		fuse_reply_err(req, ret);
}

static void vbe_cuse_release(fuse_req_t req, struct fuse_file_info *fi)
{
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct vbe_obj_s *pvbe_obj = (struct vbe_obj_s *)(pcuse_obj->pri);

	vbe_drv_close(pvbe_obj);

	fuse_reply_buf(req, NULL, 0);
}

static void vbe_cuse_read(fuse_req_t req, size_t size, off_t off,
		      struct fuse_file_info *fi)
{
	printf("%s,%d\n",__func__, __LINE__);
	fuse_reply_buf(req, NULL, 0);
}

static void vbe_cuse_write(fuse_req_t req, const char *buf, size_t size, off_t off,
		       struct fuse_file_info *fi)
{
	printf("%s,%d\n",__func__, __LINE__);
    fuse_reply_write(req, size);
}

static void vbe_cuse_g_ctrl(fuse_req_t req, const void *in_buf)
{
	int ret = 0;
	struct v4l2_control *control = (struct v4l2_control *)in_buf;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct vbe_obj_s *pvbe_obj = (struct vbe_obj_s *)(pcuse_obj->pri);

	printf("%s,%d\n",__func__, __LINE__);

    if (!control) {
        fuse_reply_err(req, EINVAL);
        return;
    }

    switch (control->id) {
		case V4L2_CID_EXT_VBE_GSR_STATUS:
			control->value = vbi_drv_g_ext_vbe_gsr_status(pvbe_obj);
			break;
		case V4L2_CID_EXT_VBE_LOCK_STATUS:
			control->value = vbi_drv_g_ext_vbe_lock_status(pvbe_obj);
			break;
		case V4L2_CID_EXT_VBE_GSR_STRESS_COUNT:
			control->value = vbi_drv_g_ext_vbe_gsr_stress_count(pvbe_obj);
			break;
		case V4L2_CID_EXT_VBE_IRR_RESULT:
			control->value = vbi_drv_g_ext_vbe_irr_result(pvbe_obj);
			break;

        default:
            break;
    }
    fuse_reply_ioctl(req, ret, control, sizeof(*control));
}

static void vbe_cuse_s_ctrl(fuse_req_t req, const void *in_buf)
{
	int ret = 0;
	struct v4l2_control *control = (struct v4l2_control *)in_buf;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct vbe_obj_s *pvbe_obj = (struct vbe_obj_s *)(pcuse_obj->pri);

	if (!control) {
		fuse_reply_err(req, EINVAL);
		return;
	}

	switch (control->id) {
		case V4L2_CID_EXT_VBE_MUTE:
			ret = vbe_drv_s_ext_vbe_mute(pvbe_obj, control->value);
			break;
		case V4L2_CID_EXT_VBE_MIRROR:
			ret = vbe_drv_s_ext_vbe_mirror(pvbe_obj, control->value);
			break;
		case V4L2_CID_EXT_VBE_MPLUS_BOEBYPASS:
			ret = vbi_drv_s_ext_vbe_mplus_boebypass(pvbe_obj, control->value);
			break;
		case V4L2_CID_EXT_VBE_CPC:
			ret = vbi_drv_s_ext_vbe_mplus_boebypass(pvbe_obj, control->value);
			break;

		default:
			break;
	}
	fuse_reply_ioctl(req, ret, control, sizeof(*control));
}

static void vbe_cuse_g_ext_ctrls(fuse_req_t req, void *arg, const void *in_buf,
    size_t in_bufsz, size_t out_bufsz)
{
    int ret = 0;
    struct iovec iov_r[2], iov_w, iov;
    struct v4l2_ext_controls *ext_controls;
    struct v4l2_ext_control *ext_control;
	struct v4l2_ext_avd_timing_info avd_timing_info;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct vbe_obj_s *pvbe_obj = (struct vbe_obj_s *)(pcuse_obj->pri);

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
		case V4L2_CID_EXT_VBE_ALPHA_OSD:
			break;
		case V4L2_CID_EXT_VBE_MPLUS_PARAM:
            iov_w.iov_len = sizeof(struct v4l2_ext_vbe_mplus_param);
            if (!out_bufsz) {
                fuse_reply_ioctl_retry(req, iov_r, 2, &iov_w, 1);
                return;
            }
			struct v4l2_ext_vbe_mplus_param param;
			printf("out_bufsz:%x,info:%x,ctl_size:%x\n",out_bufsz,
				sizeof(struct v4l2_ext_vbe_mplus_param), ext_control->size);
			if (ext_control->size != sizeof(struct v4l2_ext_vbe_mplus_param))
			{
				fuse_reply_err(req, EINVAL);
				return;
			}
			ret = vbi_drv_g_ext_vbe_mplus_param(pvbe_obj, &param);
			iov.iov_base = &param;
			iov.iov_len = sizeof(param);

			break;
		case V4L2_CID_EXT_VBE_PWM_PARAM:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_DEMURA:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_PCLRC:
			//TODO
			break;

		default:
			break;
	}
	fuse_reply_ioctl_iov(req, ret, &iov, 1);
}

static void vbe_cuse_s_ext_ctrls(fuse_req_t req, void *arg,
    const void *in_buf, size_t in_bufsz)
{
	int ret = 0;
	struct iovec iov_r[3];
	struct v4l2_ext_controls *ext_controls;
	struct v4l2_ext_control *ext_control;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct vbe_obj_s *pvbe_obj = (struct vbe_obj_s *)(pcuse_obj->pri);

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
        case V4L2_CID_EXT_VBE_INIT:
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len = sizeof(struct v4l2_ext_vbe_panel_info);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			ret = vbi_drv_s_ext_vbe_init(pvbe_obj, (struct v4l2_ext_vbe_panel_info *)in_buf);
			break;
		case V4L2_CID_EXT_VBE_RESUME:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_DISPLAYOUTPUT:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_SSC:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_MPLUS_MODE:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_DGA4CH:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_MPLUS_PARAM://get
			iov_r[2].iov_base = ext_control->ptr;
			iov_r[2].iov_len = sizeof(struct v4l2_ext_vbe_mplus_param);
			if (!in_bufsz) {
				fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
				return;
			}
			ret = vbi_drv_s_ext_vbe_mplus_param(pvbe_obj, (struct v4l2_ext_vbe_mplus_param *)in_buf);
			break;
			break;
		case V4L2_CID_EXT_VBE_INNER_PATTERN:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_TSCIC:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_EPI_SCRAMBLE:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_EPI_10BIT:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_MPLUS_DATA:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_PWM_INIT:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_PWM_PARAM://get
			//TODO
			break;
		case V4L2_CID_EXT_VBE_PWM_APPLY_PARAM:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_PWM_SET_DUTY:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_VCOM_PAT_CTRL:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_VCOM_PAT_DRAW:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_ORBIT:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_LSR:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_GSR:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_OSD_GAIN:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_DEMURA://get
			//TODO
			break;
		case V4L2_CID_EXT_VBE_PCLRC://get
			//TODO
			break;
		case V4L2_CID_EXT_VBE_GSR2:
			//TODO
			break;
		case V4L2_CID_EXT_VBE_IRR_ADAPTEDLUM:
			//TODO
			break;
		//case V4L2_CID_EXT_VBE_OUTPUT_BYPASS:
		//	break;
		default:
			break;
	}
	fuse_reply_ioctl_iov(req, ret, NULL, 0);
}

static void vbe_cuse_ioctl(fuse_req_t req, int cmd, void *arg,
		       struct fuse_file_info *fi, unsigned int flags,
		       const void *in_buf, size_t in_bufsz, size_t out_bufsz)
{
	int ret = 0;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct vbe_obj_s *pvbe_obj = (struct vbe_obj_s *)(pcuse_obj->pri);

	switch (cmd) {
		case VIDIOC_S_CTRL:
			if (!in_bufsz || !out_bufsz) {
				struct iovec iov_r = { arg, sizeof(struct v4l2_control) };
				struct iovec iov_w = { arg, sizeof(struct v4l2_control) };
				fuse_reply_ioctl_retry(req, &iov_r, 1, &iov_w, 1);
			} else {
				vbe_cuse_s_ctrl(req, in_buf);
			}
			break;
		case VIDIOC_G_CTRL:
			if (!in_bufsz || !out_bufsz) {
				struct iovec iov_r = { arg, sizeof(struct v4l2_control) };
				struct iovec iov_w = { arg, sizeof(struct v4l2_control) };
				fuse_reply_ioctl_retry(req, &iov_r, 1, &iov_w, 1);
			} else {
				vbe_cuse_g_ctrl(req, in_buf);
			}
			break;
		case VIDIOC_S_EXT_CTRLS:
			vbe_cuse_s_ext_ctrls(req, arg, in_buf, in_bufsz);
			break;
		case VIDIOC_G_EXT_CTRLS:
			vbe_cuse_g_ext_ctrls(req, arg, in_buf, in_bufsz, out_bufsz);
			break;
		default:
			fuse_reply_err(req, EINVAL);
			break;
	}
}

static struct cuse_lowlevel_ops vbe_cuse_ops = {
	.open		= vbe_cuse_open,
	.read		= vbe_cuse_read,
	.write		= vbe_cuse_write,
	.release	= vbe_cuse_release,
	.ioctl		= vbe_cuse_ioctl,
};

struct vbe_obj_s vbe_obj = {
	.obj_name   = "vbe",
	.vbe_drv_fd = -1,
};

struct cuse_obj vbe_cuse = {
	.dev_name 	= "DEVNAME=video40",
	.p_cuse_ops = &vbe_cuse_ops,
	.pri        = &vbe_obj,
};

#ifdef __cplusplus
}
#endif
