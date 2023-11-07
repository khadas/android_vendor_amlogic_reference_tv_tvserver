/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#if !defined(_CDTV_H)
#define _CDTV_H
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <utils/threads.h>
#include "CTvProgram.h"
#include "CTvEpg.h"
#include "CTvRrt.h"
#include "CTvEas.h"
#include "CTvLog.h"
#include "CTvTime.h"
#include "CTvEvent.h"
#include "CTvEv.h"
#include "CTvBooking.h"
#include "../vpp/CVpp.h"
#include "../tvin/CTvin.h"
#include "../tvin/CHDMIRxManager.h"
#include <CMsgQueue.h>
#include <serial_operate.h>
#include "CTvRecord.h"
#include "CTvSubtitle.h"
#include "CAv.h"
#include "CTvDmx.h"
#include "CBootvideoStatusDetect.h"
#include "tvin/CDevicesPollStatusDetect.h"
#include "CTvGpio.h"
#include "CTvPanel.h"
#include "CVideotunnel.h"
#ifdef SUPPORT_ADTV
#include <am_epg.h>
#include <am_mem.h>
#endif
#include "CTvScanner.h"
#include "CFrontEnd.h"


#include <CTvFactory.h>

#include "CTvPlayer.h"

using namespace android;

static const char *TV_CONFIG_FILE_PATH_0 = "odm/etc/tvconfig/tvconfig.conf";
static const char *TV_CONFIG_FILE_PATH_1 = "/mnt/vendor/odm_ext/etc/tvconfig/tvconfig.conf";

#if ANDROID_PLATFORM_SDK_VERSION >= 28
static const char *TV_DB_PATH = "/mnt/vendor/param/dtv.db";
#else
static const char *TV_DB_PATH = "/param/dtv.db";
#endif

#define LCD_ENABLE_49     "/sys/class/lcd/power"
#define LCD_ENABLE_54     "/sys/class/aml_lcd/lcd0/power_on"

#define DEVICE_CLASS_TSYNC_AV_THRESHOLD_MIN "/sys/class/tsync/av_threshold_min"
#define IWATT_SHELL_PATH    "/system/bin/get_iwatt_regs.sh"

#define _A_M  'S'
#define AMSTREAM_IOC_GET_MVDECINFO _IOR((_A_M), 0xcb, int)

#define AV_THRESHOLD_MIN_MS "540000" //6S = 6*90000

#define VRR_FUNC_CTRL_PATH    "/sys/class/hdmirx/hdmirx0/vrr_func_ctrl"
#define DLG_FUNC_PATH         "/sys/class/display/cap"

#define PROP_DLG_STATUS "persist.vendor.sys.display.dlg"
#define DLG_MODE_OFF    "off"
#define DLG_MODE_ON     "on"

#define DTV_DTMB_MODE        "dtmb"
#define DTV_DVBC_MODE        "dvbc"
#define DTV_DVBS_MODE        "dvbs"
#define DTV_ATSC_MODE        "atsc"
#define DTV_DVBT_MODE        "dvbt"
#define DTV_ISDBT_MODE       "isdbt"

/*support tconless function*/
//#define SUPPORT_PANEL

#define NEW_FRAME_TIME_OUT_COUNT  200


struct vframe_comm_s {
	int vdec_id;
	char vdec_name[16];
	unsigned int vdec_type;
};

struct vframe_qos_s {
	unsigned int num;
	unsigned int type;
	unsigned int size;
	unsigned int pts;
	int max_qp;
	int avg_qp;
	int min_qp;
	int max_skip;
	int avg_skip;
	int min_skip;
	int max_mv;
	int min_mv;
	int avg_mv;
	int decode_buffer;
} /*vframe_qos */;

