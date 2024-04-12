/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "tvserver"
#define LOG_TV_TAG "CTv"
#define UNUSED(x) (void)x

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/types.h>
#include <errno.h>
#include <dlfcn.h>
#include <cutils/properties.h>
#include <cutils/android_reboot.h>
#include <utils/threads.h>
#include <time.h>
#include <sys/prctl.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#ifdef ANDROID
#include <termios.h>
#endif
#include <string.h>
#include <signal.h>
#include <hardware_legacy/power.h>
#include <media/AudioSystem.h>
#include <sys/wait.h>

#include <tvutils.h>
#include <tvconfig.h>
#include <tvscanconfig.h>
#include <CFile.h>
#include <serial_operate.h>

#include "CTvDatabase.h"
#include "../version/version.h"
#include "../tvsetting/CTvSetting.h"

#include <hardware_legacy/power.h>

#ifdef SUPPORT_ADTV
#include <am_epg.h>
#include <am_mem.h>

extern "C" {
#include "am_ver.h"
#include "am_misc.h"
#include "am_debug.h"
#include "am_fend.h"
}
#endif

#include <math.h>
#include <sys/ioctl.h>

#include "CTv.h"

using namespace android;

CTv::CTv():mTvDmx(0), mTvDmx1(1), mTvDmx2(2)
{
    /*copy file to param
    char buf[PROPERTY_VALUE_MAX] = {0};
    int len = property_get("tv.tvconfig.force_copy", buf, "true");*/
    mPanel = CTvPanel::getInstance();
    mTvMsgQueue = sp<CTvMsgQueue>::make(this);
    mDevicesPollStatusDetectThread = sp<CDevicesPollStatusDetect>::make();
#ifdef SUPPORT_PANEL
    mPanel->AM_PANEL_Init();
#endif

#ifdef SUPPORT_ADTV
    AM_EVT_Init();
#endif

    mpObserver = NULL;
    mAutoSetDisplayFreq = false;
    mPreviewEnabled = false;
    mDTvSigStaus = true;
    mDLGStatus = false;
    mVrrStatus = false;
    mAllmInfo = false;
    mVRRStatusChange = false;
    mInitEdidVersion = false;
    mEnableLockModule = false;
    mSupportChannelLock = false;
    mBlockState = BLOCK_STATE_NONE;
    mIsLoadEdidWithPort = false;
    mTvMsgQueue->startMsgQueue();
    /*int kernelVersion = getKernelMajorVersion();
    if (kernelVersion > 4) {//kernel 5.4
        tv_config_load (TV_CONFIG_FILE_PATH_1);
        tv_scan_config_load(TV_SCAN_CONFIG_FILE_PATH_1);
    } else {//kernel 4.9
        tv_config_load (TV_CONFIG_FILE_PATH_0);
        tv_scan_config_load(TV_SCAN_CONFIG_FILE_PATH_0);
    }*/
    tv_config_load (TV_CONFIG_FILE_PATH_1);
    tv_scan_config_load(TV_SCAN_CONFIG_FILE_PATH_1);
    mFrontDev = CFrontEnd::getInstance();
    mpTvin = CTvin::getInstance();
    mpTvin->setObserver(mTvMsgQueue.get());
    mTvScanner = CTvScanner::getInstance();
    mTvRrt = CTvRrt::getInstance();
    mTvRrt->setObserver(mTvMsgQueue.get());
    sqlite3_config (SQLITE_CONFIG_SERIALIZED);
    sqlite3_soft_heap_limit(8 * 1024 * 1024);
    // Initialize SQLite.
    sqlite3_initialize();
    CTvDatabase::GetTvDb()->InitTvDb ( TV_DB_PATH );
    mSysClient = SysClientcallback::getInstance();
    mSysClient->setObserver(this);
    if ( CTvDimension::isDimensionTblExist() == false ) {
        CTvDimension::builtinAtscDimensions();
    }

    mFactoryMode.init();
    mDtvScanRunningStatus = DTV_SCAN_RUNNING_NORMAL;

    SetHdmiEdidForUboot();
    mDevicesPollStatusDetectThread->setObserver ( this );
    mFrontDev->setObserver ( mTvMsgQueue.get() );
    mTvEas = CTvEas::GetInstance();
    mTvEas->setObserver(mTvMsgQueue.get());
    mSubtitle.setObserver(this);
    mHeadSet.setObserver(this);
    mAv.setObserver(mTvMsgQueue.get());
    AM_DMX_OpenPara_t para;
    memset(&para, 0, sizeof(para));
    mTvDmx.Open (para);
    mTvDmx1.Open (para);
    mTvDmx2.Open (para);
    mTvStatus = TV_INIT_ED;
    pGpio = new CTvGpio();
    print_version_info();
    mbSameSourceEnableStatus = false;
    mATVDisplaySnow = false;
    mAllmModeCfg = false;
    mItcontentModeCfg = false;
    mDviModeCfg = false;
    MnoNeedAutoSwitchToMonitorMode = false;
    mTvAction = TV_ACTION_NULL;
    m_source_input = SOURCE_INVALID;
    m_last_source_input = SOURCE_INVALID;
    m_source_input_virtual = SOURCE_INVALID;
    mLastScreenMode = -1;
    m_win_pos.x1 = -1;
    m_win_pos.x2 = -1;
    m_win_pos.y1 = -1;
    m_win_pos.y2 = -1;
    m_win_mode   = NORMAL_WINDOW;
    mCurAnalyzeTsChannelID = -1;
    InitCurrentSignalInfo();
    InitCurrentVdin2SignalInfo();
    //init gTvinConfig
    gTvinConfig.kernelpet_disable = false;
    gTvinConfig.kernelpet_timeout = -1;
    gTvinConfig.memory512m        = false;
    gTvinConfig.userpet           = false;
    gTvinConfig.userpet_reset     = -1;
    gTvinConfig.userpet_timeout   = -1;
    m_sig_spdif_nums              = -1;
    m_bTsPlayerRls                = true;
}

CTv::~CTv()
{
    mpObserver = NULL;
    CTvDatabase::deleteTvDb();
    tv_config_unload();
    tv_scan_config_unload();
    mAv.Close();
    mTvStatus = TV_INIT_ED;
    mFrontDev->Close();

    if (pGpio != NULL) {
        delete pGpio;
        pGpio = NULL;
    }
}

void CTv::onEvent ( const CTvScanner::ScannerEvent &ev )
{
    LOGD ( "%s, CTv::onEvent lockStatus = %d type = %d\n", __FUNCTION__,  ev.mLockedStatus, ev.mType );

    if ( mDtvScanRunningStatus == DTV_SCAN_RUNNING_ANALYZE_CHANNEL ) {
        if ( ev.mType == CTvScanner::ScannerEvent::EVENT_SCAN_END ) {
            CMessage msg;
            msg.mType = CTvMsgQueue::TV_MSG_STOP_ANALYZE_TS;
            msg.mpData = this;
            mTvMsgQueue->sendMsg ( msg );
        } else if ( ev.mType == CTvScanner::ScannerEvent::EVENT_STORE_END ) {
            CTvEpg::EpgEvent epgev;
            epgev.type = CTvEpg::EpgEvent::EVENT_CHANNEL_UPDATE_END;
            mDtvScanRunningStatus = DTV_SCAN_RUNNING_NORMAL;
            sendTvEvent ( epgev );
        }
    } else {
        sendTvEvent ( ev );
    }
}

void CTv::onEvent ( const CTvEas::EasEvent &ev )
{
    LOGD("EAS event!\n");
    sendTvEvent(ev);
}

void CTv::onEvent ( const CFrontEnd::FEEvent &ev )
{
    LOGD ( "%s, FE event type = %d tvaction=%x", __FUNCTION__, ev.mCurSigStaus, mTvAction);
    if (mTvAction & TV_ACTION_SCANNING) return;


    if ( ev.mCurSigStaus == CFrontEnd::FEEvent::EVENT_FE_HAS_SIG ) {
        LOGD("onEvent fe lock, Tsplayer Status:[%d]", tsplayerStatus);
        if (/*m_source_input == SOURCE_TV || */m_source_input == SOURCE_DTV && (mTvAction & TV_ACTION_PLAYING)) {//atv and other tvin source    not to use it, and if not playing, not use have sig
            mDTvSigStaus = true;
            TvEvent::SignalInfoEvent ev;
            ev.mStatus = TVIN_SIG_STATUS_STABLE;
            ev.mTrans_fmt = TVIN_TFMT_2D;
            ev.mFmt = TVIN_SIG_FMT_NULL;
            ev.mReserved = 0;
            sendTvEvent ( ev );
            setFEStatus(1);

            if (BLOCK_STATE_BLOCKED == mChannelBlockState) {
                LOGD("%s, current has been block, return!", __FUNCTION__);
                return;
            }

            //only tsplayer is playing to show
            if (tsplayerStatus == CAv::AVEvent::EVENT_AV_RESUME ||
                tsplayerStatus > CAv::AVEvent::EVENT_AV_STOP) {
                ScreenColorControl(false, VIDEO_LAYER_COLOR_SHOW_DISABLE);
            }
            //When switching streams, enabling the video layer may lead to screen flickering.
            //DTV enable video follw avevent.
            /*
            if (mIsMultiDemux) {
                CVideotunnel::getInstance()->VT_disableColorFrame();
                mAv.SetVideoScreenColor(VIDEO_LAYER_BLACK);
            } else {
                CVpp::getInstance()->VPP_setVideoColor(false);
            }
            */
            if (propertyGetBool("vendor.tv.dtv.tsplayer.enable", false)) {
                mAv.EnableVideoNow(false);
            }
        }
    } else if ( ev.mCurSigStaus == CFrontEnd::FEEvent::EVENT_FE_NO_SIG ) {
        LOGD("onEvent fe unlock");
        setFEStatus(0);
        if (propertyGetBool("vendor.tv.dtv.tsplayer.enable", false)) {
            if (m_source_input == SOURCE_DTV &&  mTvAction & TV_ACTION_STOPING ) {
                LOGD("tv stopping, no need CAv::AVEvent::EVENT_AV_STOP");
                return;
            }
            /*
            if (mIsMultiDemux) {
                CVideotunnel::getInstance()->VT_setvideoColor(true, true);
                if (getScreenColorSetting()) {
                    mAv.SetVideoScreenColor(VIDEO_LAYER_BLUE);
                } else {
                    mAv.SetVideoScreenColor(VIDEO_LAYER_BLACK);
                }
            } else {
                CVpp::getInstance()->VPP_setVideoColor(true);
            }
            */
            ScreenColorControl(true, VIDEO_LAYER_COLOR_SHOW_ALWAYES);

            mDTvSigStaus = false;
            TvEvent::SignalInfoEvent ev;
            ev.mStatus = TVIN_SIG_STATUS_NOSIG;
            ev.mTrans_fmt = TVIN_TFMT_2D;
            ev.mFmt = TVIN_SIG_FMT_NULL;
            ev.mReserved = 0;
            sendTvEvent ( ev );
        }
    } else if (ev.mCurSigStaus == CFrontEnd::FEEvent::EVENT_VLFE_HAS_SIG) {
        LOGD("onEvent vlfe lock");
    } else if (ev.mCurSigStaus == CFrontEnd::FEEvent::EVENT_VLFE_NO_SIG) {
        LOGD("onEvent vlfe unlock");
    }
}

void CTv::onEvent ( const CTvEpg::EpgEvent &ev )
{
    switch ( ev.type ) {
    case CTvEpg::EpgEvent::EVENT_TDT_END:
        LOGD ( "%s, CTv::onEvent epg time = %ld", __FUNCTION__, ev.time );
        mTvTime.setTime ( ev.time );
        break;

    case CTvEpg::EpgEvent::EVENT_CHANNEL_UPDATE: {
        LOGD ( "%s, CTv:: onEvent channel update", __FUNCTION__ );
        CMessage msg;
        msg.mType = CTvMsgQueue::TV_MSG_START_ANALYZE_TS;
        msg.mpData = this;
        mCurAnalyzeTsChannelID = ev.channelID;
        mTvMsgQueue->sendMsg ( msg );
        break;
    }

    default:
        break;
    }

    sendTvEvent ( ev );
}

void CTv::onEvent(const CTvRecord::RecEvent &ev)
{
    LOGD ( "RecEvent = %d", ev.type);
    switch ( ev.type ) {
        case CTvRecord::RecEvent::EVENT_REC_START: {
            TvEvent::RecorderEvent RecorderEvent;
            RecorderEvent.mStatus = TvEvent::RecorderEvent::EVENT_RECORD_START;
            RecorderEvent.mError = (int)ev.error;
            RecorderEvent.mId = String8(ev.id.c_str());
            sendTvEvent(RecorderEvent);
            }break;
        case CTvRecord::RecEvent::EVENT_REC_STOP: {
            TvEvent::RecorderEvent RecorderEvent;
            RecorderEvent.mStatus = TvEvent::RecorderEvent::EVENT_RECORD_STOP;
            RecorderEvent.mError = (int)ev.error;
            RecorderEvent.mId = String8(ev.id.c_str());
            sendTvEvent(RecorderEvent);
            }break;
        default:
            break;
    }
}

void CTv::onEvent (const CTvRrt::RrtEvent &ev)
{
    LOGD("RRT event!\n");
    sendTvEvent ( ev );
}

void CTv::onEvent(const CAv::AVEvent &ev)
{
    LOGD ( "onEvent AVEvent = %d", ev.type);
    tsplayerStatus = ev.type;
    switch ( ev.type ) {
    case CAv::AVEvent::EVENT_AV_STOP: {
        if ( mTvAction & TV_ACTION_STOPING || mTvAction & TV_ACTION_PLAYING) {
            LOGD("tv stopping or playing, no need CAv::AVEvent::EVENT_AV_STOP");
            break;
        }
        //CVpp::getInstance()->VPP_setVideoColor(true);
        ScreenColorControl(true, VIDEO_LAYER_COLOR_SHOW_ALWAYES);
        TvEvent::SignalInfoEvent ev;
        ev.mStatus = TVIN_SIG_STATUS_NOSIG;
        ev.mTrans_fmt = TVIN_TFMT_2D;
        ev.mFmt = TVIN_SIG_FMT_NULL;
        ev.mReserved = 0;
        sendTvEvent ( ev );
        break;
    }

    case CAv::AVEvent::EVENT_AV_RESUME: {
        if (m_source_input == SOURCE_DTV) {
        LOGD("EVENT_AV_VIDEO_RESUME, video available");
        isVideoFrameAvailable();
        mAv.SetVideoLayerStatus(ENABLE_AND_CLEAR_VIDEO_LAYER);
            }
        TvEvent::AVPlaybackEvent AvPlayBackEvt;
        AvPlayBackEvt.mMsgType = TvEvent::AVPlaybackEvent::EVENT_AV_PLAYBACK_RESUME;
        AvPlayBackEvt.mProgramId = (int)ev.param;
        sendTvEvent(AvPlayBackEvt);
        break;
    }

    case CAv::AVEvent::EVENT_AV_SCAMBLED: {
        if (mCurrentProgramIsScambled) {
        LOGD("Scambled program");
        //CVpp::getInstance()->VPP_setVideoColor(true);
        ScreenColorControl(true, VIDEO_LAYER_COLOR_SHOW_ALWAYES);
        TvEvent::AVPlaybackEvent AvPlayBackEvt;
        AvPlayBackEvt.mMsgType = TvEvent::AVPlaybackEvent::EVENT_AV_SCAMBLED;
        AvPlayBackEvt.mProgramId = (int)ev.param;
        sendTvEvent(AvPlayBackEvt);
        }

        break;
    }
    case CAv::AVEvent::EVENT_AV_VIDEO_AVAILABLE: {
        LOGD("EVENT_AV_VIDEO_AVAILABLE, video available");
        if (m_source_input == SOURCE_DTV && (mTvAction & TV_ACTION_PLAYING)) { //atv and other tvin source    not to use it, and if not playing, not use have sig
            if ( m_win_mode == PREVIEW_WINDOW ) {
                mAv.setVideoScreenMode ( CAv::VIDEO_WIDEOPTION_FULL_STRETCH );
            }
            if (mAutoSetDisplayFreq && !mPreviewEnabled) {
                mpTvin->VDIN_SetDisplayVFreq(50);
            }
        }
        isVideoFrameAvailable();
        if (!mIsMultiDemux) {
            mAv.SetVideoLayerStatus(ENABLE_AND_CLEAR_VIDEO_LAYER);
        }
        TvEvent::AVPlaybackEvent AvPlayBackEvt;
        AvPlayBackEvt.mMsgType = TvEvent::AVPlaybackEvent::EVENT_AV_VIDEO_AVAILABLE;
        AvPlayBackEvt.mProgramId = (int)ev.param;
        sendTvEvent(AvPlayBackEvt);
        break;
    }
    case CAv::AVEvent::EVENT_AV_UNSUPPORT: {
        LOGD("EVENT_AV_UNSUPPORT");
        ScreenColorControl(false, VIDEO_LAYER_COLOR_SHOW_ALWAYES);
        TvEvent::AVPlaybackEvent AvPlayBackEvt;
        AvPlayBackEvt.mMsgType = TvEvent::AVPlaybackEvent::EVENT_AV_UNSUPPORT;
        AvPlayBackEvt.mProgramId = (int)ev.param;
        sendTvEvent(AvPlayBackEvt);
        break;
    }
    case CAv::AVEvent::EVENT_PLAY_UPDATE:
    case CAv::AVEvent::EVENT_PLAY_CURTIME_CHANGE:
    case CAv::AVEvent::EVENT_PLAY_STARTTIME_CHANGE:
    {
        //fix me: get the player, [only one player can run concurrently]
        //the av event should be handled by player..
        //need refactoring here
        CDTVTvPlayer *player = (CDTVTvPlayer *)PlayerManager::getInstance().getFirstDevWithIdContains("atsc");
        if (player)
            player->onPlayUpdate(ev);
        else {
            LOGD("no atsc player found");
        }
        break;
    }
    case CAv::AVEvent::EVENT_AUDIO_CB: {
        TvEvent::AVAudioCBEvent AvAudiocbEvt;

        LOGD("cmd:%d, param1:%d,param2:%d\n",ev.status,ev.param,ev.param1);

        AvAudiocbEvt.cmd = ev.status;
        AvAudiocbEvt.param1 = ev.param;
        AvAudiocbEvt.param2 = ev.param1;
        sendTvEvent(AvAudiocbEvt);
        break;
    }
    case CAv::AVEvent::EVENT_PLAY_INSTANCE: {
        TvEvent::PlayerInstanceEvent PlayerInstEvt;

        LOGD("EVENT_PLAY_INSTANCE cmd:%d, param1:%d,param2:%d\n", ev.status, ev.param, ev.param1);

        PlayerInstEvt.mMsgType = TvEvent::PlayerInstanceEvent::EVENT_PLAY_INSTANCE;
        PlayerInstEvt.type = ev.param;
        PlayerInstEvt.id = ev.param1;
        sendTvEvent(PlayerInstEvt);
        break;
    }
    default:
        break;
    }
}

void CTv::onEvent(const CTvin::ResourceManEvent &ev)
{
    if (mpTvin->Tvin_getResmanHandle() != -1) {
        StopTvLock();
    }
    TvEvent::ResourceManEvent tvinResManEvt;
    tvinResManEvt.cmd = TvEvent::ResourceManEvent::EVENT_RES_ON_PREEMPT;
    sendTvEvent(ev);
}

void CTv::onEvent(const CTvin::CheckSourceValidEvent &ev)
{
    LOGD("CheckSourceValidEvent event!\n");
    sendTvEvent(ev);
}

CTv::CTvMsgQueue::CTvMsgQueue(CTv *tv)
{
    mpTv = tv;
}

CTv::CTvMsgQueue::~CTvMsgQueue()
{
}

void CTv::CTvMsgQueue::handleMessage ( CMessage &msg )
{
    LOGD ("%s, CTv::CTvMsgQueue::handleMessage type = %d", __FUNCTION__,  msg.mType);

    switch ( msg.mType ) {
    case TV_MSG_COMMON:
        break;

    case TV_MSG_STOP_ANALYZE_TS:
        //mpTv->Tv_Stop_Analyze_Ts();
        break;

    case TV_MSG_START_ANALYZE_TS:
        //mpTv->Tv_Start_Analyze_Ts ( pTv->mCurAnalyzeTsChannelID );
        break;

    case TV_MSG_CHECK_FE_DELAY: {
        mpTv->mFrontDev->checkStatusOnce();
        break;
    }

    case TV_MSG_AV_EVENT: {
        mpTv->onEvent(*((CAv::AVEvent *)(msg.mpPara)));
        break;
    }

    case TV_MSG_FE_EVENT: {
        mpTv->onEvent(*((CFrontEnd::FEEvent *)(msg.mpPara)));
        break;
    }

    case TV_MSG_SCAN_EVENT: {
        mpTv->onEvent(*((CTvScanner::ScannerEvent *)(msg.mpPara)));
        break;
    }

    case TV_MSG_EPG_EVENT: {
        mpTv->onEvent(*((CTvEpg::EpgEvent *)(msg.mpPara)));
        break;
    }

    case TV_MSG_ENABLE_VIDEO_LATER: {
        //int fc = msg.mpPara[0];
        mpTv->isVideoFrameAvailable();
        break;
    }

    case TV_MSG_VIDEO_AVAILABLE_LATER: {
        int fc = msg.mpPara[0];
        mpTv->onVideoAvailableLater(fc);
        break;
    }

    case TV_MSG_RECORD_EVENT: {
        mpTv->onEvent(*((CTvRecord::RecEvent *)(msg.mpPara)));
        break;
    }

    case TV_MSG_RRT_EVENT: {
        mpTv->onEvent(*((CTvRrt::RrtEvent *)(msg.mpPara)));
        break;
    }

    case TV_MSG_EAS_EVENT: {
        mpTv->onEvent(*((CTvEas::EasEvent *)(msg.mpPara)));
        break;
    }

    case TV_MSG_TVIN_RES: {
        mpTv->onEvent(*((CTvin::ResourceManEvent *)(msg.mpPara)));
        break;
    }

    case TV_MSG_CHECK_SOURCE_VALID: {
        mpTv->onEvent(*((CTvin::CheckSourceValidEvent *)(msg.mpPara)));
        break;
    }

    default:
        break;
    }
}

void CTv::CTvMsgQueue::onEvent ( const CTvScanner::ScannerEvent &ev )
{
    CMessage msg;
    msg.mDelayMs = 0;
    msg.mType = CTvMsgQueue::TV_MSG_SCAN_EVENT;
    memcpy(msg.mpPara, (void*)&ev, sizeof(ev));
    this->sendMsg ( msg );
}

void CTv::CTvMsgQueue::onEvent ( const CFrontEnd::FEEvent &ev )
{
    CMessage msg;
    msg.mDelayMs = 0;
    msg.mType = CTvMsgQueue::TV_MSG_FE_EVENT;
    memcpy(msg.mpPara, (void*)&ev, sizeof(ev));
    this->sendMsg ( msg );
}

void CTv::CTvMsgQueue::onEvent ( const CTvEpg::EpgEvent &ev )
{
    CMessage msg;
    msg.mDelayMs = 0;
    msg.mType = CTvMsgQueue::TV_MSG_EPG_EVENT;
    memcpy(msg.mpPara, (void*)&ev, sizeof(ev));
    this->sendMsg ( msg );
}

void CTv::CTvMsgQueue::onEvent(const CAv::AVEvent &ev)
{
    CMessage msg;
    msg.mDelayMs = 0;
    msg.mType = CTvMsgQueue::TV_MSG_AV_EVENT;
    memcpy(msg.mpPara, (void*)&ev, sizeof(ev));
    this->sendMsg ( msg );
}

void CTv::CTvMsgQueue::onEvent(const CTvRecord::RecEvent &ev)
{
    CMessage msg;
    msg.mDelayMs = 0;
    msg.mType = CTvMsgQueue::TV_MSG_RECORD_EVENT;
    memcpy(msg.mpPara, (void*)&ev, sizeof(ev));
    this->sendMsg ( msg );
}

