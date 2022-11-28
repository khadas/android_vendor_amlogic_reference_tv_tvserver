/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */
 #ifndef _CVDIDEOTUNNEL_H_
 #define _CVDIDEOTUNNEL_H_

#include "video_tunnel.h"
#include "tvutils.h"

typedef enum tv_set_color_frame {
    TV_SET_BLACK,
    TV_SET_BLUE,
} tv_set_color_frame_t;

typedef enum tv_set_color_frame_time {
   /*
     * only show one frame of solid color,
     * will recovery when receive new frame
     */
    TV_SET_TIME_ONCE = 4,
    /*
     * Always show the solid color frame
     * until receive disable cmd or surface disconnect
     */
    TV_SET_TIME_ALWAYS = 5,
    /*
     * disable color frame
     */
    TV_SET_TIME_DISABLE = 6,
} tv_set_color_frame_time_t;

class CVideotunnel {

public:
    CVideotunnel();
    ~CVideotunnel();
    int SetTunnelId(int id);
    int GetTunnelId();
    int VT_setvideoColor(bool needColor, bool always);
    int VT_disableColorFrame();

    static CVideotunnel *getInstance();

private:
    static CVideotunnel *mInstance;
    int mDevFd;
    int mTunnelId;
    int mColor;

    int OpenVideotunnel();
    int CloseVideotunnel();
    int SetVideotunnelSolidColor(tv_set_color_frame cmd, tv_set_color_frame_time cmd_data);
};

 #endif

