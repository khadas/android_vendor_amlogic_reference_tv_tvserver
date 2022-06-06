/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_MOUDLE_TAG "TVIN-FUSE"
#define LOG_CLASS_TAG "PQ_DRV"

#include "../common.h"
#include "pq_fuse.h"

#ifdef __cplusplus
extern "C" {
#endif

int pq_drv_open(struct pq_obj_s *pobj, int flags)
{
	printf("%s, %d\n", __func__, __LINE__);

	/*if (pobj->video_fd == -1) {
		pobj->video_fd = open(V4L2_EXT_DEV_PATH_PQ, flags, 0);
		if (pobj->video_fd < 0) {
			printf("Unable to open %s, err:%s\n", V4L2_EXT_DEV_PATH_PQ,
				strerror(errno));
			return errno;
		}
	}*/

	return 0;
}

int pq_drv_close(struct pq_obj_s *pobj)
{
	printf("%s, %d\n", __func__, __LINE__);

	if (pobj->video_fd > 0) {
		close(pobj->video_fd);
		pobj->video_fd = -1;
	}

	return 0;
}

/* V4L2_CID_EXT_VPQ_INIT */
int pq_drv_s_ext_init(struct pq_obj_s *pobj, int init)
{
	printf("%s, %d, %d\n", __func__, __LINE__, init);
	return 0;
}

/* V4L2_CID_EXT_VPQ_LOW_DELAY_MODE */
int pq_drv_s_ext_low_delay_mode(struct pq_obj_s *pobj, int mode)
{
	printf("%s, %d, %d\n", __func__, __LINE__, mode);

	pobj->game_mode = mode;

	return 0;
}

int pq_drv_g_ext_low_delay_mode(struct pq_obj_s *pobj, int *pmode)
{
	printf("%s, %d,\n", __func__, __LINE__);

	*pmode = pobj->game_mode;

	return 0;
}

/* V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_COLOR_GAIN */
int pq_drv_s_ext_dc_color_gain(struct pq_obj_s *pobj, int gain)
{
	printf("%s, %d, %d\n", __func__, __LINE__, gain);

	pobj->dc_color_gain = gain;

	return 0;
}

int pq_drv_g_ext_dc_color_gain(struct pq_obj_s *pobj, int *pgain)
{
	printf("%s, %d,\n", __func__, __LINE__);

	*pgain = pobj->dc_color_gain;

	return 0;
}

/* V4L2_CID_EXT_VPQ_REAL_CINEMA */
int pq_drv_s_ext_real_cinema(struct pq_obj_s *pobj, int mode)
{
	printf("%s, %d, %d\n", __func__, __LINE__, mode);

	pobj->real_cinema = mode;

	return 0;
}

int pq_drv_g_ext_real_cinema(struct pq_obj_s *pobj, int *pmode)
{
	printf("%s, %d,\n", __func__, __LINE__);

	*pmode = pobj->real_cinema;

	return 0;
}

/* V4L2_CID_EXT_VPQ_STEREO_FACE_CTRL */
int pq_drv_s_ext_stereo_face_ctrl(struct pq_obj_s *pobj, int mode)
{
	printf("%s, %d, %d\n", __func__, __LINE__, mode);

	pobj->stereo_face_ctrl = mode;

	return 0;
}

int pq_drv_g_ext_stereo_face_ctrl(struct pq_obj_s *pobj, int *pmode)
{
	printf("%s, %d,\n", __func__, __LINE__);

	*pmode = pobj->stereo_face_ctrl;

	return 0;
}

/* V4L2_CID_EXT_VPQ_VIDEO_LATENCY */
int pq_drv_g_ext_video_latency(struct pq_obj_s *pobj, int *platency)
{
	printf("%s, %d,\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_VPQ_PICTURE_CTRL */
int pq_drv_s_ext_pic_ctrl(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->pic_ctrl_data, pdata->p_data,
		sizeof(struct v4l2_ext_vpq_picture_ctrl_data));

	return 0;
}

int pq_drv_g_ext_pic_ctrl(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->pic_ctrl_data;

	return 0;
}

/* V4L2_CID_EXT_VPQ_SHARPNESS */
int pq_drv_s_ext_sharpness(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->sharpness_data, pdata->p_data,
		sizeof(struct v4l2_ext_vpq_sharpness_data));

	return 0;
}

int pq_drv_g_ext_sharpness(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->sharpness_data;

	return 0;
}

/* V4L2_CID_EXT_VPQ_SUPER_RESOLUTION */
int pq_drv_s_ext_sr(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->sr_data, pdata->p_data,
		sizeof(struct v4l2_ext_vpq_super_resolution_data));

	return 0;
}

