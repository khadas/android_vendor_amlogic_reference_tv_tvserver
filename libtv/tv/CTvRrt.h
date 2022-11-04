/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#ifdef SUPPORT_ADTV
#include <am_epg.h>
#endif
#include "CTvEv.h"
#include "CTvLog.h"

#if !defined(_CDTVRRT_H)
#define _CDTVRRT_H

typedef struct rrt_info
{
    int    rrt_section_count;
    int    table_id;
    int    rating_region;
    int    version_number;
    char   rating_region_name[2048];
    int    dimension_defined;
    char   dimension_name[2048];
    int    graduated_scale;
    int    values_defined;
    char   abbrev_rating_value_text[2048];
    char   rating_value_text[2048];
} rrt_info_t;

typedef struct rrt_select_info_s
{
    int  status;
    int  rating_region_name_count;
    char rating_region_name[2048];
    int  dimensions_name_count;
    char dimensions_name[2048];
    int  rating_value_text_count;
    char rating_value_text[2048];
} rrt_select_info_t;

class CTvRrt
{
public:
    static const int MODE_ADD            = 0;
    static const int MODE_REMOVE         = 1;

    static const int SCAN_RRT            = 0x2000;
    static const int INVALID_ID          = -1;

    class RrtEvent : public CTvEv {
        public:
            RrtEvent(): CTvEv(CTvEv::TV_EVENT_RRT)
            {
                mRrtInfo = NULL;
            };
            ~RrtEvent()
            {
            };

            rrt_info_t* mRrtInfo;
        };

        class IObserver {
        public:
            IObserver() {};
            virtual ~IObserver() {};
            virtual void onEvent(const RrtEvent &ev) = 0;
        };

        int setObserver(IObserver *ob)
        {
            mpObserver = ob;
            return 0;
        }

public:
    static CTvRrt *getInstance();
    CTvRrt();
    ~CTvRrt();
    int StartRrtUpdate(void);
    int StopRrtUpdate(void);

private:
    int RrtCreate(int fend_id, int dmx_id, int src, char *textLangs);
    int RrtDestroy(void);
    static void RrtTableCallback(AM_EPG_Handle_t dev_no, int event_type, void *param, void *user_data);
    int GetRrtSectionCount(rrt_section_info_t *pData);
    void MultipleStringParser(atsc_multiple_string_t atsc_multiple_string, char *ret);
    static CTvRrt *mInstance;
    IObserver *mpObserver;
public:
    int mDmx_id ;
    AM_EPG_Handle_t mRrtScanHandle;
};


#endif //_CDTVRRT_H

