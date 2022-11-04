/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */
#define LOG_TAG                 "tvserver"
#define LOG_TV_TAG              "CTvRrt"

#include "CTvRrt.h"

CTvRrt *CTvRrt::mInstance;
CTvRrt *CTvRrt::getInstance()
{
    LOGD("start rrt action!\n");
    if (NULL == mInstance) {
        mInstance = new CTvRrt();
    }

    return mInstance;
}

CTvRrt::CTvRrt()
{
    mRrtScanHandle      = NULL;
    mpObserver            = NULL;
    mDmx_id             = INVALID_ID;
}

CTvRrt::~CTvRrt()
{
    if (mInstance != NULL) {
        delete mInstance;
        mInstance = NULL;
    }
}

/**
 * @Function: StartRrtUpdate
 * @Description: Start Update rrt info
 * @Param:mode:RRT_AUTO_SEARCH:auto search;    RRT_MANU_SEARCH:manual search
 * @Return: 0 success, -1 fail
 */
int CTvRrt::StartRrtUpdate(void)
{
    if (RrtCreate(0, 2, 0, NULL) < 0) {//2 is demux id which according to DVB module!
        LOGD("RrtCreate failed!\n");
        return -1;
    }

    AM_ErrorCode_t ret;
    ret = AM_EPG_ChangeMode(mRrtScanHandle, MODE_ADD, SCAN_RRT);
    if (ret != AM_SUCCESS) {
        LOGD("StartRrtUpdate failed!\n");
        return -1;
    } else {
        LOGD("StartRrtUpdate success!\n");
        return 0;
    }
}

/**
 * @Function: StopRrtUpdate
 * @Description: Stop Update rrt info
 * @Param:
 * @Return: 0 success, -1 fail
 */
int CTvRrt::StopRrtUpdate(void)
{
    AM_ErrorCode_t ret;
    ret = AM_EPG_ChangeMode(mRrtScanHandle, MODE_REMOVE, SCAN_RRT);
    if (ret != AM_SUCCESS) {
        LOGD("StopRrtUpdate failed!\n");
    } else {
        LOGD("StopRrtUpdate success!\n");
    }

    int val = 0;
    val = RrtDestroy();
    if (val < 0) {
        LOGD("RrtDestroy failed!\n");
        return -1;
    }

    return 0;
}

/**
 * @Function: RrtCreate
 * @Description: open dev for RRT and set RRT event
 * @Param: fend_id: fend dev id;
           dmx_id:  demux dev id;
           src:     source;
           textLangs:language;
 * @Return: 0 success, -1 fail
 */
int CTvRrt::RrtCreate(int fend_id, int dmx_id, int src, char * textLangs)
{
#ifdef SUPPORT_ADTV
    AM_EPG_CreatePara_t para;
    AM_ErrorCode_t  ret;
    AM_DMX_OpenPara_t dmx_para;
    mDmx_id = dmx_id;

    memset(&dmx_para, 0, sizeof(dmx_para));
    LOGD("Opening demux%d ...", dmx_id);
    ret = AM_DMX_Open(mDmx_id, &dmx_para);
    if (ret != AM_SUCCESS) {
        LOGD("AM_DMX_Open failed");
        return - 1;
    }

    para.fend_dev       = fend_id;
    para.dmx_dev        = dmx_id;
    para.source         = src;
    para.hdb            = NULL;
    snprintf(para.text_langs, sizeof(para.text_langs), "%s", textLangs);

    ret = AM_EPG_Create(&para, &mRrtScanHandle);

    if (ret != AM_SUCCESS) {
        LOGD("AM_EPG_Create failed");
        return - 1;
    }

    /*disable internal default table procedure*/
    ret = AM_EPG_DisableDefProc(mRrtScanHandle, AM_TRUE);
    if (ret != AM_SUCCESS) {
        LOGD("AM_EPG_DisableDefProc failed");
        return - 1;
    }

    /*handle tables directly by user*/
    ret = AM_EPG_SetTablesCallback(mRrtScanHandle, AM_EPG_TAB_RRT, RrtTableCallback, NULL);
    if (ret != AM_SUCCESS) {
        LOGD("AM_EPG_SetTablesCallback failed");
        return - 1;
    }
#endif
    return 0;
}

/**
 * @Function: RrtDestroy
 * @Description: close dev for RRT and reset RRT event
 * @Param:
 * @Return: 0 success, -1 fail
 */
int CTvRrt::RrtDestroy(void)
{
#ifdef SUPPORT_ADTV
    AM_ErrorCode_t  ret;

    if (mRrtScanHandle != NULL) {
        ret = AM_EPG_Destroy(mRrtScanHandle);
        if (ret != AM_SUCCESS) {
            LOGD("AM_EPG_Destroy failed");
            return - 1;
        }
        mRrtScanHandle = NULL;
    }

    if (mDmx_id != INVALID_ID) {
        ret = AM_DMX_Close(mDmx_id);

        if (ret != AM_SUCCESS) {
            LOGD("AM_DMX_Close failed");
            return - 1;
        }
        mDmx_id = INVALID_ID;
    }
#endif
    return 0;
}