struct vframe_counter_s {
	struct vframe_qos_s qos;
	unsigned int  decode_time_cost;/*us*/
	unsigned int frame_width;
	unsigned int frame_height;
	unsigned int frame_rate;
	unsigned int bit_depth_luma;//original bit_rate;
	unsigned int frame_dur;
	unsigned int bit_depth_chroma;//original frame_data;
	unsigned int error_count;
	unsigned int status;
	unsigned int frame_count;
	unsigned int error_frame_count;
	unsigned int drop_frame_count;
	unsigned long long total_data;//this member must be 8 bytes alignment
	unsigned int double_write_mode;//original samp_cnt;
	unsigned int offset;
	unsigned int ratio_control;
	unsigned int vf_type;
	unsigned int signal_type;
	unsigned int pts;
	unsigned long long pts_us64;
};

/*This is a versioning structure, the key member is the struct_size.
 *In the 1st version it is not used,but will have its role in future.
 *https://bytes.com/topic/c/answers/811125-struct-versioning
 */
struct av_param_mvdec_t {
	int vdec_id;

	/*This member is used for versioning this structure.
	 *When passed from userspace, its value must be
	 *sizeof(struct av_param_mvdec_t)
	 */
	int struct_size;

	int slots;

	struct vframe_comm_s comm;
	struct vframe_counter_s minfo[8];
};


typedef enum tv_window_mode_e {
    NORMAL_WINDOW,
    PREVIEW_WINDOW,
} tv_window_mode_t;

typedef enum tv_dtv_scan_running_status_e {
    DTV_SCAN_RUNNING_NORMAL,
    DTV_SCAN_RUNNING_ANALYZE_CHANNEL,
} tv_dtv_scan_running_status_t;

typedef struct tv_config_s {
    bool kernelpet_disable;
    unsigned int kernelpet_timeout;
    bool userpet;
    unsigned int userpet_timeout;
    unsigned int userpet_reset;
    bool memory512m;
} tv_config_t;

typedef enum TvRunStatus_s {
    TV_INIT_ED = -1,
    TV_OPEN_ED = 0,
    TV_START_ED ,
    TV_RESUME_ED,
    TV_PAUSE_ED,
    TV_STOP_ED,
    TV_CLOSE_ED,
} TvRunStatus_t;

enum CC_AUDIO_MUTE_STATUS {
    CC_MUTE_ON,
    CC_MUTE_OFF,
};

enum CC_AUDIO_MUTE_PARAM {
    CC_AUDIO_UNMUTE,
    CC_AUDIO_MUTE,
};

enum {
    VIDEO_LAYER_COLOR_BLACK,
    VIDEO_LAYER_COLOR_BLUE,
    VIDEO_LAYER_COLOR_MAX,
};

enum {
    VIDEO_LAYER_COLOR_SHOW_DISABLE,
    VIDEO_LAYER_COLOR_SHOW_ONCE,
    VIDEO_LAYER_COLOR_SHOW_ALWAYES,
};

typedef enum {
    BLOCK_STATE_NONE,
    BLOCK_STATE_BLOCKED,
    BLOCK_STATE_UNBLOCKED,
} tv_block_state_e;

class CTvPlayer;
class CDTVTvPlayer;
class CATVTvPlayer;