int pq_drv_g_ext_sr(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->sr_data;

	return 0;
}

/* V4L2_CID_EXT_VPQ_HISTO_DATA */
int pq_drv_g_ext_histo_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->histodata_info;

	return 0;
}

/* V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST */
int pq_drv_s_ext_dynamic_contrast(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->dc_ctrl, pdata->p_data,
		sizeof(struct v4l2_ext_dynamnic_contrast_ctrl));

	return 0;
}

int pq_drv_g_ext_dynamic_contrast(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->dc_ctrl;

	return 0;
}

/* V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_LUT */
int pq_drv_s_ext_dynamic_contrast_lut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->dc_lut, pdata->p_data,
		sizeof(struct v4l2_ext_dynamnic_contrast_lut));

	return 0;
}

int pq_drv_g_ext_dynamic_contrast_lut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->dc_lut;

	return 0;
}

/* V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_BYPASS_LUT */
int pq_drv_g_ext_dynamic_contrast_bypasslut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	int bypass_lut[MAX_DYNAMIC_CONTRAST_LUT_SIZE] = {0};

	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&bypass_lut[0];

	return 0;
}

/* V4L2_CID_EXT_VPQ_CM_DB_DATA */
int pq_drv_s_ext_cm_db_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->cm_info, pdata->p_data,
		sizeof(struct v4l2_ext_cm_info));

	return 0;
}

int pq_drv_g_ext_cm_db_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->cm_info;

	return 0;
}

/* V4L2_CID_EXT_VPQ_BYPASS_BLOCK */
int pq_drv_s_ext_bypass_block(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->block_bypass, pdata->p_data,
		sizeof(struct v4l2_ext_vpq_block_bypass));

	return 0;
}

int pq_drv_g_ext_bypass_block(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->block_bypass;

	return 0;
}

/* V4L2_CID_EXT_VPQ_BLACK_LEVEL */
int pq_drv_s_ext_black_level(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->blk_lvl_info, pdata->p_data,
		sizeof(struct v4l2_ext_vpq_black_level_info));

	return 0;
}

int pq_drv_g_ext_black_level(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->blk_lvl_info;

	return 0;
}

/* V4L2_CID_EXT_VPQ_GAMMA_DATA */
int pq_drv_s_ext_gamma_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->gamma_lut, pdata->p_data,
		sizeof(struct v4l2_ext_gamma_lut));

	return 0;
}

int pq_drv_g_ext_gamma_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->gamma_lut;

	return 0;
}

/* V4L2_CID_EXT_VPQ_COLORTEMP_DATA */
int pq_drv_s_ext_colortemp_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->color_temp, pdata->p_data,
		sizeof(struct v4l2_ext_vpq_color_temp));

	return 0;
}

int pq_drv_g_ext_colortemp_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->color_temp;

	return 0;
}

/* V4L2_CID_EXT_VPQ_GAMUT_3DLUT */
int pq_drv_s_ext_gamut_3dlut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->gamut_lut, pdata->p_data,
		sizeof(struct v4l2_ext_vpq_gamut_lut));

	return 0;
}

int pq_drv_g_ext_gamut_3dlut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->gamut_lut;

	return 0;
}

/* V4L2_CID_EXT_VPQ_OD_TABLE */
int pq_drv_s_ext_od_table(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->od, pdata->p_data,
		sizeof(struct v4l2_ext_vpq_od));

	return 0;
}

int pq_drv_g_ext_od_table(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->od;

	return 0;
}

/* V4L2_CID_EXT_VPQ_OD_EXTENSION */
int pq_drv_s_ext_od_extension(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->od_extension, pdata->p_data,
		sizeof(struct v4l2_ext_vpq_od_extension));

	return 0;
}