void CTv::CTvMsgQueue::onEvent ( const CTvRrt::RrtEvent &ev )
{
    CMessage msg;
    msg.mDelayMs = 0;
    msg.mType = CTvMsgQueue::TV_MSG_RRT_EVENT;
    memcpy(msg.mpPara, (void*)&ev, sizeof(ev));
    char* rrt_info = (char*)malloc(sizeof(rrt_info_t));
    if (rrt_info) {
        CTvRrt::RrtEvent *pRrtEvent = (CTvRrt::RrtEvent *) (&ev);
        memcpy(rrt_info, (void*)(pRrtEvent->mRrtInfo), sizeof(rrt_info_t));
        ((CTvRrt::RrtEvent *)(msg.mpPara))->mRrtInfo = (rrt_info_t*)(rrt_info);
        this->sendMsg ( msg );
    } else {
        LOGD("[TV] cannot malloc %d size", sizeof(rrt_info_t));
    }
}

void CTv::CTvMsgQueue::onEvent ( const CTvEas::EasEvent &ev )
{
    CMessage msg;
    msg.mDelayMs = 0;
    msg.mType = CTvMsgQueue::TV_MSG_EAS_EVENT;
    memcpy(msg.mpPara, (void*)&ev, sizeof(ev));
    this->sendMsg ( msg );
}

void CTv::CTvMsgQueue::onEvent(const CTvin::ResourceManEvent &ev)
{
    CMessage msg;
    msg.mDelayMs = 0;
    msg.mType = CTvMsgQueue::TV_MSG_TVIN_RES;
    memcpy(msg.mpPara, (void*)&ev, sizeof(ev));
    this->sendMsg ( msg );
}

void CTv::CTvMsgQueue::onEvent(const CTvin::CheckSourceValidEvent &ev)
{
    CMessage msg;
    msg.mDelayMs = 0;
    msg.mType = CTvMsgQueue::TV_MSG_CHECK_SOURCE_VALID;
    memcpy(msg.mpPara, (void*)&ev, sizeof(ev));
    this->sendMsg ( msg );
}

int CTv::setTvObserver ( TvIObserver *ob )
{
    mpObserver = ob;
    return 0;
}

void CTv::sendTvEvent ( const CTvEv &ev )
{
    /*if (ev.getEvType() == CTvEv::TV_EVENT_SIGNAL_DETECT) {
        TvEvent::SignalInfoEvent *pEv = (TvEvent::SignalInfoEvent *)(&ev);
    }*/

    LOGD ( "%s, tvaction=%x", __FUNCTION__, mTvAction);
    if ((mTvAction & TV_ACTION_SCANNING) && (ev.getEvType() == CTvEv::TV_EVENT_SIGNAL_DETECT)) {
        LOGD("%s, when scanning, not send sig detect event", __FUNCTION__);
        return;
    }
    if ( mpObserver != NULL ) {
        mpObserver->onTvEvent ( ev );
        if (ev.getEvType() == CTvEv::TV_EVENT_RRT) {
            CTvRrt::RrtEvent *pRrtEvent = (CTvRrt::RrtEvent *) (&ev);
            if (pRrtEvent->mRrtInfo) {
                free((void*)(pRrtEvent->mRrtInfo));
            }
        }
    }
}

int CTv::SetTvTunnelId(int tunnel_id)
{
    LOGD("%s: tunnel id = %d, mIsMultiDemux = %s\n", __FUNCTION__, tunnel_id,tunnel_id >= 0?"true":"false");
    CVideotunnel::getInstance()->SetTunnelId(tunnel_id);
    if (tunnel_id >= 0) {
        mIsMultiDemux = true;
    } else {
        mIsMultiDemux = false;
    }
    return 0;
}

int CTv::ClearAnalogFrontEnd()
{
    return mFrontDev->ClearAnalogFrontEnd ();
}

int CTv::clearFrontEnd(int para __unused)
{
    return 0;/*no need*/
    //mFrontDev->setPara(para, 470000000, 0, 0);
}

int CTv::Scan(const char *feparas, const char *scanparas) {
    AutoMutex _l(mLock);
    mTvAction = mTvAction | TV_ACTION_SCANNING;
    LOGD("%s: mTvAction = 0x%x\n", __FUNCTION__, mTvAction);
    LOGD("%s: fe[%s], scan[%s]!\n", __FUNCTION__, feparas, scanparas);

    mAv.StopTS();
    mFrontDev->setMode(TV_FE_AUTO); /* force exit play. */
    mpTvin->Tvin_StopDecoder();

    CTvScanner::ScanParas sp(scanparas);
    CFrontEnd::FEParas fp(feparas);

    int AtvMode = sp.getAtvMode();
    int DtvMode = sp.getDtvMode();
    LOGD("%s: AtvMode = %d, DtvMode = %d\n", __FUNCTION__, AtvMode, DtvMode);
    if (AtvMode != TV_SCAN_ATVMODE_NONE) {
        m_source_input = SOURCE_TV;
    } else {
        m_source_input = SOURCE_INVALID;
    }

    CTvin::CheckSourceValidEvent evt;
    evt.source = SOURCE_TV;
    mpTvin->sendEvent(evt);

    if (AtvMode & TV_SCAN_ATVMODE_AUTO) {
        LOGD("%s: clean all ATV channal!\n", __FUNCTION__);
        CTvProgram::CleanAllProgramBySrvType ( CTvProgram::TYPE_ATV );
    }

    if (DtvMode & TV_SCAN_DTVMODE_MANUAL) {
        LOGD("%s: clean appointed DTV channal!\n", __FUNCTION__);
        CTvChannel::DeleteBetweenFreq(sp.getDtvFrequency1(), sp.getDtvFrequency2());
    } else {
        LOGD("%s: clean All DTV channal!\n", __FUNCTION__);
        CTvProgram::CleanAllProgramBySrvType ( CTvProgram::TYPE_DTV );
        CTvProgram::CleanAllProgramBySrvType ( CTvProgram::TYPE_RADIO );
        CTvEvent::CleanAllEvent();
    }

    mFrontDev->Open(TV_FE_AUTO);
    mTvScanner->setObserver(mTvMsgQueue.get());
    mDtvScanRunningStatus = DTV_SCAN_RUNNING_NORMAL;
    mTvScanner->SetCurrentLanguage(GetCurrentLanguage());

    return mTvScanner->Scan(fp, sp);
}

int CTv::dtvScan(int mode, int scan_mode, int beginFreq, int endFreq, int para1, int para2) {
    AutoMutex _l(mLock);
    mTvAction = mTvAction | TV_ACTION_SCANNING;
    LOGD("mTvAction = %#x, %s", mTvAction, __FUNCTION__);
    LOGD("mode[%#x], scan_mode[%#x], freq[%d-%d], para[%d,%d] %s",
        mode, scan_mode, beginFreq, endFreq, para1, para2, __FUNCTION__);
    //mTvEpg.leaveChannel();
    //CVpp::getInstance()->VPP_setVideoColor(true);
    mAv.StopTS();

    if (scan_mode == TV_SCAN_DTVMODE_MANUAL) {
        CTvChannel::DeleteBetweenFreq(beginFreq, endFreq);
    } else {
        CTvProgram::CleanAllProgramBySrvType ( CTvProgram::TYPE_DTV );
        CTvProgram::CleanAllProgramBySrvType ( CTvProgram::TYPE_RADIO );
        CTvEvent::CleanAllEvent();
    }
    mTvScanner->setObserver(mTvMsgQueue.get());
    mDtvScanRunningStatus = DTV_SCAN_RUNNING_NORMAL;
    mTvScanner->SetCurrentLanguage(GetCurrentLanguage());

    return mTvScanner->dtvScan(mode, scan_mode, beginFreq, endFreq, para1, para2);
}

int CTv::dtvMode(const char *mode)
{
    if ( strcmp ( mode, DTV_DTMB_MODE ) == 0 )
        return (TV_FE_DTMB);
    else if ( strcmp ( mode, DTV_DVBC_MODE ) == 0 )
        return (TV_FE_QAM);
    else if ( strcmp ( mode, DTV_DVBS_MODE ) == 0 )
        return (TV_FE_QPSK);
    else if ( strcmp ( mode, DTV_ATSC_MODE ) == 0 )
        return (TV_FE_ATSC);
    else if ( strcmp ( mode, DTV_DVBT_MODE ) == 0 )
        return (TV_FE_OFDM);
    else if ( strcmp ( mode, DTV_ISDBT_MODE ) == 0 )
        return (TV_FE_ISDBT);

    return (TV_FE_DTMB);
}

int CTv::dtvAutoScan()
{
    const char *dtv_mode = config_get_str ( CFG_SECTION_TV, CFG_DTV_MODE, DTV_DTMB_MODE);
    return dtvScan(dtvMode(dtv_mode), TV_SCAN_DTVMODE_ALLBAND, 0, 0, -1, -1);
}

int CTv::dtvCleanProgramByFreq ( int freq )
{
    int iOutRet = 0;
    CTvChannel channel;
    iOutRet = CTvChannel::SelectByFreq ( freq, channel );
    if ( -1 == iOutRet ) {
        LOGD("%s, Warning: Current Freq not have program info in the ts_table\n", __FUNCTION__);
    }

    iOutRet = CTvProgram::deleteChannelsProgram ( channel );
    return iOutRet;
}

int CTv::dtvManualScan (int beginFreq, int endFreq, int modulation)
{
    const char *dtv_mode = config_get_str ( CFG_SECTION_TV, CFG_DTV_MODE, "dtmb");
    return dtvScan(dtvMode(dtv_mode), TV_SCAN_DTVMODE_MANUAL, beginFreq, endFreq, modulation, -1);

}

//searchType 0:not 256 1:is 256 Program
int CTv::atvAutoScan(int videoStd, int audioStd, int searchType, int procMode)
{
    int minScanFreq, maxScanFreq, vStd, aStd;
    AutoMutex _l( mLock );
    mTvAction |= TV_ACTION_SCANNING;
    stopPlaying(false);
    mTvScanner->setObserver ( mTvMsgQueue.get() );
    getATVMinMaxFreq (&minScanFreq, &maxScanFreq );
    if ( minScanFreq == 0 || maxScanFreq == 0 || minScanFreq > maxScanFreq ) {
        LOGE ( "%s, auto scan  freq set is error min=%d, max=%d", __FUNCTION__, minScanFreq, maxScanFreq );
        return -1;
    }
    if (needSnowEffect()) {
        SetSnowShowEnable(true);
    } else {
        mpTvin->Tvin_StopDecoder();
    }

    vStd = videoStd;
    aStd = audioStd;
    tvin_port_t source_port = mpTvin->Tvin_GetSourcePortBySourceInput(SOURCE_TV);
    mpTvin->VDIN_OpenPort ( source_port );
    unsigned long stdAndColor = mFrontDev->enumToStdAndColor(vStd, aStd);

    //int fmt = CFrontEnd::stdEnumToCvbsFmt (0, 0);
    mpTvin->AFE_SetCVBSStd ( ( tvin_sig_fmt_t ) TVIN_SIG_FMT_NULL );

    if (searchType == 0) {
        CTvProgram::CleanAllProgramBySrvType ( CTvProgram::TYPE_ATV );
    } else if (searchType == 1) { //type for skyworth, and insert 256 prog, and just update ts table
    }
    minScanFreq = mFrontDev->formatATVFreq ( minScanFreq );
    maxScanFreq = mFrontDev->formatATVFreq ( maxScanFreq );
    LOGD("%s, atv auto scan vstd=%d, astd=%d stdAndColor=0x%x", __FUNCTION__, vStd, aStd, stdAndColor);
    mTvScanner->SetCurrentLanguage(GetCurrentLanguage());

    mTvScanner->autoAtvScan ( minScanFreq, maxScanFreq, stdAndColor, searchType, procMode);
    return 0;
}

int CTv::atvAutoScan(int videoStd __unused, int audioStd __unused, int searchType)
{
    return atvAutoScan(videoStd, audioStd, searchType, 0);
}

int CTv::clearAllProgram(int arg0 __unused)
{
    CTvProgram::CleanAllProgramBySrvType ( CTvProgram::TYPE_ATV );
    CTvProgram::CleanAllProgramBySrvType ( CTvProgram::TYPE_TV );
    CTvProgram::CleanAllProgramBySrvType ( CTvProgram::TYPE_DTV );
    CTvProgram::CleanAllProgramBySrvType ( CTvProgram::TYPE_RADIO);
    return 0;
}

int CTv::atvManualScan ( int startFreq, int endFreq, int videoStd, int audioStd,
                         int store_Type , int channel_num )
{
    int minScanFreq, maxScanFreq, vStd, aStd;

    minScanFreq = mFrontDev->formatATVFreq ( startFreq );
    maxScanFreq = mFrontDev->formatATVFreq ( endFreq );
    if ( minScanFreq == 0 || maxScanFreq == 0 ) {
        LOGE ( "%s, manual scan  freq set is error min=%d, max=%d", __FUNCTION__, minScanFreq, maxScanFreq );
        return -1;
    }

    //if  set std null AUTO, use default PAL/DK
    //if(videoStd == CC_ATV_VIDEO_STD_AUTO) {
    //    vStd = CC_ATV_VIDEO_STD_PAL;
    //    aStd = CC_ATV_AUDIO_STD_DK;
    // } else {
    vStd = videoStd;
    aStd = audioStd;
    // }

    AutoMutex _l( mLock );
    if (needSnowEffect()) {
        SetSnowShowEnable(true);
    }
    mTvAction |= TV_ACTION_SCANNING;
    mTvScanner->setObserver ( mTvMsgQueue.get() );
    unsigned long stdAndColor = mFrontDev->enumToStdAndColor(vStd, aStd);
    tvin_port_t source_port = mpTvin->Tvin_GetSourcePortBySourceInput(SOURCE_TV);
    mpTvin->VDIN_OpenPort ( source_port );

    //int fmt = CFrontEnd::stdEnumToCvbsFmt (0, 0);
    mpTvin->AFE_SetCVBSStd ( ( tvin_sig_fmt_t ) TVIN_SIG_FMT_NULL );
    LOGD("%s, atv manual scan vstd=%d, astd=%d stdAndColor=0x%x", __FUNCTION__, vStd, aStd, stdAndColor);
    mTvScanner->SetCurrentLanguage(GetCurrentLanguage());

    return mTvScanner->ATVManualScan ( minScanFreq, maxScanFreq, stdAndColor, store_Type, channel_num);
}

int CTv::getVideoFormatInfo ( int *pWidth, int *pHeight, int *pFPS, int *pInterlace )
{
    int  iOutRet = -1;
    do {
        if ( NULL == pWidth || NULL == pHeight || NULL == pFPS || NULL == pInterlace ) {
            break;
        }

        iOutRet = mAv.GetVideoStatus (pWidth, pHeight, pFPS, pInterlace);

        if ( DVB_SUCCESS != iOutRet ) {
            LOGD ( "%s, ERROR: Cann't Get video format info\n", __FUNCTION__);
            break;
        }
        //LOGD("%s, w : %d h : %d fps : %d  interlaced: %d\n", __FUNCTION__, video_status.src_w,video_status.src_h,video_status.fps,video_status.interlaced);
    } while ( false );
    return iOutRet;
}

int CTv::getAudioFormatInfo ( int fmt[2], int sample_rate[2], int resolution[2], int channels[2],
    int lfepresent[2], int *frames, int *ab_size, int *ab_data, int *ab_free)
{
    int  iOutRet = -1;

    do {
        iOutRet = mAv.GetAudioStatus (fmt, sample_rate, resolution, channels, lfepresent, frames, ab_size, ab_data, ab_free);
        if ( DVB_SUCCESS != iOutRet ) {
            LOGD ( "%s, ERROR: Cann't Get audio format info\n", __FUNCTION__);
            break;
        }
    } while ( false );

    return iOutRet;
}

int CTv::stopScanLock()
{
    AutoMutex _l( mLock );
    return stopScan();
}

int CTv::stopScan()
{
    if (!(mTvAction & TV_ACTION_SCANNING)) {
        LOGD("%s, tv not scanning ,return\n", __FUNCTION__);
        return 0;
    }

    LOGD("%s, tv scanning , stop it\n", __FUNCTION__);
    if (needSnowEffect()) {
        SetSnowShowEnable(false);
    }
    if (getScreenColorSetting()) {
        mAv.SetVideoScreenColor(VIDEO_LAYER_BLUE);
    } else {
        mAv.SetVideoScreenColor(VIDEO_LAYER_BLACK);
    }
    //CVpp::getInstance()->VPP_setVideoColor(false);
    mpTvin->Tvin_StopDecoder();
    //mTvEpg.leaveChannel();
    mTvScanner->stopScan();
    mFrontDev->Close();
    mpTvin->VDIN_ClosePort();
    m_source_input = SOURCE_INVALID;
    mTvAction &= ~TV_ACTION_SCANNING;
    LOGD("%s, OK.\n", __FUNCTION__);
    return 0;
}

int CTv::pauseScan()
{
    if (!(mTvAction & TV_ACTION_SCANNING)) {
        LOGD("%s, tv not scanning ,return\n", __FUNCTION__);
        return 0;
    }
    return mTvScanner->pauseScan();
}

int CTv::resumeScan()
{
    if (!(mTvAction & TV_ACTION_SCANNING)) {
        LOGD("%s, tv not scanning ,return\n", __FUNCTION__);
        return 0;
    }
    return mTvScanner->resumeScan();
}

int CTv::getScanStatus()
{
    int status;
    if (!(mTvAction & TV_ACTION_SCANNING)) {
        LOGD("%s, tv not scanning ,return\n", __FUNCTION__);
        return 0;
    }
    mTvScanner->getScanStatus(&status);
    return status;
}
void CTv::operateDeviceForScan(int type)
{
    LOGD("%s : type:%d\n", __FUNCTION__, type);

    CTvin::CheckSourceValidEvent evt;
    evt.source = SOURCE_TV;
    mpTvin->sendEvent(evt);

    if (type == OPEN_DEV_FOR_SCAN_ATV) {
            mFrontDev->Open(TV_FE_AUTO);
            mFrontDev->SetAnalogFrontEndTimerSwitch(1);
     }else if  (type == OPEN_DEV_FOR_SCAN_DTV) {
            mFrontDev->Open(TV_FE_AUTO);
            mFrontDev->SetAnalogFrontEndTimerSwitch(0);
     }else if  (type == CLOSE_DEV_FOR_SCAN) {
            mFrontDev->SetAnalogFrontEndTimerSwitch(0);
     }
}
void CTv::setDvbTextCoding(char *coding)
{
#ifdef SUPPORT_ADTV
    if (!strcmp(coding, "standard")) {
        AM_SI_SetDefaultDVBTextCoding("");
    } else {
        AM_SI_SetDefaultDVBTextCoding(coding);
    }
#endif
}

int CTv::playDvbcProgram ( int progId )
{
    LOGD("%s, progId = %d\n", __FUNCTION__,  progId );

    CTvProgram prog;
    int ret = CTvProgram::selectByID ( progId, prog );
    if ( ret != 0 ) {
        return -1;
    }

    LOGD("%s, prog name = %s\n", __FUNCTION__,  prog.getName().string() );
    CTvChannel channel;
    prog.getChannel ( channel );

    mFrontDev->setPara ( channel.getMode(), channel.getFrequency(), channel.getSymbolRate(), channel.getModulation() );

    int vpid = 0x1fff, apid = 0x1fff, vfmt = -1, afmt = -1;

    CTvProgram::Video *pV = prog.getVideo();
    if ( pV != NULL ) {
        vpid = pV->getPID();
        vfmt = pV->getFormat();
    }

    int aindex = prog.getCurrentAudio ( String8 ( "eng" ) );
    if ( aindex >= 0 ) {
        CTvProgram::Audio *pA = prog.getAudio ( aindex );
        if ( pA != NULL ) {
            apid = pA->getPID();
            afmt = pA->getFormat();
        }
    }
    mTvEpg.leaveProgram();
    mTvEpg.leaveChannel();
    startPlayTv ( SOURCE_DTV, vpid, apid, prog.getPcrId(), vfmt, afmt );

    mTvEpg.enterChannel ( channel.getID() );
    mTvEpg.enterProgram ( prog.getID() );
    return 0;
}

int CTv::getAudioTrackNum ( int progId )
{
    int iRet, iOutCnt = 0;
    CTvProgram prog;

    do {
        if ( 0 > progId ) {
            break;
        }

        iRet = CTvProgram::selectByID ( progId, prog );
        if ( 0 != iRet ) {
            break;
        }

        iOutCnt = prog.getAudioTrackSize();
    } while ( false );

    return iOutCnt;
}

int CTv::getAudioInfoByIndex ( int progId, int idx, int *pAFmt, String8 &lang )
{
    int iRet, iOutRet = -1;
    CTvProgram prog;
    CTvProgram::Audio *pA = NULL;

    do {
        if ( NULL == pAFmt || idx < 0 ) {
            break;
        }

        iRet = CTvProgram::selectByID ( progId, prog );
        if ( 0 != iRet ) {
            break;
        }

        if ( idx >= prog.getAudioTrackSize() ) {
            break;
        }

        pA = prog.getAudio ( idx );
        if ( NULL == pA ) {
            break;
        }

        *pAFmt = pA->getFormat();
        lang = pA->getLang();
    } while ( false );

    return iOutRet;
}

int CTv::switchAudioTrack (int aPid, int aFmt, int aParam __unused)
{
    int iOutRet = 0;
    do {
        if ( aPid < 0 || aFmt < 0 ) {
            break;
        }

        iOutRet = mAv.SwitchTSAudio (( unsigned short ) aPid, aFmt );
        LOGD ("%s, iOutRet = %d AM_AV_SwitchTSAudio\n", __FUNCTION__,  iOutRet );
    } while ( false );

    return iOutRet;
}

int CTv::switchAudioTrack ( int progId, int idx )
{
    int iOutRet = 0;
    CTvProgram prog;
    CTvProgram::Audio *pAudio = NULL;
    int aPid = -1;
    int aFmt = -1;

    do {
        if ( idx < 0 ) {
            break;
        }

        iOutRet = CTvProgram::selectByID ( progId, prog );
        if ( 0 != iOutRet ) {
            break;
        }
        /*if (prog.getAudioTrackSize() <= 1) {
            LOGD("%s, just one audio track, not to switch", __FUNCTION__);
            break;
        }*/
        if ( idx >= prog.getAudioTrackSize() ) {
            break;
        }

        pAudio = prog.getAudio ( idx );
        if ( NULL == pAudio ) {
            break;
        }

        aPid = pAudio->getPID();
        aFmt = pAudio->getFormat();
        if ( aPid < 0 || aFmt < 0 ) {
            break;
        }

        iOutRet = mAv.SwitchTSAudio (( unsigned short ) aPid, aFmt );
        LOGD ( "%s, iOutRet = %d AM_AV_SwitchTSAudio\n", __FUNCTION__,  iOutRet );
    } while ( false );

    return iOutRet;
}

int CTv::ResetAudioDecoderForPCMOutput()
{
    int iOutRet = mAv.ResetAudioDecoder ();
    LOGD ( "%s, iOutRet = %d AM_AV_ResetAudioDecoder\n", __FUNCTION__,  iOutRet );
    return iOutRet;
}

int CTv::setAudioAD (int enable, int aPid, int aFmt)
{
    int iOutRet = 0;
    if ( aPid < 0 || aFmt < 0 ) {
        return iOutRet;
    }

    iOutRet = mAv.SetADAudio (enable, ( unsigned short ) aPid, aFmt );
    LOGD ("%s, iOutRet = %d (%d,%d,%d)\n", __FUNCTION__,  iOutRet, enable, aPid, aFmt );

    return iOutRet;
}