class CTv : public CDevicesPollStatusDetect::ISourceConnectObserver,
            public CTvSubtitle::IObserver,
            public CBootvideoStatusDetect::IBootvideoStatusObserver,
            public CTv2d4GHeadSetDetect::IHeadSetObserver,
            public CTvRecord::IObserver,
            public SysClientcallback::IScreenColorChangeObserver {

public:
    static const int TV_ACTION_NULL = 0x0000;
    static const int TV_ACTION_IN_VDIN = 0x0001;
    static const int TV_ACTION_STOPING = 0x0002;
    static const int TV_ACTION_SCANNING = 0x0004;
    static const int TV_ACTION_PLAYING = 0x0008;
    static const int TV_ACTION_RECORDING = 0x0010;
    static const int TV_ACTION_SOURCE_SWITCHING = 0x0020;

    static const int OPEN_DEV_FOR_SCAN_ATV = 1;
    static const int OPEN_DEV_FOR_SCAN_DTV = 2;
    static const int CLOSE_DEV_FOR_SCAN = 3;
    tvin_info_t m_cur_sig_info;
    tvin_info_t m_last_sig_info;

    static const int RECORDING_CMD_STOP = 0;
    static const int RECORDING_CMD_PREPARE = 1;
    static const int RECORDING_CMD_START = 2;

    static const int PLAY_CMD_STOP = 0;
    static const int PLAY_CMD_START = 1;
    static const int PLAY_CMD_PAUSE = 2;
    static const int PLAY_CMD_RESUME = 3;
    static const int PLAY_CMD_SEEK = 4;
    static const int PLAY_CMD_SETPARAM = 5;

public:
    class TvIObserver {
    public:
        TvIObserver() {};
        virtual ~TvIObserver() {};
        virtual void onTvEvent ( const CTvEv &ev ) = 0;
    };
    //main
    CTv();
    virtual ~CTv();
    virtual int OpenTv ( void );
    virtual int CloseTv ( void );
    virtual int StartTvLock ();
    virtual int StopTvLock ( void );
    int SetTvTunnelId(int tunnel_id);
    virtual int DoSuspend(int type);
    virtual int DoResume(int type);
    virtual int startTvDetect();

    virtual TvRunStatus_t GetTvStatus();
    virtual int ClearAnalogFrontEnd();
    virtual tv_source_input_t GetLastSourceInput (void);
    virtual int SetSourceSwitchInput (tv_source_input_t source_input );
    virtual int SetSourceSwitchInput(tv_source_input_t virtual_input, tv_source_input_t source_input);
    virtual int SetSourceSwitchInputLocked(tv_source_input_t virtual_input, tv_source_input_t source_input);
    virtual tv_source_input_t GetCurrentSourceInputLock ( void );
    virtual tv_source_input_t GetCurrentSourceInputVirtualLock ( void );
    bool isVirtualSourceInput(tv_source_input_t source_input);
    virtual void InitCurrentSignalInfo ( void );
    virtual tvin_info_t GetCurrentSignalInfo ( void );
    int setPreviewWindowMode(bool mode);
    virtual int SetPreviewWindow ( tvin_window_pos_t pos );
    virtual int dtvAutoScan();
    virtual int dtvManualScan (int beginFreq, int endFreq, int modulation = -1);
    virtual int atvAutoScan(int videoStd, int audioStd, int searchType);
    virtual int atvAutoScan(int videoStd, int audioStd, int searchType, int procMode);
    virtual int dtvScan(int mode, int scan_mode, int beginFreq, int endFreq, int para1, int para2);
    virtual int dtvMode(const char *mode);
    virtual int clearFrontEnd(int para);
    virtual int pauseScan();
    virtual int resumeScan();
    virtual int getScanStatus();
    virtual void operateDeviceForScan(int type);
    virtual void setDvbTextCoding(char *coding);
    virtual int Scan(const char *feparas, const char *scanparas);
    virtual int setFrontEnd ( const char *paras, bool force );

    virtual int clearAllProgram(int arg0);
    virtual int clearDbAllProgramInfoTable();
    virtual void setSourceSwitchAndPlay();
    virtual int atvManualScan ( int startFreq, int endFreq, int videoStd, int audioStd, int store_Type = 0, int channel_num = 0 );
    virtual int stopScanLock();
    virtual int playDvbcProgram ( int progId );
    virtual int playDtmbProgram ( int progId );
    virtual int playAtvProgram ( int, int, int, int, int, int, int);
    virtual int playDtvProgram ( int, int, int, int, int, int, int, int, int, int);
    virtual int playDtvProgramUnlocked( int, int, int, int, int, int, int, int, int, int);
    virtual int playDtvProgram(const char *, int, int, int, int, int, int, int, int, int, int);
    virtual int playDtvProgramUnlocked(const char *, int, int, int, int, int, int, int, int, int, int);
    virtual int playDtvProgram(const char *, int, int, int, int, int, int);
    virtual int playDtvProgramUnlocked(const char *, int, int, int, int, int, int);
    virtual int playDtvTimeShift (const char *feparas, void *para, int audioCompetition);
    virtual int playDtvTimeShiftUnlocked(const char *feparas, void *para, int audioCompetition);
    virtual int stopPlayingLock();
    virtual int resetFrontEndPara ( frontend_para_set_t feParms );
    int GetAudioVolumeCompensationVal(int progDbId);
    //dtv audio track info
    int getAudioTrackNum ( int progId );
    int getAudioInfoByIndex ( int progId, int idx, int *pAFmt, String8 &lang );
    int switchAudioTrack ( int progId, int idx );
    int switchAudioTrack ( int aPid, int aFmt, int aParam );
    int setAudioAD(int enable, int aPid, int aFmt);
    int getVideoFormatInfo ( int *pWidth, int *pHeight, int *pFPS, int *pInterlace );
    int getAudioFormatInfo ( int fmt[2], int sample_rate[2], int resolution[2], int channels[2],
        int lfepresent[2], int *frames, int *ab_size, int *ab_data, int *ab_free );
    int ResetAudioDecoderForPCMOutput();
    int  setAudioChannel ( int channelIdx );
    int getAudioChannel();
    int setTvObserver (TvIObserver *ob);
    int getAtscAntenna();
    long getTvTime()
    {
        return mTvTime.getTime();
    };
    int getFrontendSignalStrength();
    int getFrontendSNR();
    int getFrontendBER();
    int getChannelInfoBydbID ( int dbID, channel_info_t &chan_info );
    int saveATVProgramID ( int dbID );
    int getATVProgramID ( void );
    int saveDTVProgramID ( int dbID );
    int getDTVProgramID ( void );
    int saveRadioProgramID ( int dbID );
    int getRadioProgramID ( void );
    int getATVMinMaxFreq ( int *scanMinFreq, int *scanMaxFreq );
    int doRecordingCommand(int cmd, const char *id, const char *param);
    int doPlayCommand(int cmd, const char *id, const char *param);
    int getAverageLuma();
    int setAutobacklightData(const char *value);
    int getAutoBacklightData(int *data);
    virtual int Tv_SSMRestoreDefaultSetting();
    int handleGPIO(const char *port_name, bool is_out, int edge);
    int setLcdEnable(bool enable);
    int Tv_GetIwattRegs();
    int GetSourceConnectStatus(tv_source_input_t source_input);
    int IsDVISignal();
    int isVgaFmtInHdmi();
    int GetTvAction();
    int getHDMIFrameRate ( void );
    int getHdmiFormatInfo ( int *pWidth, int *pHeight, int *pFPS);
    int getVrrMode (void);
    int getAllmInfo (void);
    unsigned int Vpp_GetDisplayResolutionInfo(tvin_window_pos_t *win_pos);
    int setFEStatus(int value);
    //SSM
    virtual int Tv_SSMFacRestoreDefaultSetting();
    int StartHeadSetDetect();
    virtual void onHeadSetDetect(int state, int para);

    CTvFactory mFactoryMode;
    sp<CDevicesPollStatusDetect> mDevicesPollStatusDetectThread;
    CHDMIRxManager mHDMIRxManager;

    CTvSubtitle mSubtitle;
    CTv2d4GHeadSetDetect mHeadSet;

    void SetCurrentLanguage(std::string lang)
    {
        mCurrentSystemLang = lang;
    }

    std::string GetCurrentLanguage()
    {
        return mCurrentSystemLang;
    }

    int SendHDMIRxCECCustomMessage(unsigned char data_buf[]);
    int SendHDMIRxCECCustomMessageAndWaitReply(unsigned char data_buf[], unsigned char reply_buf[], int WaitCmd, int timeout);
    int SendHDMIRxCECBroadcastStandbyMessage();
    int SendHDMIRxCECGiveCECVersionMessage(tv_source_input_t source_input, unsigned char data_buf[]);
    int SendHDMIRxCECGiveDeviceVendorIDMessage(tv_source_input_t source_input, unsigned char data_buf[]);
    int SendHDMIRxCECGiveOSDNameMessage(tv_source_input_t source_input, unsigned char data_buf[]);

    int GetHdmiHdcpKeyKsvInfo(int data_buf[]);
    virtual bool hdmiOutWithFbc();
    int Tv_SetDDDRCMode(tv_source_input_t source_input);
    virtual int Tv_SetVdinForPQ (int gameStatus, int pcStatus, int autoSwitchFlag);
    int Tv_SetDLGEnable(bool enable);
    int Tv_GetDLGEnable();
    int GetAmdvEnable();
    int UpdateDlgProperty(tv_source_input_t source_input);
    int SupportDlg(void);
    int TV_SetVRREnable (bool enable);
    int TV_GetVRREnable(void);
    int SetVRRStatusBySignal(void);
    int SetHdmiColorRangeMode(tvin_color_range_t range_mode);
    int GetHdmiColorRangeMode();
    virtual void updateSubtitle(int, int);
    int GetHdmiAvHotplugDetectOnoff();
    int LoadEdidData(int isNeedBlackScreen, int isDolbyVisionEnable, int isDlgVisionEnable);
    int UpdateEdidData(tv_source_input_t source_input, unsigned char *dataBuf);
    int SetHdmiEdidVersion(tv_hdmi_port_id_t port, tv_hdmi_edid_version_t version);
    int GetHdmiEdidVersion(tv_hdmi_port_id_t port);
    int SaveHdmiEdidVersion(tv_hdmi_port_id_t port, tv_hdmi_edid_version_t version);
    int SetHdmiHDCPSwitcher(tv_hdmi_hdcpkey_enable_t enable);
    void SetHdmiEdidForUboot(void);
    tvin_port_t Tv_GetHdmiPortBySourceInput(tv_source_input_t source_input);
    int SetVideoAxis(int x, int y, int width, int height);
    int Tv_RrtUpdate();
    rrt_select_info_t Tv_RrtSearch(int rating_region_id, int dimension_id, int value_id, int program_id);
    int Tv_Easupdate();
    int Tv_SetWssStatus (int status);
    int Tv_SetDeviceIdForCec (int deviceId);
    void dump(String8 &result);

    int SetAtvAudioOutmode(int mode);
    int GetAtvAudioOutmode();
    int GetAtvAudioInputmode();

    int GetAtvAutoScanMode();
    int SetSnowShowEnable(bool enable);
    int TV_SetSameSourceEnable(bool enable);
    void SetBlocked(bool block, bool tunning);
    void SendBlockEvt(bool blocked);
    int RequestUnblock();
    int getVdecStatusInfo(int vdecId, int *decode_time_cost, int *frame_width, int *frame_height, int *frame_rate,
    int *bit_depth_luma, int *frame_dur, int *bit_depth_chroma, int *error_count, int *status, int *frame_count,
        int *error_frame_count, int *drop_frame_count, int64_t *total_data, int *double_write_mode, int *offset,
        int *ratio_control, int *vf_type, int *signal_type, int *pts, int64_t *pts_us64);
    int getBasicVdecStatusInfo(int vdecId, int *decode_time_cost, int *frame_width, int *frame_height,  int *frame_rate,
	int *error_count, int *frame_count, int *error_frame_count, int *drop_frame_count, int *double_write_mode);
    std::string request(const std::string& resource, const std::string& paras);

    int ScreenColorControl(bool color, int freq);
private:
    int SendCmdToOffBoardFBCExternalDac(int, int);
    int KillMediaServerClient();

    int setPictureModeBySignal(pq_mode_switch_type_t switchType);

    static int prepareRecording(const char *id, const char *param);
    static int startRecording(const char *id, const char *param, CTvRecord::IObserver *observer);
    static int stopRecording(const char *id, const char *param);

    static CTvRecord *getRecorder(const char *id, const char *param);

    int startPlay(const char *id, const char *param);
    int stopPlay(const char *id, const char *param);
    int pausePlay(const char *id, const char *param);
    int resumePlay(const char *id, const char *param);
    int seekPlay(const char *id, const char *param);
    int setPlayParam(const char *id, const char *param);

    CTvPlayer *getPlayer(const char *id, const char *param);

    int tryReleasePlayer(bool isEnter, tv_source_input_t si);

    void setDvbLogLevel();
    bool needSnowEffect();
    bool isChannelBlockStatusChanged();

    //end audio
    bool mATVDisplaySnow;
    bool mAllmModeCfg;
    bool mItcontentModeCfg;
    bool mDviModeCfg;
    bool mAllmInfo;
    bool MnoNeedAutoSwitchToMonitorMode;
    bool mInitEdidVersion;
    std::string mCurrentSystemLang;
    bool m_bTsPlayerRls;
    volatile bool mEnableLockModule;
    bool mSupportChannelLock;
    tv_block_state_e mBlockState;
    tv_block_state_e mChannelBlockState;
    tv_block_state_e mChannelLastBlockState;
    bool mBlockStatusChanged;
    bool mNoneStaticChange;
    bool mIsMultiDemux; //Indicates whether the new path is supported
protected:
    class CTvMsgQueue: public CMsgQueueThread, public CAv::IObserver, public CTvin::IObserver
        , public CTvScanner::IObserver , public CTvEpg::IObserver, public CFrontEnd::IObserver
        , public CTvRecord::IObserver, public CTvRrt::IObserver, public CTvEas::IObserver
        {
    public:
        static const int TV_MSG_COMMON = 0;
        static const int TV_MSG_STOP_ANALYZE_TS = 1;
        static const int TV_MSG_START_ANALYZE_TS = 2;
        static const int TV_MSG_CHECK_FE_DELAY = 3;
        static const int TV_MSG_AV_EVENT = 4;
        static const int TV_MSG_FE_EVENT = 5;
        static const int TV_MSG_SCAN_EVENT = 6;
        static const int TV_MSG_EPG_EVENT = 7;
        static const int TV_MSG_HDMI_SR_CHANGED = 8;
        static const int TV_MSG_ENABLE_VIDEO_LATER = 9;
        static const int TV_MSG_SCANNING_FRAME_STABLE = 10;
        static const int TV_MSG_VIDEO_AVAILABLE_LATER = 11;
        static const int TV_MSG_RECORD_EVENT = 12;
        static const int TV_MSG_RRT_EVENT = 13;
        static const int TV_MSG_EAS_EVENT = 14;
        static const int TV_MSG_TVIN_RES  = 15;
        static const int TV_MSG_CHECK_SOURCE_VALID = 16;

        CTvMsgQueue(CTv *tv);
        ~CTvMsgQueue();
        //scan observer
        void onEvent ( const CTvScanner::ScannerEvent &ev );
        //epg observer
        void onEvent ( const CTvEpg::EpgEvent &ev );
        //FE observer
        void onEvent ( const CFrontEnd::FEEvent &ev );
        //Record
        void onEvent(const CTvRecord::RecEvent &ev);
        //rrt observer
        void onEvent (const CTvRrt::RrtEvent &ev);
        //eas observer
        void onEvent (const CTvEas::EasEvent &ev);
        //AV
        void onEvent(const CAv::AVEvent &ev);
        //TVin(resmanager currently)
        void onEvent(const CTvin::ResourceManEvent &ev);
        void onEvent(const CTvin::CheckSourceValidEvent &ev);
    private:
        virtual void handleMessage ( CMessage &msg );
        CTv *mpTv;
    };

    void onEnableVideoLater(int framecount);
    void onVideoAvailableLater(int framecount);
    //add available frame judge
    void isVideoFrameAvailable(unsigned int u32NewFrameCount = 1);
    int resetDmxAndAvSource();
    int stopScan();
    int stopPlaying(bool isShowTestScreen);
    int stopPlaying(bool isShowTestScreen, bool resetFE);
    void sendTvEvent ( const CTvEv &ev );
    int startPlayTv ( int source, int vid, int aid, int pcrid, int vfat, int afat );
    //scan observer
    void onEvent ( const CTvScanner::ScannerEvent &ev );
    //epg observer
    void onEvent ( const CTvEpg::EpgEvent &ev );
    //FE observer
    void onEvent ( const CFrontEnd::FEEvent &ev );

    //Record
    void onEvent(const CTvRecord::RecEvent &ev);
    //rrt observer
    void onEvent (const CTvRrt::RrtEvent &ev);
    //eas observer
    void onEvent (const CTvEas::EasEvent &ev);
    //AV
    void onEvent(const CAv::AVEvent &ev);
    //TVIN (resmanage currently)
    void onEvent(const CTvin::ResourceManEvent &ev);
    void onEvent(const CTvin::CheckSourceValidEvent &ev);

    bool Tv_Start_Analyze_Ts ( int channelID );
    bool Tv_Stop_Analyze_Ts();
    int Tvin_Stop ( void );
    int Tvin_GetTvinConfig();
    int Tv_init_audio();
    int Tv_MiscSetBySource ( tv_source_input_t );
    void print_version_info ( void );
    int dtvCleanProgramByFreq ( int freq );
    void onSigToStable();
    void onSigToUnstable();
    void onSigToUnSupport();
    void onSigToNoSig();
    void onSigStillStable();
    void onSigStatusChange();
    virtual void onSourceConnect(int source_type, int connect_status);
    virtual void onVdinSignalChange();
    virtual void onThermalDetect(int state);
    virtual void ScreenColorChange(int color);
    virtual void onBootvideoRunning();
    virtual void onBootvideoStopped();

    void onBlockStateChanged(tv_block_state_e state);
    bool isBlockedByChannelLock();

    CTvEpg mTvEpg;
    CTvRrt *mTvRrt;
    CTvScanner *mTvScanner;
    CTvRecord mTvRec;
    CFrontEnd *mFrontDev;
    CTvEas *mTvEas;

    mutable Mutex mLock;
    CTvTime mTvTime;

    CTvDimension mTvVchip;
    CTvSubtitle mTvSub;
    CAv mAv;
    CTvDmx mTvDmx;
    CTvDmx mTvDmx1;
    CTvDmx mTvDmx2;
    sp<CTvMsgQueue> mTvMsgQueue;
    //
    volatile int mTvAction;
    volatile TvRunStatus_t mTvStatus;
    volatile tv_source_input_t m_source_input;
    volatile tv_source_input_t m_last_source_input;
    volatile tv_source_input_t m_source_input_virtual;
    volatile int mLastScreenMode;

    /* for tvin window mode and pos*/
    tvin_window_pos_t m_win_pos;
    tv_window_mode_t m_win_mode;

    //friend class CTvMsgQueue;
    int mCurAnalyzeTsChannelID;
    TvIObserver *mpObserver;
    tv_dtv_scan_running_status_t mDtvScanRunningStatus;
    volatile tv_config_t gTvinConfig;
    bool mAutoSetDisplayFreq;
    int m_sig_spdif_nums;
    /** for HDMI-in sampling detection. **/
    /** for display mode to bottom **/

    CTvin *mpTvin;
    CTvGpio *pGpio;
    CTvPanel *mPanel;
    SysClientcallback *mSysClient;

    bool mPreviewEnabled;
    bool mbSameSourceEnableStatus;
    bool mVrrStatus;
    bool mVRRStatusChange;
    bool mDTvSigStaus;//for dtv static frame func,default true
    bool mDLGStatus;

public:
    friend CTvPlayer;
    friend CDTVTvPlayer;
    friend CATVTvPlayer;
};

#endif  //_CDTV_H