/**
 * @Function: RrtTableCallback
 * @Description: RRT event callback function
 * @Param:AM_EPG_Handle_t: dev handle
          event_type:RRT event type
          param:callback data
          user_data:
 * @Return:
 */
void CTvRrt::RrtTableCallback(AM_EPG_Handle_t handle, int event_type, void *param, void *user_data)
{
#ifdef SUPPORT_ADTV
    if (mInstance == NULL) {
        LOGD("rrt mInstance is NULL!\n");
        return;
    }

    if (mInstance->mpObserver == NULL) {
        LOGD("rrt mpObserver is NULL!\n");
        return;
    }

    if (!param) {
        //LOGD("rrt data is NULL!\n");
        return;
    }

    switch (event_type) {
    case AM_EPG_TAB_RRT: {
        int i, j;
        rrt_section_info_t *NewRrt = (rrt_section_info_t *)param;
        rrt_info_t rrt_info;
        RrtEvent RrtEv;
        rrt_info.rrt_section_count = mInstance->GetRrtSectionCount((rrt_section_info_t *)param);
        while (NewRrt != NULL) {
            LOGD("T [RRT:0x%02x][rr:0x%04x][dd:0x%04x] v[0x%x]\n", NewRrt->i_table_id, NewRrt->rating_region,
                    NewRrt->dimensions_defined, NewRrt->version_number);
            memset(&(rrt_info.rating_region_name), 0, sizeof(rrt_info.rating_region_name));
            rrt_info.table_id = NewRrt->i_table_id;
            rrt_info.rating_region = NewRrt->rating_region;
            rrt_info.version_number = NewRrt->version_number;
            //parser rating_region_name
            mInstance->MultipleStringParser(NewRrt->rating_region_name, rrt_info.rating_region_name);
            rrt_info.dimension_defined = NewRrt->dimensions_defined;
            //parser dimensions_info
            rrt_dimensions_info  *dimensions_info = NewRrt->dimensions_info;
            while (dimensions_info != NULL) {
                //parser dimensions_name
                memset(&(rrt_info.dimension_name), 0, sizeof(rrt_info.dimension_name));
                mInstance->MultipleStringParser(dimensions_info->dimensions_name,rrt_info.dimension_name);
                rrt_info.graduated_scale = dimensions_info->graduated_scale;
                rrt_info.values_defined = dimensions_info->values_defined - 1;
                LOGD("graduated_scale:%d, values_defined:%d .\n", rrt_info.graduated_scale, rrt_info.values_defined);
                //paser dimensions_value
                for (j=1;j<dimensions_info->values_defined;j++) {
                    memset(&(rrt_info.abbrev_rating_value_text), 0, sizeof(rrt_info.abbrev_rating_value_text));
                    memset(&(rrt_info.rating_value_text), 0, sizeof(rrt_info.rating_value_text));
                    mInstance->MultipleStringParser(dimensions_info->rating_value[j].abbrev_rating_value_text, rrt_info.abbrev_rating_value_text);
                    mInstance->MultipleStringParser(dimensions_info->rating_value[j].rating_value_text, rrt_info.rating_value_text);
                    RrtEv.mRrtInfo = &rrt_info;
                    mInstance->mpObserver->onEvent(RrtEv);
                    RrtEv.mRrtInfo = NULL;
                }

                dimensions_info = dimensions_info->p_next;
            }
            NewRrt = NewRrt->p_next;
        }

        break;
    }
    default:
        break;
    }
#endif
}

/**
 * @Function: GetRrtSectionCount
 * @Description: Get RRT Section Count
 * @Param:pData:Address of atsc RRT string which need to check count
 * @Return:count result
 */
int CTvRrt::GetRrtSectionCount(rrt_section_info_t *pData)
{
    int count  = 0;
    while (pData != NULL) {
        count ++;
        pData = pData->p_next;
    }
    LOGD("Section: count = %d\n", count);
    return count;
}

/**
 * @Function: MultipleStringParser
 * @Description: Multiple string data parser
 * @Param:atsc_multiple_string: Multiple string data
          ret: data after parser
 * @Return:
 */
void CTvRrt::MultipleStringParser(atsc_multiple_string_t atsc_multiple_string, char *ret)
{
    int i;
    for (i=0;i<atsc_multiple_string.i_string_count;i++) {
        int size = strlen((char *)atsc_multiple_string.string[0].string);
        if (ret != NULL) {
            memcpy(ret, atsc_multiple_string.string[0].string, size+1);
        }
    }

    return;
}