int CTv::playDtvProgramUnlocked (const char *feparas, int mode, int freq, int para1, int para2,
                          int vpid, int vfmt, int apid, int afmt, int pcr, int audioCompetition __unused)
{
    int ret = 0;
    int FeMode = 0;
    const char nullparas = 0;
    LOGD("[%s] Start SwitchSourceTime = %fs", __FUNCTION__, getUptimeSeconds());

    if (feparas == NULL) {
        LOGD("feparas is null,set as 0", __FUNCTION__);
        feparas = &nullparas;
    }

    CFrontEnd::FEParas fp(feparas);

    SetSourceSwitchInputLocked(m_source_input_virtual, SOURCE_DTV);

    FeMode = fp.getFEMode().getBase();
    LOGD("[%s] FeMode = %d", __FUNCTION__, FeMode);
    mFrontDev->Open(FeMode);
    if (!(mTvAction & TV_ACTION_SCANNING)) {
        if (!feparas) {
            if ( SOURCE_ADTV == m_source_input_virtual ) {
                mFrontDev->setPara (FeMode, freq, para1, para2);
            } else {
                mFrontDev->setPara (mode, freq, para1, para2);
            }
        } else {
            mFrontDev->setPara(feparas);
        }
    }
    mTvAction |= TV_ACTION_PLAYING;
    ret = startPlayTv ( SOURCE_DTV, vpid, apid, pcr, vfmt, afmt );
/*No need check, FE Will report status.*/
    CMessage msg;
    msg.mDelayMs = 2000;
    msg.mType = CTvMsgQueue::TV_MSG_CHECK_FE_DELAY;
    mTvMsgQueue->sendMsg ( msg );
/**/
    LOGD("[%s] End SwitchSourceTime = %fs", __FUNCTION__, getUptimeSeconds());
    return 0;
}

int CTv::playDtvProgram (const char *feparas, int mode, int freq, int para1, int para2,
                          int vpid, int vfmt, int apid, int afmt, int pcr, int audioCompetition)
{
    AutoMutex _l( mLock );
    return playDtvProgramUnlocked(feparas, mode, freq, para1, para2,
                            vpid, vfmt, apid, afmt, pcr, audioCompetition);
}

int CTv::playDtvProgramUnlocked(int mode, int freq, int para1, int para2, int vpid, int vfmt, int apid,
        int afmt, int pcr, int audioCompetition) {
    return playDtvProgramUnlocked(NULL, mode, freq, para1, para2, vpid, vfmt, apid, afmt, pcr, audioCompetition);
}
int CTv::playDtvProgram(int mode, int freq, int para1, int para2, int vpid, int vfmt, int apid,
        int afmt, int pcr, int audioCompetition) {
    return playDtvProgram(NULL, mode, freq, para1, para2, vpid, vfmt, apid, afmt, pcr, audioCompetition);
}
int CTv::playDtvProgramUnlocked(const char* feparas, int vpid, int vfmt, int apid,
        int afmt, int pcr, int audioCompetition) {
    return playDtvProgramUnlocked(feparas, 0, 0, 0, 0, vpid, vfmt, apid, afmt, pcr, audioCompetition);
}
int CTv::playDtvProgram(const char* feparas, int vpid, int vfmt, int apid,
        int afmt, int pcr, int audioCompetition) {
    return playDtvProgram(feparas, 0, 0, 0, 0, vpid, vfmt, apid, afmt, pcr, audioCompetition);
}


int CTv::playDtvTimeShiftUnlocked (const char *feparas, void *para, int audioCompetition __unused)
{
    int ret = 0;
    int FeMode = 0;
    if (feparas) {
        SetSourceSwitchInputLocked(m_source_input_virtual, SOURCE_DTV);
        CFrontEnd::FEParas fp(feparas);
        FeMode = fp.getFEMode().getBase();
        LOGD("[%s] FeMode = %d", __FUNCTION__, FeMode);
        mFrontDev->Open(FeMode);
        if (!(mTvAction & TV_ACTION_SCANNING)) {
            mFrontDev->setPara(feparas);
        }
    }

    mTvAction |= TV_ACTION_PLAYING;
    tvWriteSysfs ( DEVICE_CLASS_TSYNC_AV_THRESHOLD_MIN, AV_THRESHOLD_MIN_MS );
    ret = mAv.startTimeShift(para);

    return ret;
}


int CTv::playDtvTimeShift (const char *feparas, void *para, int audioCompetition)
{
    AutoMutex _l( mLock );
    return playDtvTimeShiftUnlocked(feparas, para, audioCompetition);
}

int CTv::playDtmbProgram ( int progId )
{
    CTvProgram prog;
    CTvChannel channel;
    int vpid = 0x1fff, apid = 0x1fff, vfmt = -1, afmt = -1;
    int aindex;
    CTvProgram::Audio *pA;
    CTvProgram::Video *pV;
    CTvProgram::selectByID ( progId, prog );
    saveDTVProgramID ( progId );
    prog.getChannel ( channel );
    int chanVolCompValue = 0;
    chanVolCompValue = GetAudioVolumeCompensationVal(progId);

    mFrontDev->setPara ( channel.getMode(), channel.getFrequency(), channel.getBandwidth(), 0 );

    pV = prog.getVideo();
    if ( pV != NULL ) {
        vpid = pV->getPID();
        vfmt = pV->getFormat();
    }

    aindex = prog.getCurrAudioTrackIndex();
    if ( -1 == aindex ) { //db is default
        aindex = prog.getCurrentAudio ( String8 ( "eng" ) );
        if ( aindex >= 0 ) {
            prog.setCurrAudioTrackIndex ( progId, aindex );
        }
    }

    if ( aindex >= 0 ) {
        pA = prog.getAudio ( aindex );
        if ( pA != NULL ) {
            apid = pA->getPID();
            afmt = pA->getFormat();
        }
    }

    mTvEpg.leaveProgram();
    mTvEpg.leaveChannel();
    startPlayTv ( SOURCE_DTV, vpid, apid, prog.getPcrId(), vfmt, afmt );
    mTvEpg.enterChannel ( channel.getID() );
    mTvEpg.enterProgram ( prog.getID() );
    return 0;
}

int CTv::playAtvProgram (int freq, int videoStd, int audioStd, int vfmt, int soundsys, int fineTune __unused, int audioCompetition __unused)
{

    if (mTvStatus >= TV_STOP_ED) {
        LOGD("%s: atv source is stoped! return\n", __FUNCTION__);
        return 0;
    }
    LOGD("%s Start SwitchSourceTime = %fs",__FUNCTION__,getUptimeSeconds());
    SetSourceSwitchInputLocked(m_source_input_virtual, SOURCE_TV);
    mTvAction |= TV_ACTION_IN_VDIN;
    mTvAction |= TV_ACTION_PLAYING;

    /* mpTvin->Tvin_StopDecoder(); */ /* No, it will affect the handling of onVdinSignalChange */
    mFrontDev->Open(TV_FE_ANALOG);
    unsigned long stdAndColor = mFrontDev->enumToStdAndColor (videoStd, audioStd);

    //set CVBS
    int fmt = CFrontEnd::stdEnumToCvbsFmt (vfmt, stdAndColor);
    mpTvin->AFE_SetCVBSStd ( ( tvin_sig_fmt_t ) fmt );

    //set TUNER
    mFrontDev->setPara (TV_FE_ANALOG, freq, stdAndColor, -1, vfmt, soundsys);
    LOGD("%s End SwitchSourceTime = %fs",__FUNCTION__,getUptimeSeconds());
    return 0;
}

int CTv::resetFrontEndPara ( frontend_para_set_t feParms )
{
    LOGD("mTvAction = %#x, %s", mTvAction, __FUNCTION__);
#ifdef SUPPORT_ADTV
    if (mTvAction & TV_ACTION_SCANNING) {
        return -1;
    }

    if ( feParms.mode == TV_FE_ANALOG ) {
        //int progID = -1;
        int tmpFreq = feParms.freq;
        int tmpfineFreq = feParms.para2;
        //int mode = feParms.mode;

        //get tunerStd from videoStd and audioStd
        unsigned long stdAndColor = mFrontDev->enumToStdAndColor (feParms.videoStd, feParms.audioStd);

        LOGD("%s, resetFrontEndPara- vstd=%d astd=%d stdAndColor=0x%x", __FUNCTION__, feParms.videoStd, feParms.audioStd, stdAndColor);

        //set frontend parameters to tuner dev
        mpTvin->Tvin_StopDecoder();

        //set CVBS
        int fmt = CFrontEnd::stdEnumToCvbsFmt (feParms.vfmt, stdAndColor);
        mpTvin->AFE_SetCVBSStd ( ( tvin_sig_fmt_t ) fmt );

        //set TUNER
        //usleep(400 * 1000);
        mFrontDev->Open(TV_FE_AUTO);
        mFrontDev->setPara ( TV_FE_ANALOG, tmpFreq, stdAndColor, -1, feParms.vfmt);
        //usleep(400 * 1000);
        if ( tmpfineFreq != 0 ) {
            mFrontDev->fineTune ( tmpfineFreq / 1000 );
        }
    } else {
        mFrontDev->Open(TV_FE_AUTO);
        mFrontDev->setPara ( feParms.mode, feParms.freq, feParms.para1, feParms.para2 );
    }
#endif
    return 0;
}

int CTv::setFrontEnd ( const char *paras, bool force )
{
    AutoMutex _l( mLock );
    LOGD("%s: mTvAction = %#x, paras = %s, m_source_input = %d. SwitchSourceTime = %fs",
            __FUNCTION__, mTvAction, paras, m_source_input,getUptimeSeconds());

    if (mTvAction & TV_ACTION_SCANNING) {
        return -1;
    }
    LOGD("%s: source_input_virtual[%d], mDTvSigStaus[%d], mNoneStaticChange[%d]",__FUNCTION__,
        m_source_input_virtual, mDTvSigStaus, mNoneStaticChange);
    if (SOURCE_ADTV == m_source_input_virtual && getScreenStaticFrameEnable() && mDTvSigStaus
        && !isChannelBlockStatusChanged() && !mNoneStaticChange) {
        LOGD("%s: current source[%d], set static frame",__FUNCTION__,m_source_input);
        mAv.DisableVideoBlackout();
    } else {
        LOGD("%s: current source[%d], needn't static frame",__FUNCTION__,m_source_input);
        mAv.EnableVideoBlackout();
        //if change channel want display blue color,use this
        /*if (SOURCE_ADTV == m_source_input_virtual && (mBlockStatusChanged || BLOCK_STATE_BLOCKED == mChannelBlockState)) {
            ScreenColorControl(false, VIDEO_LAYER_COLOR_SHOW_ALWAYES);
        } else {
            ScreenColorControl(true, VIDEO_LAYER_COLOR_SHOW_ALWAYES);
        }*/
        //default set change channel is black
        ScreenColorControl(false, VIDEO_LAYER_COLOR_SHOW_ALWAYES);
    }
    mChannelLastBlockState = mChannelBlockState;
    stopPlaying(false);
    CFrontEnd::FEParas fp(paras);
    int FEMode_Base = fp.getFEMode().getBase();
    LOGD("%s: fp.getFEMode.getbase = %d", __FUNCTION__, FEMode_Base);
    if ( FEMode_Base == TV_FE_ANALOG ) {
        mTvAction |= TV_ACTION_IN_VDIN;
        //int tmpFreq = fp.getFrequency();
        int tmpfineFreq = fp.getFrequency2();

        //get tunerStd from videoStd and audioStd
        unsigned long stdAndColor = mFrontDev->enumToStdAndColor (fp.getVideoStd(), fp.getAudioStd());

        LOGD("%s: vstd=%d astd=%d stdAndColor=0x%x", __FUNCTION__, fp.getVideoStd(), fp.getAudioStd(), stdAndColor);
        //set frontend parameters to tuner dev
        if (needSnowEffect() && mpTvin->getSnowStatus()) {
            SetSnowShowEnable( false );
        }
        mpTvin->Tvin_StopDecoder();
        //set CVBS
        int fmt = CFrontEnd::stdEnumToCvbsFmt (fp.getVFmt(), stdAndColor);
        mpTvin->AFE_SetCVBSStd ( ( tvin_sig_fmt_t ) fmt );

        //set TUNER
        //usleep(400 * 1000);
        mFrontDev->Open(TV_FE_AUTO);
        mFrontDev->setMode(TV_FE_ANALOG);
        mFrontDev->setPara ( paras, force );
        //usleep(400 * 1000);
        if ( tmpfineFreq != 0 ) {
            mFrontDev->fineTune ( tmpfineFreq / 1000, force );
        }
    } else {
        mTvAction &= ~TV_ACTION_IN_VDIN;
        if (needSnowEffect() && mpTvin->getSnowStatus()) {
            SetSnowShowEnable( false );
        }
        mpTvin->Tvin_StopDecoder();
        mpTvin->VDIN_ClosePort();

        mFrontDev->Open(TV_FE_AUTO);
        mFrontDev->setMode(FEMode_Base);
        mFrontDev->setPara (paras, force);
    }

    int mode, freq, para1, para2;
    mFrontDev->getPara(&mode, &freq, &para1, &para2);
    mode = (FEMode_Base == TV_FE_ANALOG);
    Tv_RrtUpdate();
    Tv_Easupdate();
    LOGD("%s setFrontEnd End SwitchSourceTime = %fs",__FUNCTION__,getUptimeSeconds());
    return 0;
}

int CTv::resetDmxAndAvSource()
{
#ifdef SUPPORT_ADTV
    AM_DMX_Source_t curdmxSource;
    mFrontDev->GetTSSource ( &curdmxSource );
    LOGD ( "%s, AM_FEND_GetTSSource %d", __FUNCTION__, curdmxSource );
    mTvDmx.SetSource(curdmxSource );
    mTvDmx1.SetSource(curdmxSource);
    mTvDmx2.SetSource(curdmxSource);
    int ts_source = ( int ) curdmxSource;
    mAv.SetTSSource (ts_source );
#endif
    return 0;
}

int CTv::GetAudioVolumeCompensationVal(int progxxId __unused)
{
    int tmpVolValue = 0;
    CTvProgram prog;
    if ( CTvProgram::selectByID ( -1, prog ) != 0 ) {
        LOGE ( "%s, atv progID is not matching the db's  ret = 0\n", __FUNCTION__ );
        return tmpVolValue;
    }
    tmpVolValue = prog.getChanVolume ();
    LOGD ( "%s,progid = -1 CompensationVal = %d\n", __FUNCTION__, tmpVolValue );
    if (tmpVolValue > 20 || tmpVolValue < -20) tmpVolValue = 0;
    return tmpVolValue;
}

int CTv::startPlayTv ( int source, int vid, int aid, int pcrid, int vfat, int afat )
{
    LOGD ("%s\n", __FUNCTION__);
    if ( source == SOURCE_DTV ) {
        tvWriteSysfs ( DEVICE_CLASS_TSYNC_AV_THRESHOLD_MIN, AV_THRESHOLD_MIN_MS );
        setDvbLogLevel();
        LOGD ( "%s, start Play DTV SwitchSourceTime Time = %fs!\n", __FUNCTION__,getUptimeSeconds());
        return mAv.StartTS (vid, aid, pcrid, vfat, afat );
    }
    return -1;
}

int CTv::stopPlayingLock()
{
    AutoMutex _l( mLock );
    if (mSubtitle.sub_switch_status() == 1)
        mSubtitle.sub_stop_dvb_sub();
    return stopPlaying(true);
}

int CTv::stopPlaying(bool isShowTestScreen) {
    return stopPlaying(isShowTestScreen, true);
}

int CTv::stopPlaying(bool isShowTestScreen __unused, bool resetFE)
{
    LOGD("%s", __FUNCTION__);
    if (!(mTvAction & TV_ACTION_PLAYING)) {
        LOGD("%s, stopplay cur action = %x not playing , return", __FUNCTION__, mTvAction);
        return 0;
    }

    LOGD("%s, resetFE = %d", __FUNCTION__, resetFE);
    if (m_source_input == SOURCE_TV) {
        if (resetFE)
            ClearAnalogFrontEnd();
    } else if (m_source_input ==  SOURCE_DTV) {
        mAv.StopTS ();
    }

    mTvAction &= ~TV_ACTION_PLAYING;
    return 0;
}

int CTv::getAudioChannel()
{
    int iRet = -1;
    int audioChannelMod;
    do {
        iRet = mAv.AudioGetOutputMode (&audioChannelMod );
        if ( DVB_SUCCESS != iRet ) {
            LOGD ( "%s, audio GetOutputMode is FAILED %d\n", __FUNCTION__,  iRet );
            break;
        }
        LOGD ( "%s, getAudioChannel iRet : %d audioChannelMod %d\n", __FUNCTION__,  iRet, audioChannelMod );
    } while ( 0 );
    return audioChannelMod;
}

int CTv::setAudioChannel ( int channelIdx )
{
    int iOutRet = 0;
    int audioChannelMod;
    LOGD ( "%s, channelIdx : %d\n", __FUNCTION__,  channelIdx );
    audioChannelMod = ( int ) channelIdx;
    iOutRet = mAv.AudioSetOutputMode (audioChannelMod );
    int aud_ch = 1;
    if (audioChannelMod == TV_AOUT_OUTPUT_STEREO) {
        aud_ch = 1;
    } else if (audioChannelMod == TV_AOUT_OUTPUT_DUAL_LEFT) {
        aud_ch = 2;
    } else if (audioChannelMod == TV_AOUT_OUTPUT_DUAL_RIGHT) {
        aud_ch = 3;
    }
    CTvProgram::updateAudioChannel(-1, aud_ch);
    if ( DVB_SUCCESS != iOutRet) {
        LOGD ( "%s, TV AM_AOUT_SetOutputMode device is FAILED %d\n", __FUNCTION__, iOutRet );
    }
    return 0;
}

int CTv::getFrontendSignalStrength()
{
    return mFrontDev->getStrength();
}

int CTv::getFrontendSNR()
{
    return mFrontDev->getSNR();
}

int CTv::getFrontendBER()
{
    return mFrontDev->getBER();
}

int CTv::getChannelInfoBydbID ( int dbID, channel_info_t &chan_info )
{
    CTvProgram prog;
    CTvChannel channel;
    Vector<sp<CTvProgram> > out;
    memset ( &chan_info, 0, sizeof(chan_info));
    chan_info.freq = 44250000;
    chan_info.uInfo.atvChanInfo.videoStd = CC_ATV_VIDEO_STD_PAL;
    chan_info.uInfo.atvChanInfo.audioStd = CC_ATV_AUDIO_STD_DK;
    static const int CVBS_ID = -1;
    if (dbID == CVBS_ID) {
        unsigned char std;
        SSMReadCVBSStd(&std);
        chan_info.uInfo.atvChanInfo.videoStd = (atv_video_std_t)std;
        chan_info.uInfo.atvChanInfo.audioStd = CC_ATV_AUDIO_STD_DK;
        chan_info.uInfo.atvChanInfo.isAutoStd = (std == CC_ATV_VIDEO_STD_AUTO) ? 1 : 0;
        return 0;
    }

    int ret = CTvProgram::selectByID ( dbID, prog );
    if ( ret != 0 ) {
        LOGD ( "getChannelinfo , select dbid=%d,is not exist", dbID);
        return -1;
    }
    prog.getChannel ( channel );

    if ( CTvProgram::TYPE_ATV == prog.getProgType() ) {
        chan_info.freq = channel.getFrequency();
        chan_info.uInfo.atvChanInfo.finefreq  =  channel.getAfcData();
        LOGD("%s, get channel std = %d", __FUNCTION__, channel.getStd());
        chan_info.uInfo.atvChanInfo.videoStd =
            ( atv_video_std_t ) CFrontEnd::stdAndColorToVideoEnum ( channel.getStd() );
        chan_info.uInfo.atvChanInfo.audioStd =
            ( atv_audio_std_t ) CFrontEnd::stdAndColorToAudioEnum ( channel.getStd() );
        chan_info.uInfo.atvChanInfo.isAutoStd = ((channel.getStd() & V4L2_COLOR_STD_AUTO) ==  V4L2_COLOR_STD_AUTO) ? 1 : 0;
    } else if ( CTvProgram::TYPE_DTV == prog.getProgType()  || CTvProgram::TYPE_RADIO == prog.getProgType()) {
        chan_info.freq = channel.getFrequency();
        chan_info.uInfo.dtvChanInfo.strength = getFrontendSignalStrength();
        chan_info.uInfo.dtvChanInfo.quality = getFrontendSNR();
        chan_info.uInfo.dtvChanInfo.ber = getFrontendBER();
    }

    return 0;
}

bool CTv::Tv_Start_Analyze_Ts ( int channelID )
{
    int freq, ret;
    CTvChannel channel;

    AutoMutex _l( mLock );
    mAv.StopTS ();
    CVpp::getInstance()->VPP_setVideoColor(true);
    ret = CTvChannel::selectByID ( channelID, channel );
    if ( ret != 0 ) {
        LOGD ( "%s, CTv tv_update ts can not get freq by channel ID", __FUNCTION__ );
        return false;
    }

    mTvAction |= TV_ACTION_SCANNING;
    freq = channel.getFrequency();
    LOGD ( "%s, the freq = %d", __FUNCTION__,  freq );
    mDtvScanRunningStatus = DTV_SCAN_RUNNING_ANALYZE_CHANNEL;
    mTvScanner->setObserver ( mTvMsgQueue.get() );
    mTvScanner->manualDtmbScan ( freq, freq ); //dtmb
    return true;
}

bool CTv::Tv_Stop_Analyze_Ts()
{
    stopScanLock();
    return true;
}

int CTv::saveATVProgramID ( int dbID )
{
    config_set_int ( CFG_SECTION_TV, "atv.get.program.id", dbID );
    return 0;
}

int CTv::getATVProgramID ( void )
{
    return config_get_int ( CFG_SECTION_TV, "atv.get.program.id", -1 );
}

int CTv::saveDTVProgramID ( int dbID )
{
    config_set_int ( CFG_SECTION_TV, "dtv.get.program.id", dbID );
    return 0;
}

int CTv::getDTVProgramID ( void )
{
    return config_get_int ( CFG_SECTION_TV, "dtv.get.program.id", -1 );
}

int CTv::saveRadioProgramID ( int dbID )
{
    config_set_int ( CFG_SECTION_TV, "radio.get.program.id", dbID );
    return 0;
}

int CTv::getRadioProgramID ( void )
{
    return config_get_int ( CFG_SECTION_TV, "radio.get.program.id", -1 );
}


int CTv::getATVMinMaxFreq ( int *scanMinFreq, int *scanMaxFreq )
{
    const char *strDelimit = ",";
    char *token = NULL;

    *scanMinFreq = 44250000;
    *scanMaxFreq = 868250000;

    const char *freqList = config_get_str ( CFG_SECTION_ATV, CFG_ATV_FREQ_LIST, "null" );
    if ( strcmp ( freqList, "null" ) == 0 ) {
        LOGE( "[ctv]%s, atv freq list is null \n", __FUNCTION__ );
        return -1;
    }

    char data_str[512] = {0};
    strncpy ( data_str, freqList, sizeof ( data_str ) - 1 );

    char *pSave;
    token = strtok_r ( data_str, strDelimit, &pSave);
    if (token != NULL)
    {
        sscanf ( token, "%d", scanMinFreq );
    }
    token = strtok_r ( NULL, strDelimit , &pSave);
    if ( token != NULL ) {
        sscanf ( token, "%d", scanMaxFreq );
    }
    return 0;
}

int CTv::IsDVISignal()
{
    return m_cur_sig_info.is_dvi;
}

int CTv::getHDMIFrameRate()
{
    int ConstRate[5] = {24, 25, 30, 50, 60};
    float ConstRateDiffHz[5] = {0.5, 0.5, 0.5, 2, 2};
    int fps = m_cur_sig_info.fps;
    for (int i = 0; i < 5; i++) {
        if (abs(ConstRate[i] - fps) < ConstRateDiffHz[i])
            fps = ConstRate[i];
    }
    return fps;
}

int CTv::getHdmiFormatInfo ( int *pWidth, int *pHeight, int *pFPS)
{
    if (m_cur_sig_info.status == TVIN_SIG_STATUS_STABLE) {
        tvin_format_t hdmiFormatInfo;
        memset(&hdmiFormatInfo, 0, sizeof(tvin_format_t));
        mpTvin->VDIN_GetHdmiFormatInfo(&hdmiFormatInfo);
        *pWidth = hdmiFormatInfo.h_active;
        if (hdmiFormatInfo.scan_mode == TVIN_SCAN_MODE_INTERLACED) {
            *pHeight = hdmiFormatInfo.v_active * 2;
        } else {
            *pHeight = hdmiFormatInfo.v_active;
        }
        *pFPS = getHDMIFrameRate();
    } else {
        //LOGD("%s: signal don't stable!\n");
        LOGD("%s: signal don't stable!\n", __FUNCTION__);
    }

    return 0;
}

