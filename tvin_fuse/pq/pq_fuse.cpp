/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_MOUDLE_TAG "TVIN-PQ"
#define LOG_CLASS_TAG "TVIN-CUSE"

#include "../common.h"
#include "pq_fuse.h"

#ifdef __cplusplus
extern "C" {
#endif

pthread_mutex_t pq_mutex = PTHREAD_MUTEX_INITIALIZER;

static void pq_cuse_open(fuse_req_t req, struct fuse_file_info *fi)
{
	int ret = 0;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct pq_obj_s *pobj = (struct pq_obj_s *)(pcuse_obj->pri);

	printf("%s, in\n", __func__);

	pthread_mutex_lock(&pq_mutex);

	ret = pq_drv_open(pobj, fi->flags);
	if (ret == 0)
		fuse_reply_open(req, fi);
	else
		fuse_reply_err(req, ret);

	pthread_mutex_unlock(&pq_mutex);
}

static void pq_cuse_release(fuse_req_t req, struct fuse_file_info *fi)
{
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct pq_obj_s *pobj = (struct pq_obj_s *)(pcuse_obj->pri);

	pthread_mutex_lock(&pq_mutex);

	pq_drv_close(pobj);

	fuse_reply_buf(req, NULL, 0);

	pthread_mutex_unlock(&pq_mutex);
}

static void pq_cuse_read(fuse_req_t req, size_t size, off_t off,
	struct fuse_file_info *fi)
{
	printf("%s, %d\n", __func__, __LINE__);
	fuse_reply_buf(req, NULL, 0);
}

static void pq_cuse_write(fuse_req_t req, const char *buf,
	size_t size, off_t off, struct fuse_file_info *fi)
{
	printf("%s, %d\n", __func__, __LINE__);
	fuse_reply_write(req, size);
}

static void pq_cuse_s_ctrl(fuse_req_t req, const void *in_buf)
{
	int ret = 0;
	struct v4l2_control *control = (struct v4l2_control *)in_buf;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct pq_obj_s *pobj = (struct pq_obj_s *)(pcuse_obj->pri);

	if (!control) {
		fuse_reply_err(req, EINVAL);
		printf("%s, %d, fuse_reply_err return\n", __func__, __LINE__);
		return;
	}

	printf("%s, %d, control->id = %d\n",
		__func__, __LINE__, control->id);

	switch (control->id) {
	case V4L2_CID_EXT_VPQ_INIT:
		ret = pq_drv_s_ext_init(pobj, control->value);
		break;
	case V4L2_CID_EXT_VPQ_LOW_DELAY_MODE:
		ret = pq_drv_s_ext_low_delay_mode(pobj, control->value);
		break;
	case V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_COLOR_GAIN:
		ret = pq_drv_s_ext_dc_color_gain(pobj, control->value);
		break;
	case V4L2_CID_EXT_VPQ_TESTPATTERN:
		break;
	case V4L2_CID_EXT_VPQ_REAL_CINEMA:
		ret = pq_drv_s_ext_real_cinema(pobj, control->value);
		break;
	case V4L2_CID_EXT_VPQ_STEREO_FACE_CTRL:
		ret = pq_drv_s_ext_stereo_face_ctrl(pobj, control->value);
		break;
	case V4L2_CID_EXT_HDR_INV_GAMMA:
		ret = pq_drv_s_ext_hdr_inv_gamma(pobj, control->value);
		break;
	case V4L2_CID_EXT_AIPQ_SQM_MODE:
		ret = pq_drv_s_ext_aipq_sqm_mode(pobj, control->value);
		break;
	case V4L2_CID_EXT_AIPQ_SR_MODE:
		ret = pq_drv_s_ext_aipq_sr_mode(pobj, control->value);
		break;
	case V4L2_CID_EXT_AIPQ_DEPTH_MODE:
		ret = pq_drv_s_ext_aipq_depth_mode(pobj, control->value);
		break;
	case V4L2_CID_EXT_AIPQ_SCENE_MODE:
		ret = pq_drv_s_ext_aipq_scene_mode(pobj, control->value);
		break;
	case V4L2_CID_EXT_AIPQ_GENRE_MODE:
		ret = pq_drv_s_ext_aipq_genre_mode(pobj, control->value);
		break;
	case V4L2_CID_EXT_MEMC_INIT:
		ret = pq_drv_s_ext_memc_init(pobj, control->value);
		break;
	case V4L2_CID_EXT_MEMC_LOW_DELAY_MODE:
		ret = pq_drv_s_ext_memc_low_delay_mode(pobj, control->value);
		break;
	case V4L2_CID_EXT_MEMC_MOTION_PRO:
		ret = pq_drv_s_ext_memc_motion_pro(pobj, control->value);
		break;
	case V4L2_CID_EXT_LED_EN:
		ret = pq_drv_s_ext_led_en(pobj, control->value);
		break;
	case V4L2_CID_EXT_LED_DB_IDX:
		ret = pq_drv_s_ext_led_db_idx(pobj, control->value);
		break;
	case V4L2_CID_EXT_LED_FIN:
		break;
	default:
		break;
	}

	fuse_reply_ioctl(req, ret, control, sizeof(*control));
}

static void pq_cuse_g_ctrl(fuse_req_t req, const void *in_buf)
{
	int ret = 0;
	int tmp = 0;
	struct v4l2_control *control = (struct v4l2_control *)in_buf;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct pq_obj_s *pobj = (struct pq_obj_s *)(pcuse_obj->pri);

	if (!control) {
		fuse_reply_err(req, EINVAL);
		printf("%s, %d, fuse_reply_err return\n", __func__, __LINE__);
		return;
	}

	printf("%s, %d, control->id = %d\n",
		__func__, __LINE__, control->id);

	switch (control->id) {
	case V4L2_CID_EXT_VPQ_LOW_DELAY_MODE:
		ret = pq_drv_g_ext_low_delay_mode(pobj, &tmp);
		control->value = tmp;
		break;
	case V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_COLOR_GAIN:
		ret = pq_drv_g_ext_dc_color_gain(pobj, &tmp);
		control->value = tmp;
		break;
	case V4L2_CID_EXT_VPQ_TESTPATTERN:
		break;
	case V4L2_CID_EXT_VPQ_REAL_CINEMA:
		ret = pq_drv_g_ext_real_cinema(pobj, &tmp);
		control->value = tmp;
		break;
	case V4L2_CID_EXT_VPQ_STEREO_FACE_CTRL:
		ret = pq_drv_g_ext_stereo_face_ctrl(pobj, &tmp);
		control->value = tmp;
		break;
	case V4L2_CID_EXT_VPQ_VIDEO_LATENCY:
		ret = pq_drv_g_ext_video_latency(pobj, &tmp);
		control->value = tmp;
		break;
	case V4L2_CID_EXT_AIPQ_SQM_MODE:
		ret = pq_drv_g_ext_aipq_sqm_mode(pobj, &tmp);
		control->value = tmp;
		break;
	case V4L2_CID_EXT_AIPQ_SR_MODE:
		ret = pq_drv_g_ext_aipq_sr_mode(pobj, &tmp);
		control->value = tmp;
		break;
	case V4L2_CID_EXT_AIPQ_DEPTH_MODE:
		ret = pq_drv_g_ext_aipq_depth_mode(pobj, &tmp);
		control->value = tmp;
		break;
	case V4L2_CID_EXT_AIPQ_SCENE_MODE:
		ret = pq_drv_g_ext_aipq_scene_mode(pobj, &tmp);
		control->value = tmp;
		break;
	case V4L2_CID_EXT_AIPQ_SCENE_INFO:
		ret = pq_drv_g_ext_aipq_scene_info(pobj, &tmp);
		control->value = tmp;
		break;
	case V4L2_CID_EXT_AIPQ_GENRE_MODE:
		ret = pq_drv_g_ext_aipq_genre_mode(pobj, &tmp);
		control->value = tmp;
		break;
	case V4L2_CID_EXT_AIPQ_GENRE_INFO:
		ret = pq_drv_g_ext_aipq_genre_info(pobj, &tmp);
		control->value = tmp;
		break;
	case V4L2_CID_EXT_MEMC_LOW_DELAY_MODE:
		ret = pq_drv_g_ext_memc_low_delay_mode(pobj, &tmp);
		control->value = tmp;
		break;
	case V4L2_CID_EXT_MEMC_MOTION_PRO:
		ret = pq_drv_g_ext_memc_motion_pro(pobj, &tmp);
		control->value = tmp;
		break;
	case V4L2_CID_EXT_LED_EN:
		ret = pq_drv_g_ext_led_en(pobj, &tmp);
		control->value = tmp;
		break;
	case V4L2_CID_EXT_LED_DB_IDX:
		ret = pq_drv_g_ext_led_db_idx(pobj, &tmp);
		control->value = tmp;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	fuse_reply_ioctl(req, ret, control, sizeof(*control));
}

static void pq_cuse_s_ext_ctrls(fuse_req_t req, void *arg,
	const void *in_buf, size_t in_bufsz)
{
	int ret = 0;
	struct iovec iov_r[4];
	struct v4l2_ext_controls *ext_controls;
	struct v4l2_ext_control *ext_control;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct pq_obj_s *pobj = (struct pq_obj_s *)(pcuse_obj->pri);
	struct v4l2_ext_vpq_cmn_data *pcmn_data;

	iov_r[0].iov_base = arg;
	iov_r[0].iov_len = sizeof(struct v4l2_ext_controls);

	if (!in_bufsz) {
		fuse_reply_ioctl_retry(req, iov_r, 1, NULL, 0);
		printf("%s, out1\n", __func__);
		return;
	}

	ext_controls = (struct v4l2_ext_controls *)in_buf;
	in_buf += sizeof(struct v4l2_ext_controls);
	in_bufsz -= sizeof(struct v4l2_ext_controls);

	iov_r[1].iov_base = ext_controls->controls;
	iov_r[1].iov_len = sizeof(struct v4l2_ext_control);

	if (!in_bufsz) {
		fuse_reply_ioctl_retry(req, iov_r, 2, NULL, 0);
		printf("%s, out2\n", __func__);
		return;
	}

	ext_control = (struct v4l2_ext_control *)in_buf;
	in_buf += sizeof(struct v4l2_ext_control);
	in_bufsz -= sizeof(struct v4l2_ext_control);

	iov_r[2].iov_base = ext_control->ptr;
	iov_r[2].iov_len = sizeof(struct v4l2_ext_vpq_cmn_data);
	if (!in_bufsz) {
		fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
		printf("%s, out3\n", __func__);
		return;
	}

	pcmn_data = (struct v4l2_ext_vpq_cmn_data *)in_buf;
	in_buf += sizeof(struct v4l2_ext_vpq_cmn_data);
	in_bufsz -= sizeof(struct v4l2_ext_vpq_cmn_data);

	iov_r[3].iov_base = pcmn_data->p_data;
	iov_r[3].iov_len  = pcmn_data->length;
	if (!in_bufsz) {
		fuse_reply_ioctl_retry(req, iov_r, 4, NULL, 0);
		printf("%s, out4\n", __func__);
		return;
	}

	pcmn_data->p_data = (unsigned char *)in_buf;

	printf("%s, in, cmd = %#x\n", __func__, ext_control->id);

	switch (ext_control->id) {
	case V4L2_CID_EXT_VPQ_PICTURE_CTRL:
		ret = pq_drv_s_ext_pic_ctrl(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_SHARPNESS:
		ret = pq_drv_s_ext_sharpness(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_SUPER_RESOLUTION:
		ret = pq_drv_s_ext_sr(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST:
		ret = pq_drv_s_ext_dynamic_contrast(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_LUT:
		ret = pq_drv_s_ext_dynamic_contrast_lut(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_CM_DB_DATA:
		ret = pq_drv_s_ext_cm_db_data(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_BYPASS_BLOCK:
		ret = pq_drv_s_ext_bypass_block(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_BLACK_LEVEL:
		ret = pq_drv_s_ext_black_level(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_GAMMA_DATA:
		ret = pq_drv_s_ext_gamma_data(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_COLORTEMP_DATA:
		ret = pq_drv_s_ext_colortemp_data(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_GAMUT_3DLUT:
		ret = pq_drv_s_ext_gamut_3dlut(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_OD_TABLE:
		ret = pq_drv_s_ext_od_table(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_OD_EXTENSION:
		ret = pq_drv_s_ext_od_extension(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_LOCALCONTRAST_TABLE:
		ret = pq_drv_s_ext_localcontrast_table(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_LOCALCONTRAST_DATA:
		ret = pq_drv_s_ext_localcontrast_data(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_PSP:
		ret = pq_drv_s_ext_psp(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_GAMUT_MATRIX_PRE:
		ret = pq_drv_s_ext_gamut_matrix_pre(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_GAMUT_MATRIX_POST:
		ret = pq_drv_s_ext_gamut_matrix_post(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_PQ_MODE_INFO:
		ret = pq_drv_s_ext_pq_mode_info(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_LUMINANCE_BOOST:
		ret = pq_drv_s_ext_luminance_boost(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_OBC_DATA:
		ret = pq_drv_s_ext_obc_data(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_OBC_LUT:
		ret = pq_drv_s_ext_obc_lut(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_OBC_CTRL:
		ret = pq_drv_s_ext_obc_ctrl(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_EXTRA_PATTERN:
		ret = pq_drv_s_ext_extra_pattern(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_DB_DATA:
		ret = pq_drv_s_ext_db_data(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_PHDR_APL_GAIN_LUT:
		ret = pq_drv_s_ext_phdr_apl_gain_lut(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_HDR_3DLUT:
		ret = pq_drv_s_ext_hdr_3dlut(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_HDR_EOTF:
		ret = pq_drv_s_ext_hdr_eotf(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_HDR_OETF:
		ret = pq_drv_s_ext_hdr_oetf(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_HDR_TONEMAP:
		ret = pq_drv_s_ext_hdr_tonemap(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_HDR_COLOR_CORRECTION:
		ret = pq_drv_s_ext_hdr_color_correction(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_HDR_HLG_Y_GAIN_TBL:
		ret = pq_drv_s_ext_hdr_hlg_ygain_tbl(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_NOISE_REDUCTION:
		ret = pq_drv_s_ext_nr(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_MPEG_NOISE_REDUCTION:
		ret = pq_drv_s_ext_mpeg_nr(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_DECONTOUR:
		ret = pq_drv_s_ext_decontour(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_MEMC_MOTION_COMP:
		ret = pq_drv_s_ext_memc_motion_comp(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_LED_INIT:
		ret = pq_drv_s_ext_led_init(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_LED_DEMOMODE:
		ret = pq_drv_s_ext_led_demomode(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_LED_DB_DATA:
		ret = pq_drv_s_ext_led_db_data(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_LED_CONTROL_SPI:
		ret = pq_drv_s_ext_led_control_spi(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_LED_BPL_DATA:
		break;
	default:
		fuse_reply_err(req, EINVAL);
		break;
	}

	fuse_reply_ioctl_iov(req, ret, NULL, 0);
}

static void pq_cuse_g_ext_ctrls(fuse_req_t req, void *arg,
	const void *in_buf, size_t in_bufsz, size_t out_bufsz)
{
	int ret = 0;
	struct iovec iov_r[3], iov_w[2], iov_ret[2];
	struct v4l2_ext_controls *ext_controls;
	struct v4l2_ext_control *ext_control;
	struct cuse_obj *pcuse_obj = (struct cuse_obj *)fuse_req_userdata(req);
	struct pq_obj_s *pobj = (struct pq_obj_s *)(pcuse_obj->pri);
	struct v4l2_ext_vpq_cmn_data *pcmn_data;

	iov_r[0].iov_base = arg;
	iov_r[0].iov_len = sizeof(struct v4l2_ext_controls);

	if (!in_bufsz) {
		fuse_reply_ioctl_retry(req, iov_r, 1, NULL, 0);
		printf("%s, out1\n", __func__);
		return;
	}

	ext_controls = (struct v4l2_ext_controls *)in_buf;
	in_buf += sizeof(struct v4l2_ext_controls);
	in_bufsz -= sizeof(struct v4l2_ext_controls);

	iov_r[1].iov_base = ext_controls->controls;
	iov_r[1].iov_len = sizeof(struct v4l2_ext_control);

	if (!in_bufsz) {
		fuse_reply_ioctl_retry(req, iov_r, 2, NULL, 0);
		printf("%s, out2\n", __func__);
		return;
	}

	ext_control = (struct v4l2_ext_control *)in_buf;
	in_buf += sizeof(struct v4l2_ext_control);
	in_bufsz -= sizeof(struct v4l2_ext_control);

	iov_r[2].iov_base = ext_control->ptr;
	iov_r[2].iov_len = sizeof(struct v4l2_ext_vpq_cmn_data);

	if (!in_bufsz) {
		fuse_reply_ioctl_retry(req, iov_r, 3, NULL, 0);
		printf("%s, out3\n", __func__);
		return;
	}

	pcmn_data = (struct v4l2_ext_vpq_cmn_data *)in_buf;
	in_buf += sizeof(struct v4l2_ext_vpq_cmn_data);
	in_bufsz -= sizeof(struct v4l2_ext_vpq_cmn_data);

	iov_w[0].iov_base = pcmn_data->p_data;
	iov_w[0].iov_len = pcmn_data->length;

	if (!out_bufsz) {
		fuse_reply_ioctl_retry(req, iov_r, 3, iov_w, 1);
		printf("%s, out4\n", __func__);
		return;
	}

	printf("%s, in, cmd = %#x\n", __func__, ext_control->id);
	printf("out_bufsz: %x, data_size: %d\n",
		out_bufsz, pcmn_data->length);

	switch (ext_control->id) {
	case V4L2_CID_EXT_VPQ_PICTURE_CTRL:
		ret = pq_drv_g_ext_pic_ctrl(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_SHARPNESS:
		ret = pq_drv_g_ext_sharpness(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_SUPER_RESOLUTION:
		ret = pq_drv_g_ext_sr(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_HISTO_DATA:
		ret = pq_drv_g_ext_histo_data(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST:
		ret = pq_drv_g_ext_dynamic_contrast(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_LUT:
		ret = pq_drv_g_ext_dynamic_contrast_lut(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_BYPASS_LUT:
		ret = pq_drv_g_ext_dynamic_contrast_bypasslut(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_CM_DB_DATA:
		ret = pq_drv_g_ext_cm_db_data(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_BYPASS_BLOCK:
		ret = pq_drv_g_ext_bypass_block(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_BLACK_LEVEL:
		ret = pq_drv_g_ext_black_level(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_GAMMA_DATA:
		ret = pq_drv_g_ext_gamma_data(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_COLORTEMP_DATA:
		ret = pq_drv_g_ext_colortemp_data(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_GAMUT_3DLUT:
		ret = pq_drv_g_ext_gamut_3dlut(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_OD_TABLE:
		ret = pq_drv_g_ext_od_table(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_OD_EXTENSION:
		ret = pq_drv_g_ext_od_extension(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_LOCALCONTRAST_TABLE:
		ret = pq_drv_g_ext_localcontrast_table(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_LOCALCONTRAST_DATA:
		ret = pq_drv_g_ext_localcontrast_data(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_PSP:
		ret = pq_drv_g_ext_psp(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_GAMUT_MATRIX_PRE:
		ret = pq_drv_g_ext_gamut_matrix_pre(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_GAMUT_MATRIX_POST:
		ret = pq_drv_g_ext_gamut_matrix_post(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_PQ_MODE_INFO:
		ret = pq_drv_g_ext_pq_mode_info(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_LUMINANCE_BOOST:
		ret = pq_drv_g_ext_luminance_boost(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_OBC_DATA:
		ret = pq_drv_g_ext_obc_data(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_OBC_LUT:
		ret = pq_drv_g_ext_obc_lut(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_OBC_CTRL:
		ret = pq_drv_g_ext_obc_ctrl(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_EXTRA_PATTERN:
		ret = pq_drv_g_ext_extra_pattern(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_VIDEO_PATTERN_INFO:
		ret = pq_drv_g_ext_video_pattern_info(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_PHDR_APL_GAIN_LUT:
		ret = pq_drv_g_ext_phdr_apl_gain_lut(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_HDR_PIC_INFO:
		ret = pq_drv_g_ext_hdr_pic_info(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_HDR_TONEMAP:
		ret = pq_drv_g_ext_hdr_tonemap(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_HDR_HLG_Y_GAIN_TBL:
		ret = pq_drv_g_ext_hdr_hlg_ygain_tbl(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_NOISE_REDUCTION:
		ret = pq_drv_g_ext_nr(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_MPEG_NOISE_REDUCTION:
		ret = pq_drv_g_ext_mpeg_nr(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_VPQ_DECONTOUR:
		ret = pq_drv_g_ext_decontour(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_MEMC_MOTION_COMP:
		ret = pq_drv_g_ext_memc_motion_comp(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_LED_DEMOMODE:
		ret = pq_drv_g_ext_led_demomode(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_LED_DB_DATA:
		ret = pq_drv_g_ext_led_db_data(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_LED_CONTROL_SPI:
		ret = pq_drv_g_ext_led_control_spi(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_LED_APL_DATA:
		ret = pq_drv_g_ext_led_apl_data(pobj, pcmn_data);
		break;
	case V4L2_CID_EXT_LED_BPL_DATA:
		break;
	default:
		fuse_reply_err(req, EINVAL);
		break;
	}

	iov_ret[0].iov_base = pcmn_data->p_data;
	iov_ret[0].iov_len = pcmn_data->length;

	fuse_reply_ioctl_iov(req, ret, iov_ret, 1);
}

static void pq_cuse_ioctl(fuse_req_t req, int cmd, void *arg,
	struct fuse_file_info *fi, unsigned int flags,
	const void *in_buf, size_t in_bufsz, size_t out_bufsz)
{
	printf("%s, in, cmd = %#x\n", __func__, cmd);

	pthread_mutex_lock(&pq_mutex);

	switch (cmd) {
	case VIDIOC_S_CTRL:
		if (!in_bufsz || !out_bufsz) {
			struct iovec iov_r = {arg, sizeof(struct v4l2_control)};
			struct iovec iov_w = {arg, sizeof(struct v4l2_control)};
			fuse_reply_ioctl_retry(req, &iov_r, 1, &iov_w, 1);
		} else {
			pq_cuse_s_ctrl(req, in_buf);
		}
		break;
	case VIDIOC_G_CTRL:
		if (!in_bufsz || !out_bufsz) {
			struct iovec iov_r = {arg, sizeof(struct v4l2_control)};
			struct iovec iov_w = {arg, sizeof(struct v4l2_control)};
			fuse_reply_ioctl_retry(req, &iov_r, 1, &iov_w, 1);
		} else {
			pq_cuse_g_ctrl(req, in_buf);
		}
		break;
	case VIDIOC_S_EXT_CTRLS:
		pq_cuse_s_ext_ctrls(req, arg, in_buf, in_bufsz);
		break;
	case VIDIOC_G_EXT_CTRLS:
		pq_cuse_g_ext_ctrls(req, arg, in_buf, in_bufsz, out_bufsz);
		break;
	default:
		fuse_reply_err(req, EINVAL);
		break;
	}

	pthread_mutex_unlock(&pq_mutex);
}

static struct cuse_lowlevel_ops pq_cuse_ops = {
	.open = pq_cuse_open,
	.read = pq_cuse_read,
	.write = pq_cuse_write,
	.release = pq_cuse_release,
	.ioctl = pq_cuse_ioctl,
};

struct pq_obj_s pq_obj = {
	.obj_name = "pqfuse",
	.video_fd = -1,
};

struct cuse_obj pq_cuse = {
	.dev_name = "DEVNAME=video50",
	.p_cuse_ops = &pq_cuse_ops,
	.pri = &pq_obj,
};

#ifdef __cplusplus
}
#endif
