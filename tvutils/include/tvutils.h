/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef _TV_UTILS_H_
#define _TV_UTILS_H_

#pragma clang diagnostic ignored "-Wmacro-redefined"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utils/Mutex.h>
#include <string>
#include <map>
#include "PQType.h"
#include "json/json.h"

#include <systemcontrol/SystemControlClient.h>
using namespace android;

#ifndef DVB_SUCCESS
#define DVB_SUCCESS     (0)
#endif

#ifndef DVB_FAILURE
#define DVB_FAILURE     (-1)
#endif

#define SYS_STR_LEN                         1024
#ifndef PROPERTY_VALUE_MAX
#define PROPERTY_VALUE_MAX                  (92)
#endif
#define CC_ONE_SECTION_SIZE                 (0x1000)
#define CS_KEY_DATA_LIST_DEV_PATH           "/sys/class/unifykeys/list"
#define CS_KEY_DATA_NAME_DEV_PATH           "/sys/class/unifykeys/name"
#define CS_PANEL_INI_SPI_INFO_NAME          "lcd_tcon_spi"

#define SYS_VIDEO_INUSE_PATH                "/sys/class/video/video_inuse"
#define AUDIO_STD_PATH                      "/sys/kernel/debug/aml_atvdemod/aud_std"
#define AUDIO_OUTMODE_PATH                  "/sys/kernel/debug/aml_atvdemod/aud_mode"
#define ATVDEMODE_DEBUG_PATH                "/sys/class/aml_atvdemod/atvdemod_debug"
#define AUDIO_STREAM_OUTMODE_PATH           "/sys/kernel/debug/aml_atvdemod/signal_audmode"

static const char *PATH_WDT_USR_PET_49          = "/sys/module/aml_wdt/parameters/user_pet";
static const char *PATH_WDT_USR_PET_RESET_EN_49 = "/sys/module/aml_wdt/parameters/user_pet_reset_enable";
static const char *PATH_AFE_FORCE_NOSTD_49      = "/sys/module/tvin_afe/parameters/force_nostd";
static const char *PATH_LCD_SS_49               = "/sys/class/lcd/ss";
static const char *PATH_LCD_SS_54               = "/sys/class/aml_lcd/ss";

class SysClientcallback: public SysCtrlListener {
public:
    SysClientcallback() ;
    ~SysClientcallback() ;

    void notify(int   event __unused) {};
    void notifyFBCUpgrade(int state __unused, int param __unused) {};
    void onSetDisplayMode(int mode __unused) {};
    void onHdrInfoChange(int newHdrInfo __unused) {};
    void onAudioEvent(int param1 __unused, int param2 __unused, int param3 __unused, int param4 __unused) {};
    void onDensityChange(int param1 __unused,int param2 __unused, int param3 __unused) {};
    void onScreenColorChange(int newColor);

    class IScreenColorChangeObserver {
    public:
        IScreenColorChangeObserver() {};
        virtual ~IScreenColorChangeObserver() {};
        virtual void ScreenColorChange(int color) = 0;
    };

    void setObserver ( IScreenColorChangeObserver *pOb ) {
        mpObserver = pOb;
    };

    static SysClientcallback *getInstance();;

private:
    static SysClientcallback *mInstance;
    IScreenColorChangeObserver *mpObserver;
    SystemControlClient* mSysClient;
    sp<SysCtrlListener> mListener;
};

void setProperty(const char *key, const char *value);
bool getProperty(const char *key, char *value);
int getPropertyInt(const char *key, int def);
int tvReadSysfs(const char *path, char *value);
int tvWriteSysfs(const char *path, const char *value);
int tvWriteSysfs(const char *path, int value, int base=10);
int tvWriteDisplayMode(const char *mode);
int writeUKeyData(const char *key_name, unsigned char data_buf[], int wr_size);
int readUKeyData(const char *key_name, unsigned char data_buf[], int rd_size);
int tvSpiRead(unsigned int offset, unsigned int ulength, unsigned char *pubuf);
int tvSpiRead(int fd, unsigned int offset, unsigned int ulength, unsigned char *pubuf);
int tvSpiWrite(int fd, unsigned int offset, unsigned int ulength, unsigned char *pubuf);
int tvSpiOpen(void);
int tvFlashWrite(const char *partition_name, long offset,unsigned char *buffer, unsigned int length);
int tvFlashRead(const char *partition_name, long offset,unsigned char *buffer, unsigned int length);
int isSysfsExist(const char *path);
int getKernelMajorVersion(void);
int getKernelMinorVersion(void);