int CTv::getVrrMode (void)
{
    int tmp = 0;
    if (m_cur_sig_info.status == TVIN_SIG_STATUS_STABLE) {
        vdin_vrr_freesync_param_t vrrParm;
        memset(&vrrParm, 0, sizeof(vdin_vrr_freesync_param_t));
        mpTvin->VDIN_GetVrrFreesyncParm(&vrrParm);
        tmp = (int)vrrParm.cur_vrr_status;
    } else {
        LOGD("%s: signal don't stable!\n", __FUNCTION__);
    }
    LOGD("%s: vrr mode:%d!\n", __FUNCTION__, tmp);
    return tmp;
}

int CTv::getAllmInfo (void) {
    return mAllmInfo;
}

tv_source_input_t CTv::GetLastSourceInput ( void )
{
    return m_last_source_input;
}

int CTv::isVgaFmtInHdmi ( void )
{
    if ( CTvin::Tvin_SourceInputToSourceInputType(m_source_input) != SOURCE_TYPE_HDMI ) {
        return -1;
    }
    return CTvin::isVgaFmtInHdmi (m_cur_sig_info.fmt);
}

void CTv::print_version_info ( void )
{
    // print tvapi version info
    LOGD ( "libtvservice git branch:%s\n", tvservice_get_git_branch_info() );
    LOGD ( "libtvservice git version:%s\n",  tvservice_get_git_version_info() );
    LOGD ( "libtvservice Last Changed:%s\n", tvservice_get_last_chaned_time_info() );
    LOGD ( "libtvservice Last Build:%s\n",  tvservice_get_build_time_info() );
    LOGD ( "libtvservice Builder Name:%s\n", tvservice_get_build_name_info() );
    LOGD ( "libtvservice board version:%s\n", tvservice_get_board_version_info() );
    LOGD ( "linux kernel version:%s\n", tvservice_get_kernel_version_info());
    LOGD ( "\n\n");
#ifdef SUPPORT_ADTV
    // print dvb version info
    LOGD ( "libdvb git branch:%s\n", dvb_get_git_branch_info() );
    LOGD ( "libdvb git version:%s\n", dvb_get_git_version_info() );
    LOGD ( "libdvb Last Changed:%s\n", dvb_get_last_chaned_time_info() );
    LOGD ( "libdvb Last Build:%s\n", dvb_get_build_time_info() );
    LOGD ( "libdvb Builder Name:%s\n", dvb_get_build_name_info() );
#endif
}

int CTv::Tvin_GetTvinConfig ( void )
{
    const char *config_value;

    config_value = config_get_str ( CFG_SECTION_TV, CFG_TVIN_KERNELPET_DISABLE, "null" );
    if ( strcmp ( config_value, "disable" ) == 0 ) {
        gTvinConfig.kernelpet_disable = true;
    } else {
        gTvinConfig.kernelpet_disable = false;
    }

    config_value = config_get_str ( CFG_SECTION_TV, CFG_TVIN_KERNELPET_TIMEROUT, "null" );
    gTvinConfig.userpet_timeout = ( unsigned int ) strtol ( config_value, NULL, 10 );

    if ( gTvinConfig.kernelpet_timeout <= 0 || gTvinConfig.kernelpet_timeout > 40 ) {
        gTvinConfig.kernelpet_timeout = 10;
    }

    config_value = config_get_str ( CFG_SECTION_TV, CFG_TVIN_USERPET, "null" );
    if ( strcmp ( config_value, "enable" ) == 0 ) {
        gTvinConfig.userpet = true;
    } else {
        gTvinConfig.userpet = false;
    }

    config_value = config_get_str ( CFG_SECTION_TV, CFG_TVIN_USERPET_TIMEROUT, "null" );
    gTvinConfig.userpet_timeout = ( unsigned int ) strtol ( config_value, NULL, 10 );

    if ( gTvinConfig.userpet_timeout <= 0 || gTvinConfig.userpet_timeout > 100 ) {
        gTvinConfig.userpet_timeout = 10;
    }

    config_value = config_get_str ( CFG_SECTION_TV, CFG_TVIN_USERPET_RESET, "null" );
    if ( strcmp ( config_value, "disable" ) == 0 ) {
        gTvinConfig.userpet_reset = 0;
    } else {
        gTvinConfig.userpet_reset = 1;
    }
    return 0;
}

TvRunStatus_t CTv::GetTvStatus()
{
    AutoMutex _l( mLock );
    return mTvStatus;
}

int CTv::GetTvAction() {
    return mTvAction;
}

int CTv::OpenTv ( void )
{
    LOGD("%s: start at %fs\n", __FUNCTION__, getUptimeSeconds());
    const char * value;
    value = config_get_str ( CFG_SECTION_TV, CFG_TVIN_ATV_DISPLAY_SNOW, "null" );
    if (strcmp(value, "enable") == 0 ) {
        mATVDisplaySnow = true;
    } else {
        mATVDisplaySnow = false;
    }
    mpTvin->AFE_EnableSnowByConfig(mATVDisplaySnow);
    value = config_get_str ( CFG_SECTION_TV, CGF_TV_AUTOSWITCH_ALLM_EN, "enable" );
    if (strcmp(value, "disable") == 0 ) {
        mAllmModeCfg = false;
    } else {
        mAllmModeCfg = true;
    }

    value = config_get_str ( CFG_SECTION_TV, CGF_TV_AUTOSWITCH_ITCONTENT_EN, "disable" );
    if (strcmp(value, "enable") == 0 ) {
        mItcontentModeCfg = true;
    } else {
        mItcontentModeCfg = false;
    }

    value = config_get_str ( CFG_SECTION_TV, CGF_TV_AUTOSWITCH_DVI_EN, "disable" );
    if (strcmp(value, "enable") == 0 ) {
        mDviModeCfg = true;
    } else {
        mDviModeCfg = false;
    }

    /*value = config_get_str ( CFG_SECTION_TV, CFG_FBC_PANEL_INFO, "null" );
    LOGD("open tv, get fbc panel info:%s\n", value);
    if (strcmp(value, "edid") == 0 ) {
        rebootSystemByEdidInfo();
    } else if (strcmp(value, "uart") == 0 ) {
        rebootSystemByUartPanelInfo(fbcIns);
    }*/

    //tv ssm check
    SSMHandlePreCopying();

    mpTvin->OpenTvin();

    CVpp::getInstance()->Vpp_Init();

    SSMSetHDCPKey();
    system ( "/vendor/bin/dec" );

    value = config_get_str ( CFG_SECTION_TV, CFG_TVIN_DISPLAY_FREQ_AUTO, "null" );
    if ( strcmp ( value, "enable" ) == 0 ) {
        mAutoSetDisplayFreq = true;
    } else {
        mAutoSetDisplayFreq = false;
    }

    int isLoadEdidWithPort = config_get_int(CFG_SECTION_HDMI, TV_CONFIG_LOAD_EDID_WITH_PORT_EN, 1);
    if (getKernelMajorVersion() == 5 && getKernelMinorVersion() == 1 && isLoadEdidWithPort) {
        LOGD("%s: Load Edid With Port!\n", __FUNCTION__);
        mIsLoadEdidWithPort = true;
    } else {
        mIsLoadEdidWithPort = false;
    }

    //LOAD EDID
    mHDMIRxManager.SetHdmiPortCecPhysicAddr();
    int edidAutoLoadEnable = config_get_int(CFG_SECTION_HDMI, CFG_HDMI_EDID_AUTO_LOAD_EN, 1);
    if (edidAutoLoadEnable == 1) {
        LOGD("%s: EDID data load by tvserver!\n", __FUNCTION__);
        mInitEdidVersion = true;
        if ( 0 == SupportDlg() ) {//support DLG func
            LOGD("%s: support DLG!\n", __FUNCTION__);
            int8_t dlgEnable = 0;
            SSMReadDLGEnable(&dlgEnable);
            Tv_SetDLGEnable(dlgEnable);
        } else {//unsupport DLG func
            int amdvEnableState = GetAmdvEnable();
            LoadEdidData(0, amdvEnableState, 0);
            SSMSaveDLGEnable(0);
        }
    } else {
        LOGD("%s: EDID data load by customer!\n", __FUNCTION__);
    }

    //add tvpath
    if (mpTvin->Tvin_RemovePath (TV_PATH_TYPE_TVIN) > 0) {
        mpTvin->VDIN_AddVideoPath(TV_PATH_VDIN_AMLVIDEO2_PPMGR_DEINTERLACE_AMVIDEO);
    }
    /*if (IsSupportPIP()) {
        if (mpTvin->Tvin_RemovePath (TV_PATH_TYPE_TVIN2) > 0) {
            mpTvin->VDIN_AddVideoPath(TV_PATH_VDIN2_DEINTERLACE1_VIDEOQUEUE2);
        }
    }*/
    if (mpTvin->Tvin_RemovePath (TV_PATH_TYPE_DEFAULT) > 0) {
            mpTvin->VDIN_AddVideoPath(TV_PATH_DECODER_AMLVIDEO2_PPMGR_DEINTERLACE_AMVIDEO);
    }

    mEnableLockModule = (SSMReadChannelLockEnValue() == 0);
    mSupportChannelLock = (config_get_int(CFG_SECTION_TV, CFG_TV_CHANNEL_BLOCK_INSERVER, 0) > 0);
    mBlockState = BLOCK_STATE_NONE;
    mChannelBlockState = BLOCK_STATE_NONE;
    mChannelLastBlockState = BLOCK_STATE_NONE;
    mBlockStatusChanged = false;
    mNoneStaticChange = false;

    Tvin_GetTvinConfig();
    m_last_source_input = SOURCE_INVALID;
    m_source_input = SOURCE_INVALID;

    mAv.Open();
    //mDevicesPollStatusDetectThread.startDetect();
    //ClearAnalogFrontEnd();

    //for HDMI source connect detect
       mpTvin->VDIN_OpenHDMIPinMuxOn(true);

    InitCurrentSignalInfo();
    InitCurrentVdin2SignalInfo();
    mTvStatus = TV_OPEN_ED;
    return 0;
}

int CTv::CloseTv ( void )
{
    mpTvin->Tv_uninit_afe();
    mpTvin->uninit_vdin();
    TvMisc_DisableWDT ( gTvinConfig.userpet );
    mTvStatus = TV_CLOSE_ED;
    return 0;
}

int CTv::StartTvLock ()
{
    LOGD("StartTvLock status = %d", mTvStatus);
    if (mTvStatus == TV_START_ED)
        return 0;

    AutoMutex _l( mLock );
    //tvWriteSysfs("/sys/power/wake_lock", "tvserver.run");

    setDvbLogLevel();
    ScreenColorControl(false,VIDEO_LAYER_COLOR_SHOW_ALWAYES);
    //TvMisc_EnableWDT ( gTvinConfig.kernelpet_disable, gTvinConfig.userpet, gTvinConfig.kernelpet_timeout, gTvinConfig.userpet_timeout, gTvinConfig.userpet_reset );
    //mpTvin->TvinApi_SetCompPhaseEnable ( 1 );
    //mpTvin->VDIN_EnableRDMA ( 1 );

    mTvStatus = TV_START_ED;
    mTvAction &= ~TV_ACTION_STOPING;
    MnoNeedAutoSwitchToMonitorMode = false;
    mDTvSigStaus = true;
    mVRRStatusChange = false;
    mChannelBlockState = BLOCK_STATE_NONE;
    mChannelLastBlockState = BLOCK_STATE_NONE;
    mBlockStatusChanged = true;
    mNoneStaticChange = false;
    mCurrentProgramIsScambled = false;
    LOGD("StartTvLock end");
    return 0;
}

int CTv::startTvDetect()
{
    mDevicesPollStatusDetectThread->startDetect();
    return 0;
}

int CTv::DoSuspend(int type __unused)
{
    return 0;
}

int CTv::DoResume(int type __unused)
{
    return 0;
}

int CTv::StopTvLock ( void )
{
    LOGD("%s: mTvStatus = %d, SameSourceEnableStatus = %d\n", __FUNCTION__, mTvStatus, mbSameSourceEnableStatus);
    if (mbSameSourceEnableStatus) {
        mbSameSourceEnableStatus = false;
        return 0;
    } else {
        LOGD("%s StopTvLock Start SwitchSourceTime Time = %fs\n", __FUNCTION__,getUptimeSeconds());
        AutoMutex _l( mLock );
        mTvAction |= TV_ACTION_STOPING;
        mTvAction &= ~TV_ACTION_IN_VDIN;

        /* release ATV/DTV early */
        mFrontDev->setMode(TV_FE_AUTO);
        tryReleasePlayer(false, m_source_input);
        stopPlaying(false);

        //stop scan  if scanning
        stopScan();
        //close snow for atv
        if (needSnowEffect()) {
            SetSnowShowEnable( false );
        }

        tv_source_input_type_t current_source_type = CTvin::Tvin_SourceInputToSourceInputType(m_source_input);
        LOGD("%s current finish source type [%d]\n",__FUNCTION__,current_source_type);

        //vt interface has delay,so use disable video layer instead of vt.
        mAv.SetVideoLayerStatus(DISABLE_VIDEO_LAYER);
        mAv.SetVideoScreenColor(VIDEO_LAYER_BLACK);
        mpTvin->Tvin_StopDecoder();
        mpTvin->VDIN_ClosePort();
        //mFrontDev->SetAnalogFrontEndTimerSwitch(0);

        setAudioChannel(TV_AOUT_OUTPUT_STEREO);
        mpTvin->setMpeg2Vdin(0);
        mAv.setLookupPtsForDtmb(0);
        m_last_source_input = SOURCE_INVALID;
        m_source_input = SOURCE_INVALID;
        m_source_input_virtual = SOURCE_INVALID;
        int ret = tvSetCurrentSourceInfo(SOURCE_MPEG, TVIN_SIG_FMT_NULL, TVIN_TFMT_2D);
        InitCurrentSignalInfo();
        if (ret < 0) {
            LOGE("%s Set CurrentSourceInfo error!\n", __FUNCTION__);
        }

        mFrontDev->Close();
        mTvAction &= ~TV_ACTION_STOPING;
        mTvStatus = TV_STOP_ED;
        MnoNeedAutoSwitchToMonitorMode = false;
        // if (m_bTsPlayerRls == false) {
        //     mAv.delPlayer();
        //     m_bTsPlayerRls = true;
        // }

        mBlockState = BLOCK_STATE_NONE;
        mChannelBlockState = BLOCK_STATE_NONE;
        mChannelLastBlockState = BLOCK_STATE_NONE;
        mBlockStatusChanged = false;
        mpTvin->Resman_Release();
        LOGD("%s StopTvLock End SwitchSourceTime Time = %fs\n", __FUNCTION__,getUptimeSeconds());
        return 0;
    }
}

int CTv::StartTvInPIP ( tv_source_input_t source_input )
{
    m_pip_source_input = source_input;
    tvin_port_t cur_port = mpTvin->Tvin_GetSourcePortBySourceInput ( source_input );
    return mpTvin->SwitchVDIN2Port(cur_port);
}

int CTv::StopTvInPIP ( void )
{
    m_pip_source_input = SOURCE_INVALID;
    mpTvin->Tvin2_StopDecoder();
    int ret = mpTvin->VDIN2_ClosePort();
    InitCurrentVdin2SignalInfo();
    LOGD("%s \n", __FUNCTION__);

    return ret;
}

int CTv::Tv_MiscSetBySource ( tv_source_input_t source_input )
{
    int ret = -1;

    switch ( source_input ) {
    case SOURCE_TV:
        CVpp::getInstance()->VPP_SetScalerPathSel(1);
        ret = tvWriteSysfs ( SYS_VECM_DNLP_ADJ_LEVEL, "4" );
        break;

    case SOURCE_HDMI1:
    case SOURCE_HDMI2:
    case SOURCE_HDMI3:
    case SOURCE_HDMI4:
        CVpp::getInstance()->VPP_SetScalerPathSel(0);
        //ret = CVpp::getInstance()->Tv_SavePanoramaMode ( VPP_PANORAMA_MODE_FULL, SOURCE_TYPE_HDMI );
        ret |= tvWriteSysfs ( SYS_VECM_DNLP_ADJ_LEVEL, "5" );
        break;

    case SOURCE_DTV:
        CVpp::getInstance()->VPP_SetScalerPathSel(0);
        break;

    case SOURCE_AV1:
    case SOURCE_AV2:
    case SOURCE_VGA:
        CVpp::getInstance()->VPP_SetScalerPathSel(1);
        ret |= tvWriteSysfs ( SYS_VECM_DNLP_ADJ_LEVEL, "5" );
        break;

    case SOURCE_SVIDEO:
    case SOURCE_IPTV:
        CVpp::getInstance()->VPP_SetScalerPathSel(1);
        break;
    default:
        CVpp::getInstance()->VPP_SetScalerPathSel(0);
        ret |= tvWriteSysfs ( SYS_VECM_DNLP_ADJ_LEVEL, "5" );
        break;
    }
    return ret;
}

bool CTv::isVirtualSourceInput(tv_source_input_t source_input) {
    switch (source_input) {
        case SOURCE_ADTV:
            return true;
        default:
            return false;
    }
    return false;
}

int CTv::tryReleasePlayer(bool isEnter, tv_source_input_t si)
{
    if ((isEnter && si != SOURCE_ADTV && si != SOURCE_TV && si != SOURCE_DTV)
        ||(!isEnter && (si == SOURCE_ADTV || si == SOURCE_TV || si == SOURCE_DTV))){
        LOGD("remove all Player & Recorder");
        PlayerManager::getInstance().releaseAll();
        RecorderManager::getInstance().releaseAll();
    }
    return 0;
}

int CTv::SetSourceSwitchInput(tv_source_input_t source_input)
{
   AutoMutex _l( mLock );

    LOGD ( "%s, source input = %d SwitchSourceTime = %fs", __FUNCTION__, source_input,getUptimeSeconds());

    tryReleasePlayer(true, source_input);

    m_source_input_virtual = source_input;

    if (isVirtualSourceInput(source_input))
        return 0;//defer real source setting.

    return SetSourceSwitchInputLocked(m_source_input_virtual, source_input);
}

int CTv::SetSourceSwitchInput(tv_source_input_t virtual_input, tv_source_input_t source_input) {
    AutoMutex _l( mLock );
    return SetSourceSwitchInputLocked(virtual_input, source_input);
}

int CTv::SetSourceSwitchInputLocked(tv_source_input_t virtual_input, tv_source_input_t source_input)
{
    LOGD ( "SwitchSourceTime Time = %fs, %s, virtual source input = %d source input = %d m_source_input = %d",
           getUptimeSeconds(), __FUNCTION__, virtual_input, source_input, m_source_input );

    tvin_port_t cur_port;
    m_source_input_virtual = virtual_input;

    Tv_SetDDDRCMode(source_input);
     //DTV need frame AV sync, other source don't need it
    if (source_input != SOURCE_DTV) {
        tvWriteSysfs(VIDEO_SYNC_ENABLE, "0");
        // set sync mode to vmaster. 0: vmaster; 1:amaster
        tvWriteSysfs(VIDEO_SYNC_MODE, "0");
    } else {
        tvWriteSysfs(VIDEO_FREERUN_MODE, "0");
    }

    if (mTvAction & TV_ACTION_SCANNING) {
        LOGD("%s, tv need stop scanning \n", __FUNCTION__);
        mpTvin->Tvin_StopDecoder();
        mTvScanner->stopScan();
        mFrontDev->Close();
        mTvAction &= ~TV_ACTION_SCANNING;
    }

    if (source_input == m_source_input && source_input != SOURCE_TV) {
        LOGW("%s,same source input, return", __FUNCTION__ );
        return 0;
    }
    stopPlaying(false, !(source_input == SOURCE_DTV && m_source_input == SOURCE_TV));
    KillMediaServerClient();
    mTvAction |= TV_ACTION_SOURCE_SWITCHING;

    //set front dev mode
    if ( source_input == SOURCE_TV ) {
        mFrontDev->Open(TV_FE_AUTO);
        //mFrontDev->SetAnalogFrontEndTimerSwitch(1);
    } else if ( source_input == SOURCE_DTV ) {
        mFrontDev->Open(TV_FE_AUTO);
        //mFrontDev->SetAnalogFrontEndTimerSwitch(0);
    } else {
        mFrontDev->Close();
        //mFrontDev->SetAnalogFrontEndTimerSwitch(0);
    }

    //ok
    m_last_source_input = m_source_input;
    m_source_input = source_input;
    SSMSaveSourceInput ( source_input );

    if ( source_input == SOURCE_DTV ) {
        resetDmxAndAvSource();

        //we should stop audio first for audio mute.
        mpTvin->Tvin_StopDecoder();
        mpTvin->VDIN_ClosePort();
        //mpTvin->Tvin_WaitPathInactive ( TV_PATH_TYPE_DEFAULT );
        if (mpTvin->Tvin_CheckVideoPathComplete(TV_PATH_TYPE_DEFAULT) != 0) {
            if (mpTvin->Tvin_RemovePath (TV_PATH_TYPE_DEFAULT) > 0) {
                mpTvin->VDIN_AddVideoPath(TV_PATH_DECODER_AMLVIDEO2_PPMGR_DEINTERLACE_AMVIDEO);
            }
        }

        //double confirm we set the main volume lut buffer to mpeg
        //mpTvin->setMpeg2Vdin(1);
        //mAv.setLookupPtsForDtmb(1);
        int ret = tvSetCurrentSourceInfo(m_source_input, TVIN_SIG_FMT_NULL, TVIN_TFMT_2D);
        if (ret < 0) {
            LOGE("%s Set CurrentSourceInfo error!\n", __FUNCTION__);
        }
    } else {
        //mpTvin->setMpeg2Vdin(0);
        //mAv.setLookupPtsForDtmb(0);
    }

    cur_port = mpTvin->Tvin_GetSourcePortBySourceInput ( source_input );
    Tv_MiscSetBySource ( source_input );

    if (source_input != SOURCE_DTV) {
        if (mpTvin->SwitchPort ( cur_port ) == 0) { //ok
            mTvAction |= TV_ACTION_IN_VDIN;
            UpdateDlgProperty(source_input);//set DLG prop status
            if (source_input != SOURCE_SPDIF) {
                unsigned char std;
                SSMReadCVBSStd(&std);
                int fmt = CFrontEnd::stdEnumToCvbsFmt (std, std);
                mpTvin->AFE_SetCVBSStd ( ( tvin_sig_fmt_t ) fmt );

                //for HDMI source connect detect
                //mpTvin->VDIN_OpenHDMIPinMuxOn(true);
                //color range mode
                tvin_color_range_t colorRangeMode = (tvin_color_range_t)GetHdmiColorRangeMode();
                SetHdmiColorRangeMode(colorRangeMode);
            }
            m_sig_spdif_nums = 0;
        }
    }

    if (source_input == SOURCE_SPDIF) { //audio only in spdif source
        TvEvent::SignalInfoEvent ev;
        ev.mTrans_fmt = TVIN_TFMT_2D;
        ev.mFmt = TVIN_SIG_FMT_NULL;
        ev.mStatus = TVIN_SIG_STATUS_STABLE;
        ev.mReserved = 0;
        sendTvEvent ( ev );
    }

    mTvAction &= ~ TV_ACTION_SOURCE_SWITCHING;
    LOGD ( "%s, SwitchInputLocked End SwitchSourceTime Time = %fs", __FUNCTION__,getUptimeSeconds());
    return 0;
}