int pq_drv_g_ext_od_extension(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->od_extension;

	return 0;
}

/* V4L2_CID_EXT_VPQ_LOCALCONTRAST_TABLE */
int pq_drv_s_ext_localcontrast_table(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

int pq_drv_g_ext_localcontrast_table(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_VPQ_LOCALCONTRAST_DATA */
int pq_drv_s_ext_localcontrast_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

int pq_drv_g_ext_localcontrast_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_VPQ_PSP */
int pq_drv_s_ext_psp(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

int pq_drv_g_ext_psp(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_VPQ_GAMUT_MATRIX_PRE */
int pq_drv_s_ext_gamut_matrix_pre(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

int pq_drv_g_ext_gamut_matrix_pre(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_VPQ_GAMUT_MATRIX_POST */
int pq_drv_s_ext_gamut_matrix_post(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->gamut_post, pdata->p_data,
		sizeof(struct v4l2_ext_gamut_post));

	return 0;
}

int pq_drv_g_ext_gamut_matrix_post(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->gamut_post;

	return 0;
}

/* V4L2_CID_EXT_VPQ_PQ_MODE_INFO */
int pq_drv_s_ext_pq_mode_info(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->pq_mode_info, pdata->p_data,
		sizeof(struct v4l2_ext_pq_mode_info));

	return 0;
}

int pq_drv_g_ext_pq_mode_info(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->pq_mode_info;

	return 0;
}

/* V4L2_CID_EXT_VPQ_LUMINANCE_BOOST */
int pq_drv_s_ext_luminance_boost(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

int pq_drv_g_ext_luminance_boost(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_VPQ_OBC_DATA */
int pq_drv_s_ext_obc_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

int pq_drv_g_ext_obc_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_VPQ_OBC_LUT */
int pq_drv_s_ext_obc_lut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

int pq_drv_g_ext_obc_lut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_VPQ_OBC_CTRL */
int pq_drv_s_ext_obc_ctrl(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

int pq_drv_g_ext_obc_ctrl(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_VPQ_EXTRA_PATTERN */
int pq_drv_s_ext_extra_pattern(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->ptn_info_v2, pdata->p_data,
		sizeof(struct v4l2_vpq_ext_pattern_info_v2));

	return 0;
}

int pq_drv_g_ext_extra_pattern(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->ptn_info_v2;

	return 0;
}

/* V4L2_CID_EXT_VPQ_DB_DATA */
int pq_drv_s_ext_db_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_VPQ_VIDEO_PATTERN_INFO */
int pq_drv_g_ext_video_pattern_info(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_VPQ_PHDR_APL_GAIN_LUT */
int pq_drv_s_ext_phdr_apl_gain_lut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

int pq_drv_g_ext_phdr_apl_gain_lut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_HDR_INV_GAMMA */
int pq_drv_s_ext_hdr_inv_gamma(struct pq_obj_s *pobj, int mode)
{
	printf("%s, %d, %d\n", __func__, __LINE__, mode);

	pobj->hdr_inv_gamma = mode;

	return 0;
}

/* V4L2_CID_EXT_HDR_PIC_INFO */
int pq_drv_g_ext_hdr_pic_info(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	unsigned int hdr_pic_info[V4L2_EXT_HDR_PICINFO_SIZE] = {0};

	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&hdr_pic_info[0];

	return 0;
}

/* V4L2_CID_EXT_HDR_3DLUT */
int pq_drv_s_ext_hdr_3dlut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->hdr_3dlut, pdata->p_data,
		sizeof(struct v4l2_ext_hdr_3dlut));

	return 0;
}

/* V4L2_CID_EXT_HDR_EOTF */
int pq_drv_s_ext_hdr_eotf(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_HDR_OETF */
int pq_drv_s_ext_hdr_oetf(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_HDR_TONEMAP */
int pq_drv_s_ext_hdr_tonemap(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->hdr_tonemap, pdata->p_data,
		sizeof(struct v4l2_ext_hdr_tonemap));

	return 0;
}

int pq_drv_g_ext_hdr_tonemap(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->hdr_tonemap;

	return 0;
}

/* V4L2_CID_EXT_HDR_COLOR_CORRECTION */
int pq_drv_s_ext_hdr_color_correction(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->hdr_cc, pdata->p_data,
		sizeof(struct v4l2_ext_hdr_color_correction));

	return 0;
}

/* V4L2_CID_EXT_HDR_HLG_Y_GAIN_TBL */
int pq_drv_s_ext_hdr_hlg_ygain_tbl(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->hdr_hlg_ygain[0], pdata->p_data,
		sizeof(unsigned short) * V4L2_EXT_HDR_HLG_YGAIN_TBL_SIZE);

	return 0;
}

int pq_drv_g_ext_hdr_hlg_ygain_tbl(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->hdr_hlg_ygain[0];

	return 0;
}

/* V4L2_CID_EXT_AIPQ_SQM_MODE */
int pq_drv_s_ext_aipq_sqm_mode(struct pq_obj_s *pobj, int mode)
{
	printf("%s, %d, %d\n", __func__, __LINE__, mode);

	pobj->ai_sqm_mode = mode;

	return 0;
}

int pq_drv_g_ext_aipq_sqm_mode(struct pq_obj_s *pobj, int *pmode)
{
	printf("%s, %d,\n", __func__, __LINE__);

	*pmode = pobj->ai_sqm_mode;

	return 0;
}

/* V4L2_CID_EXT_AIPQ_SR_MODE */
int pq_drv_s_ext_aipq_sr_mode(struct pq_obj_s *pobj, int mode)
{
	printf("%s, %d, %d\n", __func__, __LINE__, mode);

	pobj->ai_sr_mode = mode;

	return 0;
}

int pq_drv_g_ext_aipq_sr_mode(struct pq_obj_s *pobj, int *pmode)
{
	printf("%s, %d,\n", __func__, __LINE__);

	*pmode = pobj->ai_sr_mode;

	return 0;
}

/* V4L2_CID_EXT_AIPQ_DEPTH_MODE */
int pq_drv_s_ext_aipq_depth_mode(struct pq_obj_s *pobj, int mode)
{
	printf("%s, %d, %d\n", __func__, __LINE__, mode);

	pobj->ai_depth_mode = mode;

	return 0;
}

int pq_drv_g_ext_aipq_depth_mode(struct pq_obj_s *pobj, int *pmode)
{
	printf("%s, %d,\n", __func__, __LINE__);

	*pmode = pobj->ai_depth_mode;

	return 0;
}

/* V4L2_CID_EXT_AIPQ_SCENE_MODE */
int pq_drv_s_ext_aipq_scene_mode(struct pq_obj_s *pobj, int mode)
{
	printf("%s, %d, %d\n", __func__, __LINE__, mode);

	pobj->ai_scene_mode = mode;

	return 0;
}

int pq_drv_g_ext_aipq_scene_mode(struct pq_obj_s *pobj, int *pmode)
{
	printf("%s, %d,\n", __func__, __LINE__);

	*pmode = pobj->ai_scene_mode;

	return 0;
}

/* V4L2_CID_EXT_AIPQ_SCENE_INFO */
int pq_drv_g_ext_aipq_scene_info(struct pq_obj_s *pobj, int *pinfo)
{
	printf("%s, %d,\n", __func__, __LINE__);

	*pinfo = 0;

	return 0;
}

/* V4L2_CID_EXT_AIPQ_GENRE_MODE */
int pq_drv_s_ext_aipq_genre_mode(struct pq_obj_s *pobj, int mode)
{
	printf("%s, %d, %d\n", __func__, __LINE__, mode);

	pobj->ai_genre_mode = mode;

	return 0;
}

int pq_drv_g_ext_aipq_genre_mode(struct pq_obj_s *pobj, int *pmode)
{
	printf("%s, %d,\n", __func__, __LINE__);

	*pmode = pobj->ai_genre_mode;

	return 0;
}

/* V4L2_CID_EXT_AIPQ_GENRE_INFO */
int pq_drv_g_ext_aipq_genre_info(struct pq_obj_s *pobj, int *pinfo)
{
	printf("%s, %d,\n", __func__, __LINE__);

	*pinfo = 0;

	return 0;
}

/* V4L2_CID_EXT_VPQ_NOISE_REDUCTION */
int pq_drv_s_ext_nr(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->nr_data, pdata->p_data,
		sizeof(struct v4l2_ext_vpq_noise_reduction_data));

	return 0;
}

int pq_drv_g_ext_nr(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->nr_data;

	return 0;
}

/* V4L2_CID_EXT_VPQ_MPEG_NOISE_REDUCTION */
int pq_drv_s_ext_mpeg_nr(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->mpeg_nr_data, pdata->p_data,
		sizeof(struct v4l2_ext_vpq_mpeg_noise_reduction_data));

	return 0;
}

int pq_drv_g_ext_mpeg_nr(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->mpeg_nr_data;

	return 0;
}

/* V4L2_CID_EXT_VPQ_DECONTOUR */
int pq_drv_s_ext_decontour(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->decontour_data, pdata->p_data,
		sizeof(struct v4l2_ext_vpq_decontour_data));

	return 0;
}

int pq_drv_g_ext_decontour(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->decontour_data;

	return 0;
}

/* V4L2_CID_EXT_MEMC_INIT */
int pq_drv_s_ext_memc_init(struct pq_obj_s *pobj, int init)
{
	printf("%s, %d, %d\n", __func__, __LINE__, init);
	return 0;
}

/* V4L2_CID_EXT_MEMC_LOW_DELAY_MODE */
int pq_drv_s_ext_memc_low_delay_mode(struct pq_obj_s *pobj, int mode)
{
	printf("%s, %d, %d\n", __func__, __LINE__, mode);
	return 0;
}

int pq_drv_g_ext_memc_low_delay_mode(struct pq_obj_s *pobj, int *pmode)
{
	printf("%s, %d,\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_MEMC_MOTION_PRO */
int pq_drv_s_ext_memc_motion_pro(struct pq_obj_s *pobj, int motion_pro)
{
	printf("%s, %d, %d\n", __func__, __LINE__, motion_pro);
	return 0;
}

int pq_drv_g_ext_memc_motion_pro(struct pq_obj_s *pobj, int *pmotion_pro)
{
	printf("%s, %d,\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_MEMC_MOTION_COMP */
int pq_drv_s_ext_memc_motion_comp(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->memc_motion_comp_info, pdata->p_data,
		sizeof(struct v4l2_ext_memc_motion_comp_info));

	return 0;
}

int pq_drv_g_ext_memc_motion_comp(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->memc_motion_comp_info;

	return 0;
}

/* V4L2_CID_EXT_LED_INIT */
int pq_drv_s_ext_led_init(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->led_panel_info, pdata->p_data,
		sizeof(struct v4l2_ext_led_panel_info));

	return 0;
}

/* V4L2_CID_EXT_LED_DEMOMODE */
int pq_drv_s_ext_led_demomode(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	memcpy(&pobj->led_demo_info, pdata->p_data,
		sizeof(struct v4l2_ext_led_ldim_demo_info));

	return 0;
}

int pq_drv_g_ext_led_demomode(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->led_demo_info;

	return 0;
}

/* V4L2_CID_EXT_LED_EN */
int pq_drv_s_ext_led_en(struct pq_obj_s *pobj, int mode)
{
	printf("%s, %d, %d\n", __func__, __LINE__, mode);

	pobj->led_mode = mode;

	return 0;
}

int pq_drv_g_ext_led_en(struct pq_obj_s *pobj, int *pmode)
{
	printf("%s, %d\n", __func__, __LINE__);

	*pmode = pobj->led_mode;

	return 0;
}

/* V4L2_CID_EXT_LED_DB_DATA */
int pq_drv_s_ext_led_db_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

int pq_drv_g_ext_led_db_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);
	return 0;
}

/* V4L2_CID_EXT_LED_APL_DATA */
int pq_drv_g_ext_led_apl_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata)
{
	printf("%s, %d\n", __func__, __LINE__);

	pdata->p_data = (unsigned char*)&pobj->led_apl_info;

	return 0;
}

#ifdef __cplusplus
}
#endif
