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
//#include "vdin_fuse.h"
#define VDIN0_DEV_PATH "/dev/vdin0"

VdinDrvIf *VdinDrvIf::mInstance;

struct TvClientWrapper_t *GetTvClientWraperInstance(void)
{
	return GetInstance();
}

static void TvEventCallback(event_type_t eventType, void *eventData)
{
    if (eventType == TV_EVENT_TYPE_SIGLE_DETECT) {
        SignalDetectCallback_t *signalDetectEvent = (SignalDetectCallback_t *)(eventData);
        LOGD("%s: source: %d, signalFmt: %d, transFmt: %d, status: %d, isDVI: %d.\n", __FUNCTION__,
                                                   signalDetectEvent->SourceInput,
                                                   signalDetectEvent->SignalFmt,
                                                   signalDetectEvent->TransFmt,
                                                   signalDetectEvent->SignalStatus,
                                                   signalDetectEvent->isDviSignal);
    } else if (eventType == TV_EVENT_TYPE_SOURCE_CONNECT) {
        SourceConnectCallback_t *sourceConnectEvent = (SourceConnectCallback_t *)(eventData);
        LOGD("%s: source: %d, connectStatus: %d\n", __FUNCTION__,
                  sourceConnectEvent->SourceInput, sourceConnectEvent->ConnectionState);
    } else {
        LOGD("%s: invalid event.\n", __FUNCTION__);
    }
}

VdinDrvIf *VdinDrvIf::GetInstance() {
    if (mInstance == NULL) {
        mInstance = new VdinDrvIf();
    }
    return mInstance;
}

VdinDrvIf::VdinDrvIf()
{
	mVdin0DevFd = -1;
	mInstance   = NULL;
	cfg_port    = SOURCE_INVALID;
	active_port = SOURCE_INVALID;

	pTvClientWrapper = GetTvClientWraperInstance();
	setTvEventCallback(TvEventCallback);
}

VdinDrvIf::~VdinDrvIf()
{
}

int VdinDrvIf::OpenDev()
{
    if (mVdin0DevFd < 0) {
        mVdin0DevFd = open(VDIN0_DEV_PATH, O_RDWR);
		printf("%s:%d,video_fd:%d\n", __func__, __LINE__, mVdin0DevFd);
    } else {
        printf("# already open\n");
    }
    return mVdin0DevFd;
}

int VdinDrvIf::CloseDev()
{
    return 0;
}

#define AV_SET_INPUT_SPECAIL_HANDING	1
int VdinDrvIf::SetInput(tv_source_input_t port)
{
	int ret = 0;

	printf("%s,%d,port=%d\n",__func__, __LINE__, port);

	if (cfg_port == port)
		return 0;
/* Makes SoCTS PASS
	StopTv takes about 270ms,donot meets SOCTS requirement(<50ms)
 */
#if AV_SET_INPUT_SPECAIL_HANDING
	if (cfg_port != SOURCE_INVALID && port != SOURCE_INVALID && active_port != SOURCE_INVALID)
	{
		ret = StopTv(pTvClientWrapper, cfg_port);
		active_port = SOURCE_INVALID;
	}
	if (port != SOURCE_INVALID && active_port == SOURCE_INVALID)
	{
		ret = StartTv(pTvClientWrapper, port);
		active_port = port;
	}
#else
	if (cfg_port != SOURCE_INVALID)
	{
		ret = StopTv(pTvClientWrapper, cfg_port);
		active_port = SOURCE_INVALID;
	}
	if (port != SOURCE_INVALID)
	{
		ret = StartTv(pTvClientWrapper, port);
		active_port = port;
	}
#endif
	cfg_port = port;
	return ret;
}

int VdinDrvIf::CloseInput(void)
{
	int ret = 0;

	printf("%s,%d,active_port=%d\n",__func__, __LINE__, active_port);

	ret = StopTv(pTvClientWrapper, active_port);
	active_port = SOURCE_INVALID;

	return ret;
}

int VdinDrvIf::DeviceIOCtl ( int request, ... )
{
    int tmp_ret = -1;
    va_list ap;
    void *arg;

    if (mVdin0DevFd < 0) {
        mVdin0DevFd = OpenDev();
    }

    if ( mVdin0DevFd > 0 ) {
        va_start ( ap, request );
        arg = va_arg ( ap, void * );
        va_end ( ap );

        tmp_ret = ioctl ( mVdin0DevFd, request, arg );
        return tmp_ret;
    }

    return -1;
}

#ifdef __cplusplus
extern "C" {
#endif
int VdinDrvInit(void)
{

	return 0;
}



#ifdef __cplusplus
}
#endif