void CTv::onSigStatusChange()
{
    if (mTvAction & TV_ACTION_SCANNING) {
        LOGD("%s: TV SCANNING return!\n",__FUNCTION__);
        return;
    } else if (SOURCE_TV == m_source_input && !(mTvAction & TV_ACTION_PLAYING)) {
        LOGD("%s: ATV not yet ready, return!\n",__FUNCTION__);
        return;
    }

    int ret = mpTvin->VDIN_GetSignalInfo ( &m_cur_sig_info );
    if (ret < 0) {
        LOGD("Get Signal Info error!\n");
        m_cur_sig_info.status = TVIN_SIG_STATUS_NULL;
    }

    LOGD("%s: trans_fmt is %d, sig_fmt is %d, status is %d, isDVI is %d, hdr_info is 0x%x\n", __FUNCTION__,
    m_cur_sig_info.trans_fmt, m_cur_sig_info.fmt, m_cur_sig_info.status, m_cur_sig_info.is_dvi, m_cur_sig_info.hdr_info);
    if ( m_cur_sig_info.status == TVIN_SIG_STATUS_STABLE ) {
        onSigToStable();
        if (mVRRStatusChange) {
            mVRRStatusChange = false;
        } else {
            onSigStillStable();
        }
    } else if ( m_cur_sig_info.status == TVIN_SIG_STATUS_UNSTABLE ) {
        onSigToUnstable();
    } else if ( m_cur_sig_info.status == TVIN_SIG_STATUS_NOTSUP ) {
        onSigToUnSupport();
    } else if ( m_cur_sig_info.status == TVIN_SIG_STATUS_NOSIG ) {
        onSigToNoSig();
    } else {
        InitCurrentSignalInfo();
    }
}

void CTv::onSigToStable()
{
    LOGD ( "SwitchSourceTime Time = %fs, onSigToStable start", getUptimeSeconds());

    tv_source_input_type_t source_type = CTvin::Tvin_SourceInputToSourceInputType(m_source_input);
    if (source_type == SOURCE_TYPE_HDMI) {
        tvSetCurrentHdrInfo(m_cur_sig_info.hdr_info);
        tvSetCurrentAspectRatioInfo(m_cur_sig_info.aspect_ratio);
    }

    int ret = tvSetCurrentSourceInfo(m_source_input, m_cur_sig_info.fmt, m_cur_sig_info.trans_fmt);
    if (ret < 0) {
        LOGE("%s Set CurrentSourceInfo error!\n", __FUNCTION__);
    }

    if (source_type == SOURCE_TYPE_HDMI) {//set pq mode according to singal for hdmi source
        setPictureModeBySignal(PQ_MODE_SWITCH_TYPE_INIT);

        SetVRRStatusBySignal();
        if (mVRRStatusChange) return;
    }

    if (source_type == SOURCE_TYPE_TV) {
        int mode = tvGetDisplaymode(SOURCE_TYPE_TV);
        //5 means VPP_DISPLAY_MODE_FULL
        if (5 != mode) {
        tvSetDisplaymode(SOURCE_TYPE_TV, mode, 0);
        }
    }

    if (mAutoSetDisplayFreq && !mPreviewEnabled) {
        int freq = 60;
        if (source_type == SOURCE_TYPE_HDMI ) {//HDMI source
            int fps = getHDMIFrameRate();
            LOGD("%s: HDMI source frame rate is %d\n", __FUNCTION__, fps);
            freq = fps;
        }else {//ATV or AV source
            if (CTvin::Tvin_is50HzFrameRateFmt(m_cur_sig_info.fmt)) {
                freq = 50;
            } else {
                freq = 60;
            }
        }

        mpTvin->VDIN_SetDisplayVFreq(freq);
    }

    //showboz mark  hdmi auto 3d, tran fmt  is 3d, so switch to 3d
    if ( m_win_mode == PREVIEW_WINDOW ) {
        mAv.setVideoAxis(m_win_pos.x1, m_win_pos.y1, m_win_pos.x2, m_win_pos.y2);
        mAv.setVideoScreenMode ( CAv::VIDEO_WIDEOPTION_FULL_STRETCH );
    }
    LOGD ( "onSigToStable end");
}

void CTv::onSigStillStable()
{
    LOGD ( "%s, signal Still Stable!\n", __FUNCTION__);

    if (m_source_input == SOURCE_TV ) {//plug/unplug cable still display normally
        if (getScreenStaticFrameEnable()) {
            mAv.DisableVideoBlackout();
        } else {
            mAv.EnableVideoBlackout();
        }
    }

    if (needSnowEffect() && mpTvin->getSnowStatus()) {
        SetSnowShowEnable( false );
        mpTvin->Tvin_StopDecoder();
    } else if ((SOURCE_TV == m_source_input) && m_last_sig_info.fmt != m_cur_sig_info.fmt) {
        mpTvin->Tvin_StopDecoder();
    }
    //When the HDMI signal enters, ensure that it is in the stop decoding state
    if ((SOURCE_YPBPR2 < m_source_input) && (m_source_input < SOURCE_VGA)) {
        mpTvin->Tvin_StopDecoder();
    }
    LOGD ( "%s, startDecoder SwitchSourceTime Time = %fs\n", __FUNCTION__,getUptimeSeconds());
    int startdec_status = mpTvin->Tvin_StartDecoder ( m_cur_sig_info );
    if (isBlockedByChannelLock() || (mChannelBlockState == BLOCK_STATE_BLOCKED && mEnableLockModule)) {
        //if (mIsMultiDemux) {
        //    CVideotunnel::getInstance()->VT_setvideoColor(false, true);
        //} else {
        //    CVpp::getInstance()->VPP_setVideoColor(true);
        //}
        ScreenColorControl(false, VIDEO_LAYER_COLOR_SHOW_ALWAYES);
    }
    LOGD ( "%s, startDecoder End SwitchSourceTime = %fs\n", __FUNCTION__,getUptimeSeconds());
    //showboz  codes from  start decode fun
    if ((startdec_status == 0) &&
        ((SOURCE_TV == m_source_input) || (SOURCE_AV1 == m_source_input) || (SOURCE_AV2 == m_source_input))) {
        const char *value = config_get_str ( CFG_SECTION_TV, CFG_TVIN_DB_REG, "null" );
        if (strcmp(value, "enable") == 0) {
            CVpp::getInstance()->VPP_SetCVD2Values();
        }
    }

    LOGD ( "%s, mChannelBlockState:%d, mEnableLockModule:%d\n", __FUNCTION__, mChannelBlockState, mEnableLockModule);
    if ((!isBlockedByChannelLock() && mChannelBlockState != BLOCK_STATE_BLOCKED)
            || (mEnableLockModule && mChannelBlockState != BLOCK_STATE_BLOCKED)
            || !mEnableLockModule) {
        mAv.SetVideoLayerStatus(ENABLE_VIDEO_LAYER);
    }
    CMessage msg;
    msg.mDelayMs = 0;
    msg.mType = CTvMsgQueue::TV_MSG_ENABLE_VIDEO_LATER;
    msg.mpPara[0] = 2;
    mTvMsgQueue->sendMsg ( msg );

    m_last_sig_info.fps            = m_cur_sig_info.fps;
    m_last_sig_info.is_dvi         = m_cur_sig_info.is_dvi;
    m_last_sig_info.trans_fmt      = m_cur_sig_info.trans_fmt;
    m_last_sig_info.fmt            = m_cur_sig_info.fmt;
    m_last_sig_info.status         = m_cur_sig_info.status;
    m_last_sig_info.cfmt           = m_cur_sig_info.cfmt;
    m_last_sig_info.hdr_info       = m_cur_sig_info.hdr_info;
    m_last_sig_info.aspect_ratio   = m_cur_sig_info.aspect_ratio;
    m_last_sig_info.amdolby_vision = m_cur_sig_info.amdolby_vision;
    m_last_sig_info.low_latency    = m_cur_sig_info.low_latency;

    TvEvent::SignalInfoEvent ev;
    ev.mTrans_fmt = m_cur_sig_info.trans_fmt;
    ev.mFmt = m_cur_sig_info.fmt;
    ev.mStatus = m_cur_sig_info.status;
    ev.mReserved = getHDMIFrameRate();
    sendTvEvent ( ev );

    if (isBlockedByChannelLock() || (mChannelBlockState == BLOCK_STATE_BLOCKED && mEnableLockModule)) {
        SendBlockEvt(true);
    }
}

void CTv::isVideoFrameAvailable(unsigned int u32NewFrameCount)
{
    unsigned int u32TimeOutCount = 0;
    while (!mIsMultiDemux) {//new path, this node has inactive
        if (m_source_input == SOURCE_TV || m_source_input == SOURCE_DTV) {
            if ((mTvAction & TV_ACTION_PLAYING) != TV_ACTION_PLAYING) {
            LOGD("%s not play,return", __FUNCTION__);
            return;
            }
        }
        if ((unsigned int)mAv.getVideoFrameCount() >= u32NewFrameCount) {
            LOGD("%s video available SwitchSourceTime = %f", __FUNCTION__,getUptimeSeconds());
            break;
        } else {
            if (u32TimeOutCount >= NEW_FRAME_TIME_OUT_COUNT) {
            LOGD("%s Not available frame consume time = %f", __FUNCTION__,getUptimeSeconds());
            break;
            }
        }
        LOGD("%s new frame count = %d",__FUNCTION__,mAv.getVideoFrameCount());
        usleep(10*1000);
        u32TimeOutCount++;
    }
    m_cur_sig_info.status = TVIN_SIG_STATUS_STABLE;
    //clean blue screen
    if (!(mTvAction & TV_ACTION_SCANNING)) {
        if (isBlockedByChannelLock() || (mChannelBlockState == BLOCK_STATE_BLOCKED && mEnableLockModule)) {
            //blocked, still show with black or blue screen
            ScreenColorControl(false, VIDEO_LAYER_COLOR_SHOW_ALWAYES);
        } else {
            //normal, clear with black/blue and then will be flush by video data
            ScreenColorControl(false, VIDEO_LAYER_COLOR_SHOW_DISABLE);
        }
    }
}
void CTv::onEnableVideoLater(int framecount)
{
    mAv.EnableVideoWhenVideoPlaying(framecount);
}

void CTv::onVideoAvailableLater(int framecount)
{
    LOGD("[ctv]onVideoAvailableLater framecount = %d", framecount);
    int ret = mAv.WaittingVideoPlaying(framecount);
    if (ret == 0) { //video available
        TvEvent::SignalInfoEvent ev;
        ev.mStatus = TVIN_SIG_STATUS_STABLE;
        ev.mTrans_fmt = TVIN_TFMT_2D;
        ev.mFmt = TVIN_SIG_FMT_NULL;
        ev.mReserved = 1;/*<< VideoAvailable flag here*/
        sendTvEvent ( ev );
    }
}

void CTv::onSigToUnstable()
{
    LOGD ( "%s, signal to Unstable SwitchSourceTime = %fs\n", __FUNCTION__,getUptimeSeconds());
    if (SOURCE_TV == m_source_input) {
        if (mTvAction & TV_ACTION_SCANNING) {
            if (needSnowEffect()) {
                SetSnowShowEnable(true);
                mpTvin->Tvin_StartDecoder(m_cur_sig_info);
                mAv.EnableVideoNow(false);
            } else {
                CVpp::getInstance()->VPP_setVideoColor(false);
                mpTvin->Tvin_StopDecoder();
            }
        } else {
            if (isBlockedByChannelLock()) {
                CVpp::getInstance()->VPP_setVideoColor(true);
                SendBlockEvt(true);
            }
        }
    } else {
        ScreenColorControl(false,VIDEO_LAYER_COLOR_SHOW_ALWAYES);
        mpTvin->Tvin_StopDecoder();
    }
}

void CTv::onSigToUnSupport()
{
    LOGD ( "%s, signal to UnSupport\n", __FUNCTION__);
    if (mTvAction & TV_ACTION_SCANNING) {
        if (needSnowEffect()) {
            SetSnowShowEnable(true);
            mpTvin->Tvin_StartDecoder(m_cur_sig_info);
            mAv.EnableVideoNow(false);
        } else {
            CVpp::getInstance()->VPP_setVideoColor(true);
            mpTvin->Tvin_StopDecoder();
        }
    } else {
        if (SOURCE_TV == m_source_input) {
            if (getScreenColorSetting() == VIDEO_LAYER_COLOR_BLUE) {
                CVpp::getInstance()->VPP_setVideoColor(true);
            } else {
                if (needSnowEffect()) {
                    SetSnowShowEnable(true);
                    mpTvin->Tvin_StartDecoder(m_cur_sig_info);
                    mAv.EnableVideoNow(false);
                } else {
                    CVpp::getInstance()->VPP_setVideoColor(true);
                }
            }
        } else {
            CVpp::getInstance()->VPP_setVideoColor(true);
            mpTvin->Tvin_StopDecoder();
        }
    }

    if (!(mTvAction & TV_ACTION_SCANNING)) {
        //tvin_info_t info = m_cur_sig_info;
        TvEvent::SignalInfoEvent ev;
        ev.mTrans_fmt = m_cur_sig_info.trans_fmt;
        ev.mFmt = m_cur_sig_info.fmt;
        ev.mStatus = m_cur_sig_info.status;
        ev.mReserved = m_cur_sig_info.is_dvi;
        sendTvEvent ( ev );
    }
}

void CTv::onSigToNoSig()
{
    LOGD ( "%s, signal to NoSignal\n", __FUNCTION__);

    tv_source_input_type_t source_type = CTvin::Tvin_SourceInputToSourceInputType(m_source_input);
    if (source_type == SOURCE_TYPE_HDMI) {
        tvSetCurrentHdrInfo(m_cur_sig_info.hdr_info);
        tvSetCurrentAspectRatioInfo(m_cur_sig_info.aspect_ratio);
    }

    if (mTvAction & TV_ACTION_SCANNING) {
        if (needSnowEffect()) {
            SetSnowShowEnable(true);
            mpTvin->Tvin_StartDecoder(m_cur_sig_info);
            mAv.EnableVideoNow(false);
            CVpp::getInstance()->VPP_setVideoColor(false);
        } else {
            CVpp::getInstance()->VPP_setVideoColor(true);
            mpTvin->Tvin_StopDecoder();
        }
    } else {
        if (SOURCE_TV == m_source_input) {
            if (needSnowEffect()) {//first open snow
                SetSnowShowEnable(true);
                //5 means VPP_DISPLAY_MODE_FULL
                tvSetDisplaymode(m_source_input, 5, 0);
                mpTvin->Tvin_StartDecoder(m_cur_sig_info);
            }
            if (getScreenColorSetting() == VIDEO_LAYER_COLOR_BLUE) {
                ScreenColorControl(true, VIDEO_LAYER_COLOR_SHOW_ALWAYES);
            } else {
                if (needSnowEffect()) {
                    mAv.EnableVideoNow(false);
                    ScreenColorControl(false, VIDEO_LAYER_COLOR_SHOW_DISABLE);
                } else {
                    ScreenColorControl(true, VIDEO_LAYER_COLOR_SHOW_ALWAYES);
                }
            }
        } else {
            ScreenColorControl(true, VIDEO_LAYER_COLOR_SHOW_ALWAYES);
            //CVpp::getInstance()->VPP_setVideoColor(true);
            mpTvin->Tvin_StopDecoder();
        }
    }

    if (!(mTvAction & TV_ACTION_SCANNING)) {
        //tvin_info_t info = m_cur_sig_info;
        TvEvent::SignalInfoEvent ev;
        ev.mTrans_fmt = m_cur_sig_info.trans_fmt;
        ev.mFmt = m_cur_sig_info.fmt;
        ev.mStatus = m_cur_sig_info.status;
        ev.mReserved = m_cur_sig_info.is_dvi;
        sendTvEvent ( ev );
    }
}

void CTv::onBootvideoRunning() {
    //LOGD("%s,boot video is running", __FUNCTION__);
}

void CTv::onBootvideoStopped() {
    LOGD("%s,boot video has stopped", __FUNCTION__);
    if (mpTvin->Tvin_RemovePath (TV_PATH_TYPE_TVIN) > 0) {
        mpTvin->VDIN_AddVideoPath(TV_PATH_VDIN_AMLVIDEO2_PPMGR_DEINTERLACE_AMVIDEO);
    }
}

void CTv::onSourceConnect(int source_type, int connect_status)
{
    TvEvent::SourceConnectEvent ev;
    ev.mSourceInput = source_type;
    ev.connectionState = connect_status;
    sendTvEvent(ev);
}

int CTv::GetSourceConnectStatus(tv_source_input_t source_input)
{
    return mDevicesPollStatusDetectThread->GetSourceConnectStatus((tv_source_input_t)source_input);
}

tv_source_input_t CTv::GetCurrentSourceInputLock ( void )
{
    AutoMutex _l( mLock );
    return m_source_input;
}

tv_source_input_t CTv::GetCurrentSourceInputVirtualLock ( void )
{
    AutoMutex _l( mLock );
    return m_source_input_virtual;
}

//dtv and tvin
void CTv::InitCurrentSignalInfo ( void )
{
    m_cur_sig_info.fps = 0;
    m_cur_sig_info.is_dvi = 0;
    m_cur_sig_info.trans_fmt      = TVIN_TFMT_2D;
    m_cur_sig_info.fmt            = TVIN_SIG_FMT_NULL;
    m_cur_sig_info.status         = TVIN_SIG_STATUS_NULL;
    m_cur_sig_info.cfmt           = COLOR_FMT_MAX;
    m_cur_sig_info.hdr_info       = 0;
    m_cur_sig_info.aspect_ratio   = TVIN_ASPECT_NULL;
    m_cur_sig_info.amdolby_vision = 0;
    m_cur_sig_info.low_latency    = 0;

    m_last_sig_info.fps = 0;
    m_last_sig_info.is_dvi = 0;
    m_last_sig_info.trans_fmt      = TVIN_TFMT_2D;
    m_last_sig_info.fmt            = TVIN_SIG_FMT_NULL;
    m_last_sig_info.status         = TVIN_SIG_STATUS_NULL;
    m_last_sig_info.cfmt           = COLOR_FMT_MAX;
    m_last_sig_info.hdr_info       = 0;
    m_last_sig_info.aspect_ratio   = TVIN_ASPECT_NULL;
    m_last_sig_info.amdolby_vision = 0;
    m_last_sig_info.low_latency    = 0;
}

void CTv::InitCurrentVdin2SignalInfo ( void )
{
    m_cur_vdin2_sig_info.fps = 0;
    m_cur_vdin2_sig_info.is_dvi = 0;
    m_cur_vdin2_sig_info.trans_fmt      = TVIN_TFMT_2D;
    m_cur_vdin2_sig_info.fmt            = TVIN_SIG_FMT_NULL;
    m_cur_vdin2_sig_info.status         = TVIN_SIG_STATUS_NULL;
    m_cur_vdin2_sig_info.cfmt           = COLOR_FMT_MAX;
    m_cur_vdin2_sig_info.hdr_info       = 0;
    m_cur_vdin2_sig_info.aspect_ratio   = TVIN_ASPECT_NULL;
    m_cur_vdin2_sig_info.amdolby_vision = 0;
    m_cur_vdin2_sig_info.low_latency    = 0;
}

tvin_info_t CTv::GetCurrentSignalInfo ( void )
{
    tvin_trans_fmt det_fmt = TVIN_TFMT_2D;
    //tvin_sig_status_t signalState = TVIN_SIG_STATUS_NULL;
    //tvin_info_t signal_info = m_cur_sig_info;

    int feState = mFrontDev->getStatus();
    if ( (CTvin::Tvin_SourceInputToSourceInputType(m_source_input) == SOURCE_TYPE_DTV ) ) {
        det_fmt = mpTvin->TvinApi_Get3DDectMode();
        if ((feState & TV_FE_HAS_LOCK) == TV_FE_HAS_LOCK) {
            m_cur_sig_info.status = TVIN_SIG_STATUS_STABLE;
        } else if ((feState & TV_FE_TIMEDOUT) == TV_FE_TIMEDOUT) {
            m_cur_sig_info.status = TVIN_SIG_STATUS_NOSIG;
        }
        if ( det_fmt != TVIN_TFMT_2D ) {
            m_cur_sig_info.trans_fmt = det_fmt;
        }
        m_cur_sig_info.fmt = mAv.getVideoResolutionToFmt();
    } else {
        //Check whether the signal line is inserted, if there is no insertion, return directly without signal
        if ((m_source_input == SOURCE_AV1)
            || (m_source_input == SOURCE_AV2 )
            || (m_source_input == SOURCE_INVALID)) {
            if (GetSourceConnectStatus(m_source_input) == CC_SOURCE_PLUG_OUT) {
                m_cur_sig_info.status = TVIN_SIG_STATUS_NOSIG;
            }
        }
    }
    LOGD("%s, m_source_input = %d, trans_fmt is %d,fmt is %d, status is %d", __FUNCTION__,
            m_source_input, m_cur_sig_info.trans_fmt, m_cur_sig_info.fmt, m_cur_sig_info.status);
    return m_cur_sig_info;
}

int CTv::setPreviewWindowMode(bool mode)
{
    mPreviewEnabled = mode;
    int screenColor = 0;
    SSMReadScreenColorForSignalChange(&screenColor);
    if (screenColor != 0) {
       if (mPreviewEnabled) {
           mAv.SetVideoScreenColor(VIDEO_LAYER_BLACK);
       } else if (m_cur_sig_info.status != TVIN_SIG_STATUS_STABLE) {
           mAv.SetVideoScreenColor(VIDEO_LAYER_BLUE);
       }
   }
    return 0;
}

int CTv::SetPreviewWindow ( tvin_window_pos_t pos )
{
    m_win_pos.x1 = pos.x1;
    m_win_pos.y1 = pos.y1;
    m_win_pos.x2 = pos.x2;
    m_win_pos.y2 = pos.y2;
    LOGD ( "%s, SetPreviewWindow x = %d y=%d", __FUNCTION__, pos.x2, pos.y2 );

    tvin_window_pos_t def_pos;
    Vpp_GetDisplayResolutionInfo(&def_pos);

    if (pos.x1 != 0 || pos.y1 != 0 || pos.x2 != def_pos.x2 || pos.y2 != def_pos.y2) {
        m_win_mode = PREVIEW_WINDOW;
    } else {
        m_win_mode = NORMAL_WINDOW;
    }

    return mAv.setVideoAxis(m_win_pos.x1, m_win_pos.y1, m_win_pos.x2, m_win_pos.y2);
}

unsigned int CTv::Vpp_GetDisplayResolutionInfo(tvin_window_pos_t *win_pos)
{
    int display_resolution = mAv.getVideoDisplayResolution();
    int width = 0, height = 0;
    switch (display_resolution) {
    case VPP_DISPLAY_RESOLUTION_1366X768:
        width = CC_RESOLUTION_1366X768_W;
        height = CC_RESOLUTION_1366X768_H;
        break;
    case VPP_DISPLAY_RESOLUTION_1920X1080:
        width = CC_RESOLUTION_1920X1080_W;
        height = CC_RESOLUTION_1920X1080_H;
        break;
    case VPP_DISPLAY_RESOLUTION_3840X2160:
        width = CC_RESOLUTION_3840X2160_W;
        height = CC_RESOLUTION_3840X2160_H;
        break;
    default:
        width = CC_RESOLUTION_3840X2160_W;
        height = CC_RESOLUTION_3840X2160_H;
        break;
    }

    if (win_pos != NULL) {
        win_pos->x1 = 0;
        win_pos->y1 = 0;
        win_pos->x2 = width - 1;
        win_pos->y2 = height - 1;
    }
    return 0;
}

bool CTv::needSnowEffect() {
    bool isEnable = false;
    if ((SOURCE_TV == m_source_input) && mATVDisplaySnow) {
        isEnable = true;
        LOGD("%s: snow display is enabled.\n", __FUNCTION__);
    } else {
        LOGD("%s: snow display is disabled.\n", __FUNCTION__);
    }

    return isEnable;
}

int CTv::getAverageLuma()
{
    return mpTvin->VDIN_Get_avg_luma();
}

int CTv::setAutobacklightData(const char *value)
{
    const char *KEY = "haier.autobacklight.mp.data";

    const char *keyValue = config_get_str(CFG_SECTION_TV, KEY, "null");
    if (strcmp(keyValue, value) != 0) {
        config_set_str(CFG_SECTION_TV, KEY, value);
        LOGD("%s, config string now set as: %s \n", __FUNCTION__, keyValue);
    } else {
        LOGD("%s, config string has been set as: %s \n", __FUNCTION__, keyValue);
    }
    return 0;
}