//add for PQ
int tvLoadPQSettings(source_input_param_t source_input_param);
int tvSSMReadNTypes(int id, int data_len,  int *data_buf, int offset);
int tvSSMWriteNTypes(int id, int data_len, int data_buf, int offset);
int tvGetActualAddr(int id);
int tvGetActualSize(int id);
int tvSetPQMode ( vpp_picture_mode_t mode, int is_save, int is_autoswitch);
vpp_picture_mode_t tvGetPQMode ( void );
int tvSavePQMode ( vpp_picture_mode_t mode);
vpp_picture_mode_t tvGetLastPQMode ( void );
int tvSetCurrentSourceInfo(tv_source_input_t tv_source_input, tvin_sig_fmt_t sig_fmt, tvin_trans_fmt_t trans_fmt);
int tvSetCVD2Values();
int tvSetCurrentHdrInfo(int hdr_info);
int tvSetCurrentAspectRatioInfo(int AspectRatioInfo);
int tvSetDisplaymode(int source_input, int mode, int isSave);
int tvGetDisplaymode(int source_input);
int tvSetDLGEnable(int enable);
int tvGetDLGEnable();

//PQ end

int setVideoColor(int color);
int getScreenColorSetting();
int getScreenStaticFrameEnable();
int tvsetAudioParam(int param1, int param2, int param3, int param4);
void setDlgControl();

extern int Tv_MiscRegs(const char *cmd);

extern int TvMisc_SetLVDSSSC(int val);
extern int TvMisc_SetUserCounterTimeOut(int timeout);
extern int TvMisc_SetUserCounter(int count);
extern int TvMisc_SetUserPetResetEnable(int enable);
extern int TvMisc_SetSystemPetResetEnable(int enable);
extern int TvMisc_SetSystemPetEnable(int enable);
extern int TvMisc_SetSystemPetCounter(int count);
extern int SetAudioOutmode(int mode);
extern int GetAudioOutmode();
extern int GetAudioStreamOutmode();

extern void TvMisc_EnableWDT(bool kernelpet_disable, unsigned int userpet_enable, unsigned int kernelpet_timeout, unsigned int userpet_timeout, unsigned int userpet_reset);
extern void TvMisc_DisableWDT(unsigned int userpet_enable);
extern int GetTvDBDefineSize();

extern int Set_Fixed_NonStandard(int value);


extern int TvMisc_DeleteDirFiles(const char *strPath, int flag);

extern bool isFileExist(const char *file_name);
extern int GetPlatformHaveDDFlag();
extern int getBootEnv(const char *key, char *value, const char *def_val);
extern void setBootEnv(const char *key, const char *value);
extern int readSysfs(const char *path, char *value);
extern void writeSysfs(const char *path, const char *value);
extern int GetFileAttrIntValue(const char *fp, int flag = O_RDWR);

template<typename T1, typename T2>
int ArrayCopy(T1 dst_buf[], T2 src_buf[], int copy_size)
{
    int i = 0;

    for (i = 0; i < copy_size; i++) {
        dst_buf[i] = src_buf[i];
    }

    return 0;
};


typedef std::map<std::string, std::string> STR_MAP;

extern void jsonToMap(STR_MAP &m, const std::string &j);
extern void mapToJson(std::string &j, const STR_MAP &m);
extern void mapToJson(std::string &j, const STR_MAP &m, const std::string &k);
extern void mapToJsonAppend(std::string &j, const STR_MAP &m, const std::string &k);

class Paras {
protected:
    STR_MAP mparas;

public:
    Paras() {}
    Paras(const Paras &paras):mparas(paras.mparas) {}
    Paras(const char *paras) { jsonToMap(mparas, std::string(paras)); }
    Paras(const STR_MAP &paras):mparas(paras) {}

    void clear() { mparas.clear(); }

    int getInt (const char *key, int def) const;
    void setInt(const char *key, int v);

    const std::string toString() { std::string s; mapToJson(s, mparas); return s; }

    Paras operator + (const Paras &p);
    Paras& operator = (const Paras &p) { mparas = p.mparas; return *this; };
};

float getUptimeSeconds();

extern bool jsonIsMember(const char *json, const char* key);
extern int jsonGetInt(const char *json, const char *obj, const char *value, int def);
extern int64_t jsonGetInt64(const char *json, const char *obj, const char *value, int64_t def);
extern const std::string jsonGetString(const char *json, const char *obj, const char *value, const char *def);

extern const std::string jsonToString(Json::Value value);

extern int paramGetInt(const char *param, const char *section, const char *value, int def);
extern int64_t paramGetInt64(const char *param, const char *section, const char *value, int64_t def);
extern const std::string paramGetString(const char *param, const char *section, const char *value, const char *def);

inline const char *toReadable(const char *s) { return s? s : "null"; }

extern bool propertyGetBool(const char *prop, bool def);

extern bool isVideoInuse();

#endif  //__TV_MISC_H__
