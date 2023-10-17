/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */


#define LOG_TAG "tvserver"
#define LOG_TV_TAG "CVideotunnel"

#include "CVideotunnel.h"
#include <CTvLog.h>

CVideotunnel *CVideotunnel::mInstance;

CVideotunnel *CVideotunnel::getInstance()
{
    if (NULL == mInstance) mInstance = new CVideotunnel();
    return mInstance;
}

CVideotunnel::CVideotunnel()
{
    mTunnelId = -1;
    mColor = 0;
    mDevFd = -1;
}

CVideotunnel::~CVideotunnel()
{
    CloseVideotunnel();
    mTunnelId = -1;
    mColor = 0;
}

int CVideotunnel::OpenVideotunnel()
{

    if (mDevFd >= 0) {
        return mDevFd;
    }

    mDevFd = meson_vt_open();
    if (mDevFd < 0) {
        LOGE("%s: open meson_vt error!",__FUNCTION__);
    } else if (mTunnelId < 0) {
        LOGE("%s: error tunnel Id, can't connect,close!",__FUNCTION__);
        meson_vt_close(mDevFd);
        mDevFd = -1;
    }
    /* else {
        ret = meson_vt_connect(mDevFd, mTunnelId, 0);
    } */

    return mDevFd;
}

int CVideotunnel::CloseVideotunnel()
{
    int ret = -1;
    if (mDevFd >= 0) {
        //meson_vt_disconnect(mDevFd, mTunnelId, 0);
        ret = meson_vt_close(mDevFd);
        mDevFd = -1;
    } else {
        LOGD("%s: needn't close meson_vt!",__FUNCTION__);
    }
    return ret;
}

int CVideotunnel::SetTunnelId(int id)
{
    mTunnelId = id;
    return 0;
}

int CVideotunnel::GetTunnelId()
{
    return mTunnelId;
}

int CVideotunnel::SetVideotunnelSolidColor(tv_set_color_frame cmd, tv_set_color_frame_time cmd_data)
{
    int ret = -1;
    ret = OpenVideotunnel();
    if (ret >= 0) {
        ret = meson_vt_set_solid_color(mDevFd, mTunnelId, (vt_color_cmd)cmd, (vt_color_data)cmd_data);
    } else {
        LOGD("%s: Open Video tunnel error!",__FUNCTION__);
    }
    return ret;
}

int CVideotunnel::VT_setvideoColor(bool needColor, bool always)
{
    int ret = -1 ;
    LOGD("%s: needColor:%d, always:%d", __FUNCTION__, needColor, always);
    if (needColor) {
        if (always) {
            ret = SetVideotunnelSolidColor((tv_set_color_frame)getScreenColorSetting(),TV_SET_TIME_ALWAYS);
        } else {
            ret = SetVideotunnelSolidColor((tv_set_color_frame)getScreenColorSetting(),TV_SET_TIME_ONCE);
        }
    } else {
        //default use black frame
        if (always) {
            ret = SetVideotunnelSolidColor(TV_SET_BLACK,TV_SET_TIME_ALWAYS);
        } else {
            ret = SetVideotunnelSolidColor(TV_SET_BLACK,TV_SET_TIME_ONCE);
        }
    }
    return ret;
}

int CVideotunnel::VT_disableColorFrame()
{
    int ret = -1;
    LOGD("%s", __FUNCTION__);
        ret = SetVideotunnelSolidColor(TV_SET_BLACK,TV_SET_TIME_DISABLE);//disable show color frame
    return ret;
}