int CTv::getAutoBacklightData(int *data)
{
    char datas[512] = {0};
    char delims[] = ",";
    char *result = NULL;
    const char *KEY = "haier.autobacklight.mp.data";
    int i = 0;

    if (NULL == data) {
        LOGE("[ctv]%s, data is null", __FUNCTION__);
        return -1;
    }

    const char *keyValue = config_get_str(CFG_SECTION_TV, KEY, (char *) "null");
    if (strcmp(keyValue, "null") == 0) {
        LOGE("[ctv]%s, value is NULL\n", __FUNCTION__);
        return -1;
    }

    strncpy(datas, keyValue, sizeof(datas) - 1);
    result = strtok( datas, delims );
    while ( result != NULL ) {
        char *pd = strstr(result, ":");
        if (pd != NULL) {
            data[i] = strtol(pd + 1, NULL, 10 );
            i++;
        }
        result = strtok( NULL, delims );
    }
    return i;
}

int CTv::setLcdEnable(bool enable)
{
    int kernelVersion = getKernelMajorVersion();
    if (kernelVersion > 4) {
        return tvWriteSysfs(LCD_ENABLE_54, enable ? 1 : 0);
    } else {
        return tvWriteSysfs(LCD_ENABLE_49, enable ? 1 : 0);
    }
}

