/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef TV_SERVICE_H
#define TV_SERVICE_H

#include <binder/Binder.h>
#include <binder/Parcel.h>
#include "CTv.h"
#include "CPQControl.h"

using namespace android;

class TvService: public CTv::TvIObserver, public BBinder{
public:
    TvService();
    ~TvService();
    static TvService *GetInstance();

    enum command {
        CMD_START = IBinder::FIRST_CALL_TRANSACTION,
        CMD_TV_ACTION = IBinder::FIRST_CALL_TRANSACTION + 1,
        CMD_SET_TV_CB = IBinder::FIRST_CALL_TRANSACTION + 2,
        CMD_CLR_TV_CB = IBinder::FIRST_CALL_TRANSACTION + 3,
        EVT_SRC_CT_CB = IBinder::FIRST_CALL_TRANSACTION + 4,
        EVT_SIG_DT_CB = IBinder::FIRST_CALL_TRANSACTION + 5,
    };

private:
    virtual void onTvEvent(CTvEvent &event);
    int SendSignalForSignalDetectEvent(CTvEvent &event);
    int SendSignalForSourceConnectEvent(CTvEvent &event);
    int ParserTvCommand(const char *commandData);

    CTv *mpTv;
    CPQControl *mpPQcontrol;

    sp<IBinder> evtCallBack;
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data, Parcel* reply,
                                uint32_t flags = 0);
};
#endif
