/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef PQ_FUSE_H
#define PQ_FUSE_H

#include <binder/Binder.h>
#include <binder/Parcel.h>
#include <map>

using namespace android;

#ifdef __cplusplus
extern "C" {
#endif

#define V4L2_EXT_HDR_HLG_YGAIN_TBL_SIZE 66

struct pq_obj_s {
	const char *obj_name;
	int video_fd;
	int game_mode;
	int dc_color_gain;
	int real_cinema;
	int stereo_face_ctrl;
	int led_mode;
	int ai_sqm_mode;
	int ai_sr_mode;
	int ai_depth_mode;
	int ai_scene_mode;
	int ai_genre_mode;
	int hdr_inv_gamma;
	unsigned short hdr_hlg_ygain[V4L2_EXT_HDR_HLG_YGAIN_TBL_SIZE];
	struct v4l2_ext_vpq_picture_ctrl_data pic_ctrl_data;
	struct v4l2_ext_vpq_sharpness_data sharpness_data;
	struct v4l2_ext_vpq_super_resolution_data sr_data;
	struct v4l2_ext_vpq_dc2p_histodata_info histodata_info;
	struct v4l2_ext_dynamnic_contrast_ctrl dc_ctrl;
	struct v4l2_ext_dynamnic_contrast_lut dc_lut;
	struct v4l2_ext_cm_info cm_info;
	struct v4l2_ext_vpq_block_bypass block_bypass;
	struct v4l2_ext_vpq_black_level_info blk_lvl_info;
	struct v4l2_ext_gamma_lut gamma_lut;
	struct v4l2_ext_vpq_color_temp color_temp;
	struct v4l2_ext_vpq_gamut_add_info gamut_add_info;
	struct v4l2_ext_vpq_gamut_lut gamut_lut;
	struct v4l2_ext_gamut_post gamut_post;
	struct v4l2_ext_pq_mode_info pq_mode_info;
	struct v4l2_ext_vpq_noise_reduction_data nr_data;
	struct v4l2_ext_vpq_mpeg_noise_reduction_data mpeg_nr_data;
	struct v4l2_ext_vpq_decontour_data decontour_data;
	struct v4l2_ext_vpq_od od;
	struct v4l2_ext_vpq_od_extension od_extension;
	struct v4l2_vpq_ext_pattern_info_v2 ptn_info_v2;
	struct v4l2_ext_hdr_3dlut hdr_3dlut;
	struct v4l2_ext_hdr_tonemap hdr_tonemap;
	struct v4l2_ext_hdr_color_correction hdr_cc;
	struct v4l2_ext_memc_motion_comp_info memc_motion_comp_info;
	struct v4l2_ext_led_panel_info led_panel_info;
	struct v4l2_ext_led_ldim_demo_info led_demo_info;
	struct v4l2_ext_led_apl_info led_apl_info;
};

int pq_drv_open(struct pq_obj_s *pobj, int flags);
int pq_drv_close(struct pq_obj_s *pobj);

int pq_drv_s_ext_init(struct pq_obj_s *pobj, int init);
int pq_drv_s_ext_low_delay_mode(struct pq_obj_s *pobj, int mode);
int pq_drv_g_ext_low_delay_mode(struct pq_obj_s *pobj, int *pmode);
int pq_drv_s_ext_dc_color_gain(struct pq_obj_s *pobj, int gain);
int pq_drv_g_ext_dc_color_gain(struct pq_obj_s *pobj, int *pgain);
int pq_drv_s_ext_real_cinema(struct pq_obj_s *pobj, int mode);
int pq_drv_g_ext_real_cinema(struct pq_obj_s *pobj, int *pmode);
int pq_drv_s_ext_stereo_face_ctrl(struct pq_obj_s *pobj, int mode);
int pq_drv_g_ext_stereo_face_ctrl(struct pq_obj_s *pobj, int *pmode);
int pq_drv_g_ext_video_latency(struct pq_obj_s *pobj, int *platency);

int pq_drv_s_ext_pic_ctrl(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_pic_ctrl(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_sharpness(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_sharpness(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_sr(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_sr(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_histo_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_dynamic_contrast(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_dynamic_contrast(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_dynamic_contrast_lut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_dynamic_contrast_lut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_dynamic_contrast_bypasslut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_cm_db_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_cm_db_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_bypass_block(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_bypass_block(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_black_level(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_black_level(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_gamma_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_gamma_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_colortemp_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_colortemp_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_gamut_3dlut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_gamut_3dlut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_od_table(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_od_table(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_od_extension(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_od_extension(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_localcontrast_table(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_localcontrast_table(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_localcontrast_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_localcontrast_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_psp(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_psp(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_gamut_matrix_pre(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_gamut_matrix_pre(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_gamut_matrix_post(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_gamut_matrix_post(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_pq_mode_info(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_pq_mode_info(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_luminance_boost(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_luminance_boost(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_obc_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_obc_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_obc_lut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_obc_lut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_obc_ctrl(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_obc_ctrl(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_extra_pattern(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_extra_pattern(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_db_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_video_pattern_info(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_phdr_apl_gain_lut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_phdr_apl_gain_lut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);

/*HDR related APIs*/
int pq_drv_s_ext_hdr_inv_gamma(struct pq_obj_s *pobj, int mode);
int pq_drv_g_ext_hdr_pic_info(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_hdr_3dlut(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_hdr_eotf(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_hdr_oetf(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_hdr_tonemap(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_hdr_tonemap(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_hdr_color_correction(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_hdr_hlg_ygain_tbl(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_hdr_hlg_ygain_tbl(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);

/*AI PQ related APIs*/
int pq_drv_s_ext_aipq_sqm_mode(struct pq_obj_s *pobj, int mode);
int pq_drv_g_ext_aipq_sqm_mode(struct pq_obj_s *pobj, int *pmode);
int pq_drv_s_ext_aipq_sr_mode(struct pq_obj_s *pobj, int mode);
int pq_drv_g_ext_aipq_sr_mode(struct pq_obj_s *pobj, int *pmode);
int pq_drv_s_ext_aipq_depth_mode(struct pq_obj_s *pobj, int mode);
int pq_drv_g_ext_aipq_depth_mode(struct pq_obj_s *pobj, int *pmode);
int pq_drv_s_ext_aipq_scene_mode(struct pq_obj_s *pobj, int mode);
int pq_drv_g_ext_aipq_scene_mode(struct pq_obj_s *pobj, int *pmode);
int pq_drv_g_ext_aipq_scene_info(struct pq_obj_s *pobj, int *pinfo);
int pq_drv_s_ext_aipq_genre_mode(struct pq_obj_s *pobj, int mode);
int pq_drv_g_ext_aipq_genre_mode(struct pq_obj_s *pobj, int *pmode);
int pq_drv_g_ext_aipq_genre_info(struct pq_obj_s *pobj, int *pinfo);

/*Noise Reduction related APIs*/
int pq_drv_s_ext_nr(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_nr(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_mpeg_nr(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_mpeg_nr(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_decontour(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_decontour(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);

/*MEMC related APIs*/
int pq_drv_s_ext_memc_init(struct pq_obj_s *pobj, int init);
int pq_drv_s_ext_memc_low_delay_mode(struct pq_obj_s *pobj, int mode);
int pq_drv_g_ext_memc_low_delay_mode(struct pq_obj_s *pobj, int *pmode);
int pq_drv_s_ext_memc_motion_pro(struct pq_obj_s *pobj, int motion_pro);
int pq_drv_g_ext_memc_motion_pro(struct pq_obj_s *pobj, int *pmotion_pro);
int pq_drv_s_ext_memc_motion_comp(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_memc_motion_comp(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);

/*LED related APIs*/
int pq_drv_s_ext_led_init(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_led_demomode(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_led_demomode(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_s_ext_led_en(struct pq_obj_s *pobj, int mode);
int pq_drv_g_ext_led_en(struct pq_obj_s *pobj, int *pmode);
int pq_drv_s_ext_led_db_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_led_db_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);
int pq_drv_g_ext_led_apl_data(struct pq_obj_s *pobj,
	struct v4l2_ext_vpq_cmn_data *pdata);

#ifdef __cplusplus
}
#endif

#endif
