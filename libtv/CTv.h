/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#include "common.h"
#include "CTvin.h"
#include "CTvDevicesPollDetect.h"
#include "CTvEvent.h"
#include "CHDMIRxManager.h"
#ifdef HAVE_AUDIO
#include "CTvAudio.h"
#endif


#define CONFIG_FILE_PATH_DEF               "/vendor/etc/tvconfig/tvconfig.conf"

#define VIDEO_DISABLE_VIDEO                "/sys/class/video/disable_video"
#define DOLBY_VISION_TV_KO_PATH            "/vendor/lib/modules/dovi_tv.ko"
#define DOLBY_VISION_STB_KO_PATH           "/vendor/lib/modules/dovi.ko"
#define DOLBY_VISION_ENABLE_PATH           "/sys/module/amdolby_vision/parameters/dolby_vision_enable"

typedef enum video_layer_status_e {
    VIDEO_LAYER_STATUS_ENABLE,
    VIDEO_LAYER_STATUS_DISABLE,
    VIDEO_LAYER_STATUS_ENABLE_AND_CLEAN,
    VIDEO_LAYER_STATUS_MAX,
} videolayer_status_t;

class CTv : public CTvDevicesPollDetect::ISourceConnectObserver {
public:
    class TvIObserver {
    public:
        TvIObserver() {};
        virtual ~TvIObserver() {};
        virtual void onTvEvent (CTvEvent &event) = 0;
    };
    CTv();
    ~CTv();
    int setTvObserver (TvIObserver *ob);
    int StartTv(tv_source_input_t source);
    int StopTv(tv_source_input_t source);
    int SwitchSource(tv_source_input_t dest_source);
    int SetCurrenSourceInfo(tvin_info_t sig_info);
    tvin_info_t GetCurrentSourceInfo(void);
    int SetEDIDData(tv_source_input_t source, char *data);
    int GetEDIDData(tv_source_input_t source, char *data);
    int LoadEdidData(int isNeedBlackScreen, int isDolbyVisionEnable);
    int SetEdidVersion(tv_source_input_t source, tv_hdmi_edid_version_t edidVer);
    int GetEdidVersion(tv_source_input_t source);
    int SetVdinWorkMode(vdin_work_mode_t vdinWorkMode);

    virtual void onSourceConnect(int source, int connect_status);
    virtual void onVdinSignalChange();

private:
    void onSigStatusChange(void);
    void onSigToStable();
    void onSigToUnstable();
    void onSigToUnSupport();
    void onSigToNoSig();
    int sendTvEvent(CTvEvent &event);
    int setVideoLayerStatus(videolayer_status_t status);
#ifdef HAVE_AUDIO
    int mapSourcetoAudiotupe(tv_source_input_t dest_source);
#endif

    CTvin *mpTvin;
    CHDMIRxManager *mpHDMIRxManager;
    tvin_info_t mCurrentSignalInfo;
    tv_source_input_t mCurrentSource;
    CTvDevicesPollDetect mTvDevicesPollDetect;
    TvIObserver *mpObserver;
    vdin_work_mode_t mVdinWorkMode = VDIN_WORK_MODE_VFM;
};