int CTv::Tv_GetIwattRegs()
{
    int ret = -1;
    time_t timer;
    struct tm *tp;
    char path[100] = {0};

    time(&timer);
    tp = localtime(&timer);

    sprintf(path, "%s /data/log/%s-%d-%d-%d-%d%d%d",
            IWATT_SHELL_PATH,
            "iwatt-7019", tp->tm_year + 1900, tp->tm_mon+1,
            tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
    LOGD("save iwatt regs to %s", path);

    ret = system(path);
    if (ret < 0) {
        LOGE("%s: system command error\n", __FUNCTION__);
    }

    return ret;
}

/*********************** SSM  start **********************/
int CTv::Tv_SSMRestoreDefaultSetting()
{
    SSMRestoreDeviceMarkValues();
    MiscSSMRestoreDefault();
    ReservedSSMRestoreDefault();
    SSMSaveCVBSStd ( 0 );
    SSMSaveLastSelectSourceInput ( SOURCE_TV );
    SSMSavePanelType ( 0 );
    //tvconfig default
    saveDTVProgramID ( -1 );
    saveATVProgramID ( -1 );
    SSMSaveStandbyMode( 0 );
    //restore EDID
    const char * value;
    value = config_get_str ( CFG_SECTION_TV, CFG_SSM_HDMI_EDID_VERSION, "null" );
    if (strcmp(value, "edid_20") == 0 ) {
        SSMHDMIEdidRestoreDefault(HDMI_EDID_VER_20);
    } else {
        SSMHDMIEdidRestoreDefault(HDMI_EDID_VER_14);
    }
    SSMSaveHDMIHdcpSwitcher(0);
    SSMSaveHDMIColorRangeMode(TVIN_COLOR_RANGE_AUTO);
    return 0;
}

int CTv::clearDbAllProgramInfoTable()
{
    return CTvDatabase::GetTvDb()->clearDbAllProgramInfoTable();
}

int CTv::Tv_SSMFacRestoreDefaultSetting()
{
    MiscSSMFacRestoreDefault();
    return 0;
}
/*********************** SSM  End **********************/

//not in CTv, not use lock
void CTv::setSourceSwitchAndPlay()
{
    int progID = 0;
    CTvProgram prog;
    LOGD ( "%s\n", __FUNCTION__ );
    static const int POWERON_SOURCE_TYPE_NONE = 0;//not play source
    static const int POWERON_SOURCE_TYPE_LAST = 1;//play last save source
    static const int POWERON_SOURCE_TYPE_SETTING = 2;//play ui set source
    int to_play_source = -1;
    int powerup_type = SSMReadPowerOnOffChannel();
    LOGD("read power on source type = %d", powerup_type);
    if (powerup_type == POWERON_SOURCE_TYPE_NONE) {
        return ;
    } else if (powerup_type == POWERON_SOURCE_TYPE_LAST) {
        to_play_source = SSMReadSourceInput();
    } else if (powerup_type == POWERON_SOURCE_TYPE_SETTING) {
        to_play_source = SSMReadLastSelectSourceInput();
    }
    SetSourceSwitchInput (( tv_source_input_t ) to_play_source );
    if ( to_play_source == SOURCE_TV ) {
        progID = getATVProgramID();
    } else if ( to_play_source == SOURCE_DTV ) {
        progID = getDTVProgramID();
    }
}

//==============vchip end================================

int CTv::GetHdmiHdcpKeyKsvInfo(int data_buf[])
{
    int ret = -1;
    if (m_source_input != SOURCE_HDMI1 && m_source_input != SOURCE_HDMI2 && m_source_input != SOURCE_HDMI3
       &&m_source_input != SOURCE_HDMI4) {
        return -1;
    }

    struct _hdcp_ksv msg;
    ret = mHDMIRxManager.GetHdmiHdcpKeyKsvInfo(&msg);
    memset((void *)data_buf, 0, 2);
    data_buf[0] = msg.bksv0;
    data_buf[1] = msg.bksv1;
    return ret;
}

bool CTv::hdmiOutWithFbc()
{
    const char *value = config_get_str(CFG_SECTION_TV, CFG_FBC_USED, "true");
    if (strcmp(value, "true") == 0) {
        return true;
    }

    return false;
}

int CTv::StartHeadSetDetect()
{
    mHeadSet.startDetect();
    return 0;
}

void CTv::onHeadSetDetect(int state, int para)
{
    TvEvent::HeadSetOf2d4GEvent ev;
    if (state == 1)
        property_set("audio.headset_plug.enable", "1");
    else
        property_set("audio.headset_plug.enable", "0");
    ev.state = state;
    ev.para = para;
    sendTvEvent(ev);
}

int CTv::setFEStatus(int value)
{
    return mAv.setFEStatus(value);
}

void CTv::onThermalDetect(int state)
{
    const char *value;
    int val = 0;
    static int preValue = -1;

    value = config_get_str ( CFG_SECTION_TV, CFG_TVIN_THERMAL_THRESHOLD_ENABLE, "null" );
    if ( strcmp ( value, "enable" ) == 0 ) {
        value = config_get_str ( CFG_SECTION_TV, CFG_TVIN_THERMAL_THRESHOLD_VALUE, "null" );
        int threshold = atoi(value);
        LOGD ( "%s, threshold value: %d\n", __FUNCTION__, threshold);

        if (state > threshold) {
            const char *valueNormal = config_get_str ( CFG_SECTION_TV, CFG_TVIN_THERMAL_FBC_NORMAL_VALUE, "null" );
            val = atoi(valueNormal);
            if (val == 0) {
                val = 0x4210000;    //normal default
            }
            LOGD ( "%s, current temp: %d set 1\n", __FUNCTION__, state);
        } else {
            const char *valueCold = config_get_str ( CFG_SECTION_TV, CFG_TVIN_THERMAL_FBC_COLD_VALUE, "null" );
            val = atoi(valueCold);
            if (val == 0) {
                val = 0x8210000;    //cold default
            }
            LOGD ( "%s, current temp: 0x%x set 0\n", __FUNCTION__, state);
        }

        if (preValue != val) {
            LOGD ( "%s, pre value :0x%x, new value :0x%x, bypass\n", __FUNCTION__, preValue, val);
            preValue = val;
            mFactoryMode.fbcSetThermalState(val);
        }
    } else {
        LOGD ( "%s, tvin thermal threshold disable\n", __FUNCTION__);
    }
}

int CTv::Tv_SetDDDRCMode(tv_source_input_t source_input)
{
    if (source_input == SOURCE_DTV) {
        if (GetPlatformHaveDDFlag() == 1) {
            tvWriteSysfs(SYS_AUDIO_DSP_AC3_DRC, (char *)"drcmode 3");
        }
    } else {
        if (GetPlatformHaveDDFlag() == 1) {
            tvWriteSysfs(SYS_AUDIO_DSP_AC3_DRC, (char *)"drcmode 2");
        }
    }
    return 0;
}

int CTv::Tv_SetVdinForPQ (int gameStatus, int pcStatus, int autoSwitchFlag)
{
    LOGD("%s: game mode: %d, pc mode: %d, autoSwitchFlag: %d\n", __FUNCTION__, gameStatus, pcStatus, autoSwitchFlag);

    int ret = -1;
    if (pcStatus == MODE_ON) {
        CVpp::getInstance()->enableMonitorMode(true);
        mpTvin->VDIN_SetMonitorMode(MODE_ON);
    } else if (pcStatus == MODE_OFF){
        CVpp::getInstance()->enableMonitorMode(false);
        mpTvin->VDIN_SetMonitorMode(MODE_OFF);
    } else {
        LOGD("%s: not about pc mode!\n", __FUNCTION__);
    }

    if (gameStatus != MODE_STABLE) {
        ret = mpTvin->VDIN_SetGameMode((pq_status_update_e)gameStatus);
    }

    if (m_source_input != SOURCE_INVALID) {
        setDlgControl();
    }

    if (autoSwitchFlag == PQ_MODE_SWITCH_TYPE_INIT) {
        MnoNeedAutoSwitchToMonitorMode = false;
    } else {
        if (autoSwitchFlag == PQ_MODE_SWITCH_TYPE_MANUAL) {
            MnoNeedAutoSwitchToMonitorMode = true;
        } else {
            MnoNeedAutoSwitchToMonitorMode = false;
        }
        //tvin_port_t cur_port = mpTvin->Tvin_GetSourcePortBySourceInput(m_source_input);
        //mpTvin->SwitchPort (cur_port);
        ScreenColorControl(false, VIDEO_LAYER_COLOR_SHOW_ALWAYES);
        if (mpTvin->Tvin_StopDecoder() < 0) {
        LOGW ( "%s,stop decoder failed.", __FUNCTION__);
        }
    }

    return ret;
}

int CTv::Tv_SetDLGEnable(bool enable)
 {
    int ret = -1;
    if ( 0 ==SupportDlg() ) {
        LOGD("%s:update DLG EDID status, new status = [%d]!\n",__FUNCTION__,enable);
        int amdvEnableState = GetAmdvEnable();
        if ( enable ) {
            ret = LoadEdidData(1,amdvEnableState,1);
             if ( 0 == ret ) tvSetDLGEnable(1);
        } else {
            ret = LoadEdidData(1,amdvEnableState,0);
            if ( 0 == ret ) tvSetDLGEnable(0);
        }

        if ( 0 == ret ) {
            if (m_source_input != SOURCE_INVALID) {
                setDlgControl();
            }
            LOGD("%s:update DLG EDID status success!\n",__FUNCTION__);
        } else {
            LOGD("%s:update DLG EDID status fail!\n",__FUNCTION__);
        }
    }else {
    LOGE("%s:unsupport DLG func,return !\n",__FUNCTION__);
    }

    return ret;
}

int CTv::Tv_GetDLGEnable()
{
    int dlgEnable = tvGetDLGEnable();
    LOGD("%s:DLG status [%d] !\n",__FUNCTION__,dlgEnable);
    return dlgEnable;
}

int CTv::UpdateDlgProperty(tv_source_input_t source_input)
{
    int ret = -1;
    //char buf[64] = {0};
    tv_hdmi_port_id_t cur_port = mpTvin->Tvin_GetHdmiPortIDBySourceInput(source_input);
    tv_source_input_type_t type = mpTvin->Tvin_SourceInputToSourceInputType(source_input);
    int value = SSMReadHDMIEdidVersion(cur_port);
    int DlgEnable = Tv_GetDLGEnable();
    LOGD("%s: cur_port[%d],source type[%d] ,EdidVersion[%d],DlgEnable[%d],mDlgStatus[%d] !\n",__FUNCTION__,cur_port,type,value,DlgEnable,mDLGStatus);
    if ( (1 == value) && (SOURCE_TYPE_HDMI == type)) {//2.0 open & HDMI source
        if ((1 == DlgEnable) && (false == mDLGStatus)) {//DLG switch open & last DLG status as false
            //strcpy(buf,"3840x1080p120hz");
            //LOGD("%s: Write DLG DisplayMode[%s] !\n",__FUNCTION__,buf);
            //ret = tvWriteDisplayMode(buf);
            setProperty(PROP_DLG_STATUS,DLG_MODE_ON);
            mDLGStatus = true;
            LOGD("%s: set DLG prop on!\n",__FUNCTION__);
        } else if ( (0 == DlgEnable) && (true == mDLGStatus)) {//last DLG status as true and DLG close, close it
            setProperty(PROP_DLG_STATUS,DLG_MODE_OFF);
            mDLGStatus = false;
            LOGD("%s: set DLG prop off!\n",__FUNCTION__);
        } else {
            LOGD("%s: need't do anything!\n",__FUNCTION__);
        }
    } else if (mDLGStatus) {
        setProperty(PROP_DLG_STATUS,DLG_MODE_OFF);
        mDLGStatus = false;
        LOGD("%s: Conditions not met, set DLG prop off !\n",__FUNCTION__);
    } else {
        LOGD("%s: needn't change DLG status !\n",__FUNCTION__);
    }
    return ret;
}

int CTv::SupportDlg()
{
    int ret = -1;
    char buf[1024] = {0};
    ret = tvReadSysfs(DLG_FUNC_PATH, buf);
    LOGD("%s: %s\n", __FUNCTION__, buf);
    if (ret < 0) {
        LOGD("%s: read /sys/class/display/cap error, return!\n",__FUNCTION__);
        return ret;
    }

    std::string parm = buf;

    //if 4k1k && 4k2k exist,support dlg
    std::string support4K1K = "3840x1080";
    //The format of 4K2K includes the following variants
    std::string support4K2K_1 = "3840x2160";
    std::string support4K2K_2 = "2160p60hz";
    std::string support4K2K_3 = "4k2k60hz";
    if ((parm.find(support4K1K) != -1)
        && ((parm.find(support4K2K_1) != -1)
        || (parm.find(support4K2K_2) != -1)
        || (parm.find(support4K2K_3) != -1))) {
        LOGD("%s: find exist!\n",__FUNCTION__);
        ret = 0;
    }else{
        LOGD("%s: unsupport DLG func,return!\n",__FUNCTION__);
    }

    return ret;
}

int CTv::GetAmdvEnable()
{
    int amdvEnableState = 0;
    char buf[32] = {0};
    int kernelVersion = getKernelMajorVersion();
    if (kernelVersion > 4) {
        tvReadSysfs(DOLBY_VISION_ENABLE_PATH_54, buf);
    } else {
        tvReadSysfs(DOLBY_VISION_ENABLE_PATH_49, buf);
    }
    if (strcmp("Y", buf) == 0) {
        amdvEnableState = true;
    } else {
        amdvEnableState = false;
            }
    return amdvEnableState;
}

int CTv::TV_SetVRREnable (bool enable)
{
    LOGD("%s: Set VRR switch! enable = [%d]\n", __FUNCTION__, enable);

    int ret = -1;
    int kernelVersion = getKernelMajorVersion();
    if (kernelVersion < 5) {
        LOGD("%s: kernel Version unsupport VRR func, return!\n", __FUNCTION__);
        return ret;
    }

    mpTvin->Tvin_StopDecoder();
    tvWriteSysfs(VRR_FUNC_CTRL_PATH, (int)enable );
    SSMSaveVRREnable(int8_t(enable));
    mVrrStatus = enable;
    if (mIsLoadEdidWithPort) {
        for (int i=HDMI_PORT_1;i<HDMI_PORT_MAX;i++) {
            mHDMIRxManager.HdmiRxEdidUpdateWithPort(i);
        }
    } else {
        mHDMIRxManager.HdmiRxEdidUpdate();
    }
    LOGD("%s: Set VRR success!\n", __FUNCTION__);

    return 0;
}

int CTv::TV_GetVRREnable()
{
    int8_t vrrEnable = 0;
    SSMReadVRREnable(&vrrEnable);
    LOGD("%s:VRR switch status [%d] !\n",__FUNCTION__,vrrEnable);
    return vrrEnable;
}

int CTv::SetVRRStatusBySignal(void)
{
    int ret = 0;
    if (TV_GetVRREnable()) {
        //vpp_picture_mode_t BeforePQmode = tvGetLastPQMode();
        vpp_picture_mode_t CurrentPQmode = tvGetPQMode();
        tv_hdmi_port_id_t cur_port = mpTvin->Tvin_GetHdmiPortIDBySourceInput(m_source_input);
        int edidVersion = SSMReadHDMIEdidVersion(cur_port);
        LOGD("%s: currentPQmode:[%d], HDMI 2.0:%d, mVrrStatus:[%d]\n",__FUNCTION__,
            CurrentPQmode, edidVersion, mVrrStatus);
        if ((1 == edidVersion) && (VPP_PICTURE_MODE_GAME == CurrentPQmode)) {
            if (mVrrStatus) {
                LOGD("%s:VRR has opend,return !\n",__FUNCTION__);
                return ret;
            } else {
                tvWriteSysfs(VRR_FUNC_CTRL_PATH, 1 );
                mVrrStatus = true;
            }
        } else if ((1 != edidVersion) || (VPP_PICTURE_MODE_GAME != CurrentPQmode)) {
            if (mVrrStatus) {
                tvWriteSysfs(VRR_FUNC_CTRL_PATH, 0 );
                mVrrStatus = false;
            } else {
                LOGD("%s:VRR has closed,return !\n",__FUNCTION__);
                return ret;
            }
        }
    } else {
        LOGD("%s:needn't do anything !\n",__FUNCTION__);
        return ret;
    }

    LOGD("%s:update VRR status:[%d] !\n", __FUNCTION__, mVrrStatus );
    mVRRStatusChange = true;
    if (mIsLoadEdidWithPort) {
        for (int i=HDMI_PORT_1;i<HDMI_PORT_MAX;i++) {
            mHDMIRxManager.HdmiRxEdidUpdateWithPort(i);
        }
    } else {
        mHDMIRxManager.HdmiRxEdidUpdate();
    }
    return ret;
}

int CTv::Tv_SetWssStatus (int status)
{
    return mpTvin->VDIN_SetWssStatus(status);
}

int CTv::Tv_Easupdate()
{
    int ret = 0;
    if (mTvEas->mEasScanHandle != NULL) {
        ret = mTvEas->StopEasUpdate();
        if (ret < 0) {
            LOGD("StopEasUpdate error!\n");
            return ret;
        }
    }

    return mTvEas->StartEasUpdate();
}

int CTv::Tv_SetDeviceIdForCec(int deviceId)
{
    //send source switch event for cec while HDMI source
    TvEvent::SourceSwitchEvent ev;
    ev.DestSourceInput = deviceId;
    sendTvEvent(ev);
    return 0;
}
//audio

void CTv::updateSubtitle(int pic_width, int pic_height)
{
    TvEvent::SubtitleEvent ev;
    ev.pic_width = pic_width;
    ev.pic_height = pic_height;
    sendTvEvent(ev);
}

int CTv::SendCmdToOffBoardFBCExternalDac(int cmd __unused, int para __unused)
{
    /*int set_val = 0;
    CFbcCommunication *pFBC = GetSingletonFBC();
    if (pFBC != NULL) {
        if (cmd == AUDIO_CMD_SET_MUTE) {
            if (para == CC_AUDIO_MUTE) {
                set_val = CC_MUTE_ON;
            } else if (para == CC_AUDIO_UNMUTE) {
                set_val = CC_MUTE_OFF;
            } else {
                return -1;
            }

            return pFBC->cfbc_Set_Mute(COMM_DEV_SERIAL, set_val);
        } else if (cmd == AUDIO_CMD_SET_VOLUME_BAR) {
            LOGD("%s, send AUDIO_CMD_SET_VOLUME_BAR (para = %d) to fbc.\n", __FUNCTION__, para);
            return pFBC->cfbc_Set_Volume_Bar(COMM_DEV_SERIAL, para);
        } else if (cmd == AUDIO_CMD_SET_BALANCE) {
            LOGD("%s, send AUDIO_CMD_SET_BALANCE (para = %d) to fbc.\n", __FUNCTION__, para);
            return pFBC->cfbc_Set_Balance(COMM_DEV_SERIAL, para);
        } else if (cmd == AUDIO_CMD_SET_SOURCE) {
            LOGD("%s, send AUDIO_CMD_SET_SOURCE (para = %d) to fbc.\n", __FUNCTION__, para);
            return pFBC->cfbc_Set_FBC_Audio_Source(COMM_DEV_SERIAL, para);
        }
    }*/
    return 0;
}

int CTv::GetHdmiAvHotplugDetectOnoff()
{
    const char *value = config_get_str ( CFG_SECTION_TV, CFG_SSM_HDMI_AV_DETECT, "null" );
    if ( strtoul(value, NULL, 10) == 1 ) {
        return 1;
    }

    return 0;
}

int CTv::LoadEdidData(int isNeedBlackScreen, int isDolbyVisionEnable, int isDlgVisionEnable)
{
    AutoMutex _l( mLock );
    LOGD("%s:isNeedBlackScreen [%d],isDolbyVisionEnable [%d],isDlgVisionEnable [%d]!\n",
         __FUNCTION__,isNeedBlackScreen,isDolbyVisionEnable,isDlgVisionEnable);
    int ret = -1;

    if (isNeedBlackScreen  == 1) {
        mpTvin->Tvin_StopDecoder();
    }

    bool isLoadDvEdid = IsNeedLoadDolbyVisionEdid(isDolbyVisionEnable);
    if (mIsLoadEdidWithPort) {
        LOGD("%s:Load edid with port!\n", __FUNCTION__);
        ret = SSMLoadHDMIEdidDataWithPort(isLoadDvEdid, isDlgVisionEnable);
    } else {
        LOGD("%s:Load edid with hdmirx0/edid note!\n", __FUNCTION__);
        ret = SSMLoadHDMIEdidData(isLoadDvEdid, isDlgVisionEnable);
    }

    if (ret == 0) {
        if (mInitEdidVersion) {//open tv set edid version and vrr status
            LOGD("%s:init edid version & VRR status!\n", __FUNCTION__);
            char tmp[32] = {0};
            int EdidVersion = 0;
            for (int i=HDMI_PORT_1;i<HDMI_PORT_MAX;i++) {
                EdidVersion |= (SSMReadHDMIEdidVersion((tv_hdmi_port_id_t)i)<<(4*i-4));
            }
            sprintf(tmp, "%x", EdidVersion);
            SSMSetHDMIEdidVersion(tmp);
            mInitEdidVersion = false;
            //init vrr false;
            tvWriteSysfs(VRR_FUNC_CTRL_PATH, 0 );
            mVrrStatus = false;
        }
        if (mIsLoadEdidWithPort) {
            for (int i=HDMI_PORT_1;i<HDMI_PORT_MAX;i++) {
                mHDMIRxManager.HdmiRxEdidUpdateWithPort(i);
            }
        } else {
            mHDMIRxManager.HdmiRxEdidUpdate();
        }
    } else {
        LOGE("%s failed!\n", __FUNCTION__);
    }

    return ret;
}

int CTv::UpdateEdidData(tv_source_input_t source_input, unsigned char *dataBuf)
{
    AutoMutex _l(mLock);
    //LOGD("%s: source is %d.\n", __FUNCTION__);
    LOGD("%s: source is %d.\n", __FUNCTION__, source_input);
    int ret = -1;
    if (dataBuf == NULL) {
        LOGE("%s: dataBuf is NULL.\n", __FUNCTION__);
    } else {
        tv_hdmi_port_id_t portId = mpTvin->Tvin_GetHdmiPortIDBySourceInput(source_input);
        ret = UpdataEdidDataWithPort(portId, dataBuf);
    }

    return ret;
}

int CTv::SetHdmiEdidVersion(tv_hdmi_port_id_t port, tv_hdmi_edid_version_t version)
{
    AutoMutex _l( mLock );

    LOGD("%s: port: %d, curent source: %d, setVersion: %d.\n", __FUNCTION__, port, m_source_input, version);

    if (m_source_input == (port + 4) ) {
        tv_hdmi_edid_version_t currentVersion = SSMReadHDMIEdidVersion(port);
        if (currentVersion != version) {
            ScreenColorControl(false, VIDEO_LAYER_COLOR_SHOW_ALWAYES);
            mpTvin->Tvin_StopDecoder();

            char tmp[32] = {0};
            int setValue = 0;
            for (int i=HDMI_PORT_1;i<HDMI_PORT_MAX;i++) {
                setValue |= (SSMReadHDMIEdidVersion((tv_hdmi_port_id_t)i)<<(4*i-4));
            }
            setValue &=  (~(0xf << (4*port - 4)));
            setValue |=  (version << (4*port - 4));
            LOGD("%s: new all edid version: 0x%x\n", __FUNCTION__, setValue);
            sprintf(tmp, "%x", setValue);
            setBootEnv(UBOOTENV_EDID_VERSION, tmp);
            SSMSetHDMIEdidVersion(tmp);

            //when change HDMI2.0 ,also change VRR status
            if (TV_GetVRREnable()) {
                vpp_picture_mode_t CurrentPQmode = tvGetPQMode();
                if ( VPP_PICTURE_MODE_GAME ==CurrentPQmode) {
                    if ((0 == version) && mVrrStatus) {
                        tvWriteSysfs(VRR_FUNC_CTRL_PATH, 0 );
                        mVrrStatus = false;
                       LOGD("%s: update VRR status:[%d]\n", __FUNCTION__, mVrrStatus);
                    } else if ((1 == version) && !mVrrStatus) {
                        tvWriteSysfs(VRR_FUNC_CTRL_PATH, 1 );
                        mVrrStatus = true;
                        LOGD("%s: update VRR status:[%d]\n", __FUNCTION__, mVrrStatus);
                    } else {
                        LOGD("%s: needn't update VRR", __FUNCTION__);
                    }
                }
            }

            if (mIsLoadEdidWithPort) {
                mHDMIRxManager.HdmiRxEdidUpdateWithPort(port);
            } else {
                mHDMIRxManager.HdmiRxEdidUpdate();
            }
        } else {
            LOGD("%s: same EDID version, no need set.\n", __FUNCTION__);
        }
    } else {
        LOGD("%s: not curent source, don't set.\n", __FUNCTION__);
    }

    return 0;
}

int CTv::GetHdmiEdidVersion(tv_hdmi_port_id_t port)
{
    AutoMutex _l( mLock );

    int value = SSMReadHDMIEdidVersion(port);
    LOGD("%s: port = %d, version = %d\n", __FUNCTION__, port, value);
    return value;
}

int CTv::SaveHdmiEdidVersion(tv_hdmi_port_id_t port, tv_hdmi_edid_version_t version)
{
    AutoMutex _l( mLock );

    LOGD("%s: port = %d, version = %d\n", __FUNCTION__, port, version);
    return SSMSaveHDMIEdidVersion(port, version);
}

void CTv::SetHdmiEdidForUboot(void)
{
    char tmp[32] = {0};
    //set edid file path
    setBootEnv(UBOOTENV_EDID14_PATH, TV_CONFIG_EDID14_FILE_PATH);
    setBootEnv(UBOOTENV_EDID20_PATH, TV_CONFIG_EDID20_FILE_PATH);

    //set hdmi port map
    int portmap = mHDMIRxManager.CalHdmiPortCecPhysicAddr();
    sprintf(tmp, "%x", portmap);
    setBootEnv(UBOOTENV_PORT_MAP, tmp);
    //set edid version
    memset(tmp, 0x0, sizeof(tmp));
    int EdidVersion = 0;
    for (int i=HDMI_PORT_1;i<HDMI_PORT_MAX;i++) {
        EdidVersion |= (SSMReadHDMIEdidVersion((tv_hdmi_port_id_t)i)<<(4*i-4));
        sprintf(tmp, "%x", EdidVersion);
    }
    setBootEnv(UBOOTENV_EDID_VERSION, tmp);
}

int CTv::SetHdmiHDCPSwitcher(tv_hdmi_hdcpkey_enable_t enable)
{
    mHDMIRxManager.HdmiRxHdcpOnOff(enable);
    return 0;
}

int CTv::SetHdmiColorRangeMode(tvin_color_range_t range_mode)
{
    LOGD("%s, mode: %d!\n", __FUNCTION__, range_mode);
    tv_source_input_type_t sourceType = mpTvin->Tvin_SourceInputToSourceInputType(m_source_input);
    int ret = -1;
    if (sourceType == SOURCE_TYPE_HDMI) {//HDMI source
        ret = mpTvin->VDIN_SetColorRangeMode(range_mode);
    } else {//AV or ATV source
        ret = mpTvin->VDIN_SetColorRangeMode(TVIN_COLOR_RANGE_LIMIT);
    }
    if (ret >= 0) {
        SSMSaveHDMIColorRangeMode(range_mode);
    }

    return ret;
}

int CTv::GetHdmiColorRangeMode()
{
    int mode = 0;
    SSMReadHDMIColorRangeMode(&mode);
    if ((mode > TVIN_COLOR_RANGE_LIMIT) || (mode < TVIN_COLOR_RANGE_AUTO)) {
        LOGE("%s error!\n", __FUNCTION__);
        mode = TVIN_COLOR_RANGE_AUTO;
    }
    LOGD("%s: mode = %d!\n", __FUNCTION__, mode);
    return mode;
}

tvin_port_t CTv::Tv_GetHdmiPortBySourceInput(tv_source_input_t source_input)
{
    tv_source_input_type_t source_type = mpTvin->Tvin_SourceInputToSourceInputType(source_input);
    if (source_type == SOURCE_TYPE_HDMI) {
        return mpTvin->Tvin_GetSourcePortBySourceInput(source_input);
    } else {
        LOGE("Not HDMI source!\n");
        return TVIN_PORT_NULL;
    }
}

int CTv::SetVideoAxis(int x, int y, int width, int height)
{
    return mAv.setVideoAxis(x, y, width, height);
}

int CTv::handleGPIO(const char *port_name, bool is_out, int edge)
{
    return pGpio->processCommand(port_name, is_out, edge);
}

int CTv::KillMediaServerClient()
{
    char buf[PROPERTY_VALUE_MAX] = { 0 };
    int len = property_get("media.player.pid", buf, "");
    if (len > 0) {
        char* end;
        int pid = strtol(buf, &end, 0);
        if (end != buf) {
            LOGD("[ctv] %s, remove video path, but video decoder has used, kill it:%d\n",
                __FUNCTION__, pid);
            kill(pid, SIGKILL);
            property_set("media.player.pid", "");
        }
    }
    return 0;
}

int CTv::setPictureModeBySignal(pq_mode_switch_type_t switchType)
{
    int ret = -1;
    bool automonitor = false;
    mAllmInfo = false;
    if (!MnoNeedAutoSwitchToMonitorMode) {
        LOGD("%s, PQ mode set by signal!\n", __FUNCTION__);
        tvin_latency_t allmInfo;
        memset(&allmInfo, 0x0, sizeof(tvin_latency_t));
        mpTvin->VDIN_GetAllmInfo(&allmInfo);
        LOGD("%s: allm_mode is %d, it_content is %d, cn_type is %d!\n", __FUNCTION__, allmInfo.allm_mode,
                                   allmInfo.it_content, allmInfo.cn_type);

        vpp_picture_mode_t NewPictureMode = VPP_PICTURE_MODE_MAX;

        if (mAllmModeCfg && allmInfo.allm_mode) {//allm mode
            LOGD("%s: allm mode, autoswitch to game mode!\n", __FUNCTION__);
            NewPictureMode = VPP_PICTURE_MODE_GAME;
            mAllmInfo = true;
        } else if (mItcontentModeCfg && allmInfo.it_content) {//itcontent
            if (allmInfo.cn_type == GAME) {
                LOGD("%s: game fmt, autoswitch to game mode!\n", __FUNCTION__);
                NewPictureMode = VPP_PICTURE_MODE_GAME;
                mAllmInfo = true;
            } else {//Graphics/photo/cinema
                LOGD("%s:vesa fmt, autoswitch to monitor mode!\n", __FUNCTION__);
                NewPictureMode = VPP_PICTURE_MODE_MONITOR;
                automonitor = true;
            }
        } else if (mDviModeCfg && m_cur_sig_info.is_dvi) {//dvi
            LOGD("%s: DVI fmt, autoswitch to monitor mode!\n", __FUNCTION__);
            NewPictureMode = VPP_PICTURE_MODE_MONITOR;
            automonitor = true;
        } else {
            if (mAllmModeCfg || mItcontentModeCfg || mDviModeCfg) {
                LOGD("%s: signal don't match any auto switch condition!\n", __FUNCTION__);
                NewPictureMode = tvGetPQMode();
                if ((mAllmModeCfg && (NewPictureMode == VPP_PICTURE_MODE_GAME))
                    || (mItcontentModeCfg && ((NewPictureMode == VPP_PICTURE_MODE_GAME) || (NewPictureMode == VPP_PICTURE_MODE_MONITOR)))
                    || (mDviModeCfg && (NewPictureMode == VPP_PICTURE_MODE_MONITOR))) {
                    //the standard mode is not automatically switched
                    //NewPictureMode = VPP_PICTURE_MODE_MAX;
                    //if want automatic change to standard mode ,use this
                    NewPictureMode = tvGetLastPQMode();
                    automonitor = true;
                    LOGD("%s: last PQ mode :[%d]\n", __FUNCTION__,NewPictureMode);
                } else {
                    NewPictureMode = VPP_PICTURE_MODE_MAX;
                }
            } else {
                LOGD("%s: all signal check disabled!\n", __FUNCTION__);
                NewPictureMode = VPP_PICTURE_MODE_MAX;
            }
        }

        //when VRR switch is open,can't allow auto set monitor mode
        if (TV_GetVRREnable()) {
            if ( automonitor ) {
                LOGD("%s, VRR switch opened, can't auto set  mode!\n", __FUNCTION__);
                return 0;
            } else if ( (NewPictureMode == VPP_PICTURE_MODE_GAME) && (tvGetPQMode() == VPP_PICTURE_MODE_GAME) ) {
                mAllmInfo = false;
                LOGD("%s, already is game needn't set game!\n", __FUNCTION__);
                return 0;
            } else {
                LOGD("%s, non auto monitor!\n", __FUNCTION__);
            }
        } else if (getVrrMode() != 0) {
             LOGD("%s, auto vrr, notify PQ game!\n", __FUNCTION__);
            NewPictureMode = VPP_PICTURE_MODE_GAME;
        }

        if (NewPictureMode != VPP_PICTURE_MODE_MAX) {
            ret = CVpp::getInstance()->SetPQMode(NewPictureMode, m_source_input, m_cur_sig_info.fmt,
                                                 m_cur_sig_info.trans_fmt, INDEX_2D, 1, switchType);
            //set audio pqmode for game && reset for other pqmode
            if (NewPictureMode == VPP_PICTURE_MODE_GAME) {
                tvsetAudioParam(31,1,0,0);
            } else {
                tvsetAudioParam(31,0,0,0);
            }
        }
    }else {
        LOGD("%s, PQ mode set by user!\n", __FUNCTION__);
        ret = 0;
    }

    return ret;
}


void CTv::onVdinSignalChange()
{
    AutoMutex _l( mLock );
    LOGD("%s: mTvAction = 0x%x, m_source_input = %d\n", __FUNCTION__, mTvAction, m_source_input);
    if (!((mTvAction & TV_ACTION_IN_VDIN) || (mTvAction & TV_ACTION_SCANNING)) || (SOURCE_SPDIF == m_source_input)) {
        return;
    }

    vdin_event_info_s SignalEventInfo;
    memset(&SignalEventInfo, 0, sizeof(vdin_event_info_s));
    int ret  = mpTvin->VDIN_GetSignalEventInfo(&SignalEventInfo);
    if (ret < 0) {
        LOGD("Get vdin event error!\n");
    } else {
        tv_source_input_type_t source_type = CTvin::Tvin_SourceInputToSourceInputType(m_source_input);
        tvin_sig_change_flag_t vdinEventType = (tvin_sig_change_flag_t)SignalEventInfo.event_sts;
        switch (vdinEventType) {
        case TVIN_SIG_CHG_SDR2HDR:
        case TVIN_SIG_CHG_HDR2SDR:
        case TVIN_SIG_CHG_DV2NO:
        case TVIN_SIG_CHG_NO2DV:
            LOGD("%s: hdr info change!\n", __FUNCTION__);
            tvin_info_t vdinSignalInfo;
            memset(&vdinSignalInfo, 0, sizeof(tvin_info_t));
            ret = mpTvin->VDIN_GetSignalInfo(&vdinSignalInfo);
            if (ret < 0) {
                LOGD("%s: Get vdin event error!\n", __FUNCTION__);
            } else {
                if ((m_cur_sig_info.status == TVIN_SIG_STATUS_STABLE) && (m_cur_sig_info.hdr_info != vdinSignalInfo.hdr_info)) {
                    if (source_type == SOURCE_TYPE_HDMI) {
                        tvSetCurrentHdrInfo(vdinSignalInfo.hdr_info);
                    }
                    m_cur_sig_info.hdr_info = vdinSignalInfo.hdr_info;
                } else {
                    LOGD("%s: hdmi signal don't stable!\n", __FUNCTION__);
                }
            }
            break;
        case TVIN_SIG_CHG_COLOR_FMT:
            LOGD("%s: no need do any thing for colorFmt change!\n", __FUNCTION__);
            break;
        case TVIN_SIG_CHG_RANGE:
            LOGD("%s: no need do any thing for colorRange change!\n", __FUNCTION__);
            break;
        case TVIN_SIG_CHG_BIT:
            LOGD("%s: no need do any thing for color bit depth change!\n", __FUNCTION__);
            break;
        case TVIN_SIG_CHG_VS_FRQ:
            LOGD("%s: FPS info change!\n", __FUNCTION__);
            if (source_type == SOURCE_TYPE_HDMI) {
                tvin_info_t frqSignalInfo;
                memset(&frqSignalInfo, 0, sizeof(tvin_info_t));
                int temp = mpTvin->VDIN_GetSignalInfo(&frqSignalInfo);
                if (temp < 0) {
                    LOGD("%s: Get Signal Info error!\n", __FUNCTION__);
                } else if ((frqSignalInfo.status == TVIN_SIG_STATUS_STABLE)
                    && (m_cur_sig_info.fps != frqSignalInfo.fps)) {
                    LOGD("%s: m_cur_sig_info.fps is %d, frqSignalInfo.fps is %d !\n", __FUNCTION__,
                    m_cur_sig_info.fps, frqSignalInfo.fps);
                    if ( (100 == frqSignalInfo.fps) || (120 == frqSignalInfo.fps) ) {
                        LOGD("%s: send fps change event!\n", __FUNCTION__);
                        m_cur_sig_info.fps = frqSignalInfo.fps;
                        TvEvent::SignalInfoEvent ev;
                        ev.mTrans_fmt = m_cur_sig_info.trans_fmt;
                        ev.mFmt = m_cur_sig_info.fmt;
                        ev.mStatus = m_cur_sig_info.status;
                        ev.mReserved = m_cur_sig_info.fps;
                        sendTvEvent ( ev );
                    }
                }
            } else {
                    LOGD("%s: no need do any thing for VS_FRQ change!\n", __FUNCTION__);
            }
            break;
        case TVIN_SIG_CHG_STS:
            LOGD("%s: vdin signal status change!\n", __FUNCTION__);
            onSigStatusChange();
            break;
        case TVIN_SIG_CHG_AFD: {
            LOGD("%s: AFD info change!\n", __FUNCTION__);
            if (source_type == SOURCE_TYPE_HDMI) {
                tvin_info_t newSignalInfo;
                memset(&newSignalInfo, 0, sizeof(tvin_info_t));
                int ret = mpTvin->VDIN_GetSignalInfo(&newSignalInfo);
                if (ret < 0) {
                    LOGD("%s: Get Signal Info error!\n", __FUNCTION__);
                } else {
                    if ((newSignalInfo.status == TVIN_SIG_STATUS_STABLE)
                        && (m_cur_sig_info.aspect_ratio != newSignalInfo.aspect_ratio)) {
                        m_cur_sig_info.aspect_ratio = newSignalInfo.aspect_ratio;
                        tvSetCurrentAspectRatioInfo(newSignalInfo.aspect_ratio);
                    } else {
                        LOGD("%s: signal not stable or same AFD info!\n", __FUNCTION__);
                    }
                }
            }
            break;
        }
        case TVIN_SIG_CHG_DV_ALLM:
            LOGD("%s: allm info change!\n", __FUNCTION__);
            if (source_type == SOURCE_TYPE_HDMI) {
                setPictureModeBySignal(PQ_MODE_SWITCH_TYPE_AUTO);
            } else {
                LOGD("%s: not hdmi source!\n", __FUNCTION__);
            }
            break;
        case TVIN_SIG_CHG_VRR:
            LOGD("%s: VRR change!\n", __FUNCTION__);
            setPictureModeBySignal(PQ_MODE_SWITCH_TYPE_AUTO);
            break;
        case TVIN_SIG_CHG_QMS: {
            LOGD("%s: QMS change!\n", __FUNCTION__);
            vdin_qms_param_t qmsInfo;
            memset(&qmsInfo, 0, sizeof(vdin_qms_param_t));
            int ret = mpTvin->VDIN_GetQmsParm(&qmsInfo);
            if (ret < 0) {
                LOGD("%s: Get QMS Info error!\n", __FUNCTION__);
            } else {
                LOGD("%s: send QMS change event, new fps：%d!\n", __FUNCTION__, qmsInfo.qms_fr);
                TvEvent::QMSEvent ev;
                ev.qms_en = (int)qmsInfo.qms_en;
                ev.qms_fps = (int)qmsInfo.qms_fr;
                ev.qms_base_fps = (int)qmsInfo.qms_base_fr;
                sendTvEvent ( ev );
            }
            break;
        }
        default:
            LOGD("%s: invalid vdin event!\n", __FUNCTION__);
            break;
        }
    }
}

void CTv::onVdin2SignalChange() {

    LOGD("%s: source = %d \n", __FUNCTION__, m_pip_source_input);
    int ret = mpTvin->VDIN2_GetSignalInfo ( &m_cur_vdin2_sig_info );
    if (ret < 0) {
        LOGD("Get vdin2 Signal Info error!\n");
        m_cur_vdin2_sig_info.status = TVIN_SIG_STATUS_NULL;
    }

    LOGD("%s: trans_fmt is %d, sig_fmt is %d, status is %d, isDVI is %d, hdr_info is 0x%x\n", __FUNCTION__,
    m_cur_vdin2_sig_info.trans_fmt, m_cur_vdin2_sig_info.fmt, m_cur_vdin2_sig_info.status, m_cur_vdin2_sig_info.is_dvi, m_cur_vdin2_sig_info.hdr_info);
    if ( m_cur_vdin2_sig_info.status == TVIN_SIG_STATUS_STABLE ) {
        ret = mpTvin->Tvin2_StartDecoder(m_cur_vdin2_sig_info);

        TvEvent::SignalInfoEvent ev;
        ev.mTrans_fmt = m_cur_vdin2_sig_info.trans_fmt;
        ev.mFmt = m_cur_vdin2_sig_info.fmt;
        ev.mStatus = m_cur_vdin2_sig_info.status;
        ev.mIsPiP = TV_SOURCE_PIP;
        sendTvEvent ( ev );
    } else if ( m_cur_vdin2_sig_info.status == TVIN_SIG_STATUS_UNSTABLE ) {
        ret = mpTvin->Tvin2_StopDecoder();

    } else if ( m_cur_vdin2_sig_info.status == TVIN_SIG_STATUS_NOTSUP ) {
        ret = mpTvin->Tvin2_StopDecoder();

    } else if ( m_cur_vdin2_sig_info.status == TVIN_SIG_STATUS_NOSIG ) {
        ret = mpTvin->Tvin2_StopDecoder();

        TvEvent::SignalInfoEvent ev;
        ev.mTrans_fmt = m_cur_vdin2_sig_info.trans_fmt;
        ev.mFmt = m_cur_vdin2_sig_info.fmt;
        ev.mStatus = m_cur_vdin2_sig_info.status;
        ev.mIsPiP = TV_SOURCE_PIP;
        sendTvEvent ( ev );
    } else {
        InitCurrentVdin2SignalInfo();
    }

}

CTvRecord *CTv::getRecorder(const char *id, const char *param) {
    CTvRecord *recorder = RecorderManager::getInstance().getDev(id);
    if (recorder) {
        recorder->stop(NULL);
        recorder->setupDefault(param);
        return recorder;
    }
    recorder = new CTvRecord();
    recorder->setupDefault(param);
    recorder->setId(id);
    int err = RecorderManager::getInstance().addDev(*recorder);
    if (err) {
        LOGE("create Recorder(%s) fail(%d)", toReadable(id), err);
        delete recorder;
        recorder = NULL;
    }
    return recorder;
}

int CTv::prepareRecording(const char *id, const char *param)
{
    //validate param
    if (!getRecorder(id, param))
        return -1;
    return 0;
}

int CTv::startRecording(const char *id, const char *param, CTvRecord::IObserver *observer)
{
    CTvRecord *recorder = NULL;
    int ret = -1;
    if (!(recorder = getRecorder(id, param))) {
        LOGD("recorder(%s) not found", toReadable(id));
        return -1;
    }
    recorder->setObserver(observer);
    ret = recorder->start(param);
    if (ret != 0)
        RecorderManager::getInstance().removeDev(id);
    return ret;
}

int CTv::stopRecording(const char *id, const char *param)
{
    CTvRecord *recorder = NULL;
    if (!(recorder = RecorderManager::getInstance().getDev(id))) {
        LOGD("recorder(%s) not found", toReadable(id));
        return -1;
    }
    recorder->stop(param);
    recorder->setObserver(NULL);
    RecorderManager::getInstance().removeDev(id);
    return 0;
}

int CTv::doRecordingCommand(int cmd, const char *id, const char *param)
{
    LOGD("doRec(cmd:%d, id:%s, para:%s)", cmd, id, param);
    AutoMutex _l(mLock);
    int ret = -10;
    setDvbLogLevel();
    switch (cmd) {
        case RECORDING_CMD_PREPARE:
        ret = prepareRecording(id, param);
        break;
        case RECORDING_CMD_START:
        ret = startRecording(id, param, mTvMsgQueue.get());
        break;
        case RECORDING_CMD_STOP:
        ret = stopRecording(id, param);
        break;
    }
    return ret;
}

CTvPlayer *CTv::getPlayer(const char *id, const char *param) {
    CTvPlayer *player = PlayerManager::getInstance().getDev(id);
    if (player) {
        player->setupDefault(param);
        player->stop(param);
        return player;
    }

    if (PlayerManager::singleMode)
        PlayerManager::getInstance().releaseAll();


    std::string type = paramGetString(param, NULL, "type", "dtv");
    LOGE("get Player fail(%s)", param);
    if (type.compare("dtv") == 0)
        player = new CDTVTvPlayer(this);
    else if (type.compare("atv") == 0)
        player = new CATVTvPlayer(this);

    if (player) {
        player->setupDefault(param);
        player->setId(id);
        int err = PlayerManager::getInstance().addDev(*player);
        if (err) {
            LOGE("create Player(%s) fail(%d)", toReadable(id), err);
            delete player;
            player = NULL;
        }
    }
    return player;
}

int CTv::startPlay(const char *id, const char *param)
{
    LOGD("%s\n", __FUNCTION__);
    int ret = -1;
    CTvPlayer *player = NULL;
    if (!(player = getPlayer(id, param))) {
        LOGD("player(%s) not found", toReadable(id));
        return -1;
    }
    ret =  player->start(param);
    if (ret != 0)
        RecorderManager::getInstance().removeDev(id);
    return ret;
}

int CTv::stopPlay(const char *id, const char *param)
{
    LOGD("%s\n", __FUNCTION__);
    CTvPlayer *player = NULL;
    if (!(player = PlayerManager::getInstance().getDev(id))) {
        LOGD("player(%s) not found", toReadable(id));
        return -1;
    }
    player->stop(param);
    PlayerManager::getInstance().removeDev(id);
    return 0;
}

int CTv::pausePlay(const char *id, const char *param)
{
    CTvPlayer *player = NULL;
    if (!(player = PlayerManager::getInstance().getDev(id))) {
        LOGD("player(%s) not found", toReadable(id));
        return -1;
    }
    return player->pause(param);
}

int CTv::resumePlay(const char *id, const char *param)
{
    CTvPlayer *player = NULL;
    if (!(player = PlayerManager::getInstance().getDev(id))) {
        LOGD("player(%s) not found", toReadable(id));
        return -1;
    }
    return player->resume(param);
}

int CTv::seekPlay(const char *id, const char *param)
{
    CTvPlayer *player = NULL;
    if (!(player = PlayerManager::getInstance().getDev(id))) {
        LOGD("player(%s) not found", toReadable(id));
        return -1;
    }
    return player->seek(param);
}

int CTv::setPlayParam(const char *id, const char *param)
{
    CTvPlayer *player = NULL;
    if (!(player = PlayerManager::getInstance().getDev(id))) {
        LOGD("player(%s) not found", toReadable(id));
        return -1;
    }
    return player->set(param);
}

int CTv::doPlayCommand(int cmd, const char *id, const char *param)
{
    LOGD("%s: cmd:%d, id:%s, para:%s\n", __FUNCTION__, cmd, id, param);
    AutoMutex _l(mLock);
    int ret = -10;
    setDvbLogLevel();
    switch (cmd) {
        case PLAY_CMD_START:
        ret = startPlay(id, param);
        break;
        case PLAY_CMD_STOP:
        ret = stopPlay(id, param);
        break;
        case PLAY_CMD_PAUSE:
        ret = pausePlay(id, param);
        break;
        case PLAY_CMD_RESUME:
        ret = resumePlay(id, param);
        break;
        case PLAY_CMD_SEEK:
        ret = seekPlay(id, param);
        break;
        case PLAY_CMD_SETPARAM:
        ret = setPlayParam(id, param);
        break;
    }
    return ret;
}

ANDROID_SINGLETON_STATIC_INSTANCE(RecorderManager);
ANDROID_SINGLETON_STATIC_INSTANCE(PlayerManager);

int CTv::Tv_RrtUpdate()
{
    int ret = 0;
#ifdef SUPPORT_ADTV
    if (mTvRrt->mRrtScanHandle != NULL) {
        ret = mTvRrt->StopRrtUpdate();
        if (ret < 0) {
            LOGD("StopRrtUpdate error!\n");
            return ret;
        }
    }

    return mTvRrt->StartRrtUpdate();
#else
    return ret;
#endif
}

rrt_select_info_t CTv::Tv_RrtSearch(int rating_region_id, int dimension_id, int value_id, int program_id)
{
    rrt_select_info_t tmp;
    memset(&tmp,0,sizeof(tmp));
#if 0
    memset(&tmp, 0, sizeof(rrt_select_info_t));
#ifdef SUPPORT_ADTV

    int ret = mTvRrt->GetRRTRating(rating_region_id, dimension_id, value_id, program_id, &tmp);
    if (ret < 0) {
        LOGD("Tv_RrtSearch error!\n");
    }

#endif
#endif
    return tmp;
}

void CTv::setDvbLogLevel() {
#ifdef SUPPORT_ADTV
    AM_DebugSetLogLevel(property_get_int32("persist.vendor.tv.dvb.loglevel", 1));
#endif
}

void CTv::dump(String8 &result)
{
    result.appendFormat("\naction = %x\n", mTvAction);
    result.appendFormat("status = %d\n", mTvStatus);
    result.appendFormat("current source input = %d\n", m_source_input);
    result.appendFormat("last source input = %d\n\n", m_last_source_input);

    result.appendFormat("tvserver git branch:%s\n", tvservice_get_git_branch_info());
    result.appendFormat("tvserver git version:%s\n", tvservice_get_git_version_info());
    result.appendFormat("tvserver Last Changed:%s\n", tvservice_get_last_chaned_time_info());
    result.appendFormat("tvserver Last Build:%s\n", tvservice_get_build_time_info());
    result.appendFormat("tvserver Builder Name:%s\n", tvservice_get_build_name_info());
    result.appendFormat("tvserver board version:%s\n", tvservice_get_board_version_info());
    result.appendFormat("linux kernel version:%s\n", tvservice_get_kernel_version_info());

#ifdef SUPPORT_ADTV
    result.appendFormat("libdvb git branch:%s\n", dvb_get_git_branch_info());
    result.appendFormat("libdvb git version:%s\n", dvb_get_git_version_info());
    result.appendFormat("libdvb Last Changed:%s\n", dvb_get_last_chaned_time_info());
    result.appendFormat("libdvb Last Build:%s\n", dvb_get_build_time_info());
    result.appendFormat("libdvb Builder Name:%s\n\n", dvb_get_build_name_info());
#endif
}

int CTv::SetAtvAudioOutmode(int mode)
{
#ifdef SUPPORT_ADTV
    return mFrontDev->setVLPropLocked(V4L2_SOUND_SYS, mode);
#else
    return -1;
#endif
}

int CTv::GetAtvAudioOutmode()
{
    int mode = 0;
#ifdef SUPPORT_ADTV
    mFrontDev->getVLPropLocked(V4L2_SOUND_SYS, &mode);
#endif
    return (mode & 0xFF);
}

int CTv::GetAtvAudioInputmode()
{
    int mode = 0;
#ifdef SUPPORT_ADTV
    mFrontDev->getVLPropLocked(V4L2_SOUND_SYS, &mode);
#endif
    /*
     * bit24-16: audio std.
     * bit15-8: audio input type.
     * bit7-0: audio output mode.
     */
    return (mode >> 8 & 0xFFFF);
}

int CTv::GetAtvAutoScanMode()
{
    int mode = -1;
    const char *config_value  = config_get_str(CFG_SECTION_ATV, CFG_ATV_AUTO_SCAN_MODE, "0");
    mode = strtoul(config_value, NULL, 10);

    LOGD("%s: %d", "atv.scan.auto.mode", mode);

    /* @mode
     * 0: use freq table list mode to scan.
     * 1: use step mode to auto scan.
     */
    return mode;
}

int CTv::SetSnowShowEnable(bool enable)
{
    if (enable) {
        mLastScreenMode = mAv.getVideoScreenMode();
        LOGD("%s: Get LastScreenMode = %d", __FUNCTION__, mLastScreenMode);
        mAv.setVideoScreenMode(1);//while show snow,need show full screen
    } else {
        LOGD("%s: Set LastScreenMode = %d", __FUNCTION__, mLastScreenMode);
        mAv.setVideoScreenMode(mLastScreenMode);
    }

    return mpTvin->SwitchSnow(enable);
}

int CTv::TV_SetSameSourceEnable(bool enable)
{
    mbSameSourceEnableStatus = enable;
    return 0;
}

void CTv::SetBlocked(bool block, bool tunning)
{
    if (mSupportChannelLock) {
        if (block) {
            mBlockState = BLOCK_STATE_BLOCKED;
        } else {
            mBlockState = BLOCK_STATE_NONE;
        }
        if (!tunning && (mTvAction & TV_ACTION_PLAYING)) {
            onBlockStateChanged(mBlockState);
        }
    }
}

int CTv::RequestUnblock()
{
    if (mSupportChannelLock
        && mEnableLockModule
        && mBlockState == BLOCK_STATE_BLOCKED) {
        mBlockState = BLOCK_STATE_UNBLOCKED;
        onBlockStateChanged(mBlockState);
    }
    return 0;
}

void CTv::onBlockStateChanged(tv_block_state_e state)
{
    LOGD("onBlockStateChanged: enable: %d, state: %d, current sig status: %d",
        mEnableLockModule, state, m_cur_sig_info.status);
    if (m_cur_sig_info.status > TVIN_SIG_STATUS_NOSIG && mEnableLockModule) {
        SendBlockEvt(state == BLOCK_STATE_BLOCKED);
        if (state == BLOCK_STATE_BLOCKED) {
             CVpp::getInstance()->VPP_setVideoColor(true);
        } else {
             CVpp::getInstance()->VPP_setVideoColor(false);
        }
    }
}

void CTv::SendBlockEvt(bool blocked)
{
    TvEvent::AVPlaybackEvent AvPlayBackEvt;
    AvPlayBackEvt.mMsgType = blocked ?
        TvEvent::AVPlaybackEvent::EVENT_AV_PLAYER_BLOCKED :
        TvEvent::AVPlaybackEvent::EVENT_AV_PLAYER_UNBLOCK;
    AvPlayBackEvt.mProgramId = 0;
    sendTvEvent(AvPlayBackEvt);
}

bool CTv::isBlockedByChannelLock() {
    return (mSupportChannelLock
        && mEnableLockModule
        && (m_source_input == SOURCE_TV)
        && (mBlockState == BLOCK_STATE_BLOCKED));
}

bool CTv::isChannelBlockStatusChanged()
{
    return mBlockStatusChanged;
}

int CTv::getBasicVdecStatusInfo(int vdecId, int *decode_time_cost, int *frame_width, int *frame_height,
    int *frame_rate, int *error_count, int *frame_count, int *error_frame_count, int *drop_frame_count,
    int *double_write_mode) {
    return getVdecStatusInfo(vdecId, decode_time_cost, frame_width, frame_height, frame_rate, NULL,
    NULL, NULL, error_count, NULL, frame_count, error_frame_count, drop_frame_count, NULL, double_write_mode,
    NULL, NULL, NULL, NULL, NULL, NULL);
}


int CTv::getVdecStatusInfo(int vdecId, int *decode_time_cost, int *frame_width, int *frame_height, int *frame_rate,
    int *bit_depth_luma, int *frame_dur, int *bit_depth_chroma, int *error_count, int *status, int *frame_count,
    int *error_frame_count, int *drop_frame_count, int64_t *total_data, int *double_write_mode, int *offset, int *ratio_control, int *vf_type,
    int *signal_type, int *pts, int64_t *pts_us64) {
    int fd,i,r;
    struct av_param_mvdec_t para;
    char *fn = "/dev/amstream_userdata";//This file should not affect other dev file

    int count = 10;
    AutoMutex _l(mLock);
    fd = open(fn, O_RDONLY);
    if (fd < 0) {
        LOGD("%s, open %s error", __FUNCTION__, fn);
        return -1;
    }

    while (count-- > 0) {
        memset(&para.minfo, 0, 8*sizeof(struct vframe_counter_s));
        /* Prepare the parameter */
        para.vdec_id = vdecId;
        para.struct_size = sizeof(struct av_param_mvdec_t);
        //printf("para.struct_size=%u\n",para.struct_size);

        /* Get the vdec frames information via ioctl */
        r = ioctl(fd, AMSTREAM_IOC_GET_MVDECINFO, &para);
        if (r < 0) {
                LOGD("ioctl error for vdec %d, ret=%d \n", vdecId, r);
                break;
        } else if (para.slots == 0) {
                //The decoder does not provide the info yet, wait sometime
                usleep(100*1000);
                continue;
        }

        /* Print the vdec frames related informations */
        LOGD("got %d frames info, vdec_id=%d\n",para.slots, vdecId);
        if (para.slots > 0) {
                i = para.slots -1; // get the latest frame info
                LOGD("slots is : %d\n", i);
                LOGD("vdec_name : %s\n", para.comm.vdec_name);
                LOGD("vdec_type : %d\n", para.comm.vdec_type);
                LOGD("frame_width : %d\n", para.minfo[i].frame_width);
                LOGD("frame_height : %d\n", para.minfo[i].frame_height);
                LOGD("frame_rate : %d\n", para.minfo[i].frame_rate);
                LOGD("double_write_mode : %d\n", para.minfo[i].double_write_mode);
                LOGD("decode_time_cost : %d\n", para.minfo[i].decode_time_cost);
                LOGD("error_count: %d\n", para.minfo[i].error_count);
                LOGD("frame_count : %d\n", para.minfo[i].frame_count);
                LOGD("frame_error_count : %d\n", para.minfo[i].error_frame_count);
                LOGD("drop_frame : %d\n", para.minfo[i].drop_frame_count);

                if (decode_time_cost != NULL)
                        *decode_time_cost = para.minfo[i].decode_time_cost;
                if (frame_width != NULL)
                        *frame_width = para.minfo[i].frame_width;
                if (frame_height != NULL)
                        *frame_height = para.minfo[i].frame_height;
                if (frame_rate != NULL)
                        *frame_rate = para.minfo[i].frame_rate;
                if (bit_depth_luma != NULL)
                        *bit_depth_luma = para.minfo[i].bit_depth_luma;
                if (frame_dur != NULL)
                        *frame_dur = para.minfo[i].frame_dur;
                if (bit_depth_chroma != NULL)
                        *bit_depth_chroma = para.minfo[i].bit_depth_chroma;
                if (error_count != NULL)
                        *error_count = para.minfo[i].error_count;
                if (status != NULL)
                        *status = para.minfo[i].status;
                if (frame_count != NULL)
                        *frame_count = para.minfo[i].frame_count;
                if (error_count != NULL)
                        *error_count = para.minfo[i].error_count;
                if (error_frame_count != NULL)
                        *error_frame_count = para.minfo[i].error_frame_count;
                if (drop_frame_count != NULL)
                        *drop_frame_count = para.minfo[i].drop_frame_count;
                if (total_data != NULL)
                        *total_data = para.minfo[i].total_data;
                if (double_write_mode != NULL)
                        *double_write_mode = para.minfo[i].double_write_mode;
                if (offset != NULL)
                        *offset = para.minfo[i].offset;
                if (ratio_control != NULL)
                        *ratio_control = para.minfo[i].ratio_control;
                if (vf_type != NULL)
                        *vf_type = para.minfo[i].vf_type;
                if (signal_type != NULL)
                        *signal_type = para.minfo[i].signal_type;
                if (pts != NULL)
                        *pts = para.minfo[i].pts;
                if (pts_us64 != NULL)
                        *pts_us64 = para.minfo[i].pts_us64;

                LOGD("----------------\n");
                close(fd);
                return 1;
        }
    }
    close(fd);
    return 1;
}

std::string CTv::request(const std::string& resource, const std::string& paras)
{
    LOGD("request: %s - %s", resource.c_str(), paras.c_str());
    if (std::string("ADTV.block") == resource) {
        int lock = paramGetInt(paras.c_str(), NULL, "isblocked", 0);
        int tunning = paramGetInt(paras.c_str(), NULL, "tunning", 0);
        SetBlocked(lock, tunning);
        return std::string("{\"ret\":0}");
    } else if (std::string("ADTV.unblockContent") == resource) {
        RequestUnblock();
        return std::string("{\"ret\":0}");
    } else if (std::string("ADTV.isCurrentChannelblocked") == resource) {
        char ret[32];
        snprintf(ret, sizeof(ret), "{\"ret\":0,\"blocked\":%s}",
            isBlockedByChannelLock() ? "true" : "false");
        ret[sizeof(ret) -1] = '\0';
        return std::string(ret);
    } else if (std::string("ADTV.setBlockEn") == resource) {
        bool enable = (paramGetInt(paras.c_str(), NULL, "enable", 0) > 0);
        if (mEnableLockModule != enable) {
            mEnableLockModule = enable;
            SSMSaveChannelLockEnValue(!enable);//0:default-enable, 1;disable
            if (isBlockedByChannelLock()) {
                onBlockStateChanged(mBlockState);
            }
            if (mBlockState == BLOCK_STATE_BLOCKED && !mEnableLockModule) {
                SendBlockEvt(false);
                CVpp::getInstance()->VPP_setVideoColor(false);
            }
        }
        return std::string("{\"ret\":0}");
    } else if (std::string("ADTV.getBlockEn") == resource) {
        char ret[32];
        snprintf(ret, sizeof(ret), "{\"ret\":0,\"enable\":%s}",
            mEnableLockModule ? "true" : "false");
        ret[sizeof(ret) -1] = '\0';
        return std::string(ret);
    } else if (std::string("ADTV.setCurrentChannelBlockStatus") == resource) {
        std::string blocked = paramGetString(paras.c_str(), NULL, "Blocked", "false");
        if (blocked.compare("true") == 0) {
            mChannelBlockState = BLOCK_STATE_BLOCKED;
        } else {
            mChannelBlockState = BLOCK_STATE_UNBLOCKED;
        }

        if (mChannelBlockState == mChannelLastBlockState) {
            mBlockStatusChanged = false;
        } else {
            mBlockStatusChanged = true;
        }
        LOGD("%s: ChannelBlockState:%d - BlockStatusChanged:%d",__FUNCTION__, mChannelBlockState, mBlockStatusChanged);
        return std::string("{\"ret\":0}");
    } else if (std::string("ADTV.BlockCurrentChannel") == resource) {
        mChannelBlockState = BLOCK_STATE_BLOCKED;
        mChannelLastBlockState = BLOCK_STATE_BLOCKED;
        mBlockStatusChanged = false;
        ScreenColorControl(false, VIDEO_LAYER_COLOR_SHOW_ALWAYES);
        return std::string("{\"ret\":0}");
    } else if (std::string("ADTV.UnblockCurrentChannel") == resource) {
         mChannelBlockState = BLOCK_STATE_UNBLOCKED;
         mChannelLastBlockState = BLOCK_STATE_UNBLOCKED;
         mBlockStatusChanged = false;
         ScreenColorControl(false, VIDEO_LAYER_COLOR_SHOW_DISABLE);
         return std::string("{\"ret\":0}");
    } else if (std::string("ADTV.GetAudioMute") == resource) {
         unsigned int ATV_mute = 0;
         unsigned int DTV_mute = 0;
         mAv.AudioGetMute(&ATV_mute, &DTV_mute);
         char ret[64];
         snprintf(ret, sizeof(ret), "{\"ret\":0,\"ATV_mute\":%d,DTV_mute\":%d}", ATV_mute, DTV_mute);
         ret[sizeof(ret) -1] = '\0';
         return std::string(ret);
    } else if (std::string("ADTV.AudioSetMute") == resource) {
         unsigned int atv_mute = paramGetInt(paras.c_str(), NULL, "ATV_mute", 0);
         unsigned int dtv_mute = paramGetInt(paras.c_str(), NULL, "DTV_mute", 0);
         mAv.AudioSetMute(atv_mute, dtv_mute);
         return std::string("{\"ret\":0}");
    } else if (std::string("setTestPattern") == resource) {
        int blue = paramGetInt(paras.c_str(), NULL, "blue", 0);
        if (blue) {
            mAv.SetVideoScreenColor(VIDEO_LAYER_BLUE);
        } else {
            mAv.SetVideoScreenColor(VIDEO_LAYER_BLACK);
        }
        return std::string("{\"ret\":0}");
    } else if (std::string("ADTV.setNoneStaticChangeToCurrentProgram") == resource) {
        int Scrambled = paramGetInt(paras.c_str(), NULL, "Scrambled", 0);
        int RadioChannel = paramGetInt(paras.c_str(), NULL, "RadioChannel", 0);
        int invalidService = paramGetInt(paras.c_str(), NULL, "invalidService", 0);

        mCurrentProgramIsScambled = (Scrambled?true:false);
        if (Scrambled || RadioChannel || invalidService) {
            mNoneStaticChange = true;
        } else {
            mNoneStaticChange = false;
        }
        return std::string("{\"ret\":0}");
    }
    return std::string("{\"ret\":1}");
}

int CTv::ScreenColorControl(bool color, int freq) {
    if (mIsMultiDemux) {//new path
        switch (freq) {
            case VIDEO_LAYER_COLOR_SHOW_DISABLE://disable color
                CVideotunnel::getInstance()->VT_disableColorFrame();
                break;
            case VIDEO_LAYER_COLOR_SHOW_ONCE://once
                CVideotunnel::getInstance()->VT_setvideoColor(color,false);
                break;
            case VIDEO_LAYER_COLOR_SHOW_ALWAYES://always
                CVideotunnel::getInstance()->VT_setvideoColor(color,true);
                break;
        }
    } else {//old path
        switch (freq) {
            case VIDEO_LAYER_COLOR_SHOW_DISABLE://disable color
                mAv.SetVideoLayerStatus(ENABLE_VIDEO_LAYER);
                mAv.SetVideoScreenColor(VIDEO_LAYER_BLACK);
                break;
            case VIDEO_LAYER_COLOR_SHOW_ONCE://once
                if ( color && getScreenColorSetting()) {
                    mAv.SetVideoScreenColor(VIDEO_LAYER_BLUE);
                } else {
                    mAv.SetVideoScreenColor(VIDEO_LAYER_BLACK);
                }
                mAv.SetVideoLayerStatus(ENABLE_AND_CLEAR_VIDEO_LAYER);
                break;
            case VIDEO_LAYER_COLOR_SHOW_ALWAYES://always
                if (color && getScreenColorSetting()) {
                    mAv.SetVideoScreenColor(VIDEO_LAYER_BLUE);
                } else {
                    mAv.SetVideoScreenColor(VIDEO_LAYER_BLACK);
                }
                mAv.SetVideoLayerStatus(DISABLE_VIDEO_LAYER);
                break;
        }
    }
    return 0;
}

void CTv::ScreenColorChange(int color) {
    LOGD("%s  new color:%d, m_source_input:%d, m_cur_sig_info.status:%d\n",__FUNCTION__, color, m_source_input, m_cur_sig_info.status);
    if (m_source_input != SOURCE_INVALID) {//for tvserver source
        tv_source_input_type_t input = mpTvin->Tvin_SourceInputToSourceInputType(m_source_input);
        if (SOURCE_TYPE_TV == input) {//for atv
            if (mBlockState == BLOCK_STATE_BLOCKED) {
                 LOGD("%s ATV channel blocked\n",__FUNCTION__);
                ScreenColorControl(true,VIDEO_LAYER_COLOR_SHOW_ALWAYES);
            } else if (m_cur_sig_info.status == TVIN_SIG_STATUS_NOSIG) {
                if (getScreenColorSetting()) {
                     ScreenColorControl(true,VIDEO_LAYER_COLOR_SHOW_ALWAYES);//ATV show blue
                 } else {
                     ScreenColorControl(true,VIDEO_LAYER_COLOR_SHOW_ONCE);//ATV show snow
                 }
            } else {
                LOGD("%s ATV signal normal, needn't do anything\n",__FUNCTION__);
            }
        } else if (SOURCE_TYPE_HDMI == input || SOURCE_TYPE_AV == input) {//for HDMI/AV
            if  (m_cur_sig_info.status == TVIN_SIG_STATUS_NOSIG) {
                ScreenColorControl(true,VIDEO_LAYER_COLOR_SHOW_ONCE);
            }
        }
    } else {
    //for none tune source,only set test pattern , do not control video layer status
        int ret = -1;
        char buf[256] = {0};
        ret = tvReadSysfs("/sys/class/video/disable_video", buf);
        if (ret < 0) {
            LOGD("%s: read /sys/class/video/disable_video error, return!\n",__FUNCTION__);
        }
        ret = atoi(buf);
        if (ret != 1) {
            LOGD("%s: video has enable!,only set black\n",__FUNCTION__);
            mAv.SetVideoScreenColor(VIDEO_LAYER_BLACK);
        } else {
            LOGD("%s: video is disabled!\n",__FUNCTION__);
            if (getScreenColorSetting()) {
                mAv.SetVideoScreenColor(VIDEO_LAYER_BLUE);
            } else {
                mAv.SetVideoScreenColor(VIDEO_LAYER_BLACK);
            }
        }
    }
}

int CTv::IsSupportPIP()
{
    int ret = 0;
    if (isFileExist("/dev/vdin2")) {
        LOGD("%s: support HDMI PIP\n",__FUNCTION__);
        ret = 1;
    } else {
        LOGD("%s: unsupport HDMI PIP\n",__FUNCTION__);
    }
    return ret;
}

