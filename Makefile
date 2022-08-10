################################################################################
#
# aml_tvserver
#
################################################################################
LOCAL_PATH = $(shell pwd)
LDFLAGS += -Wl,--no-as-needed -lstdc++ -lpthread -lz -ldl -lrt -L$(STAGING_DIR)/usr/lib
CFLAGS += -Wall -Wno-unknown-pragmas -Wno-format -Wno-format-security\
          -O3 -fexceptions -fnon-call-exceptions -D_GNU_SOURCE \
          -I$(STAGING_DIR)/usr/include
CXXFLAGS += -D_FILE_OFFSET_BITS=64
LDFLAGS += -lbinder -llog

################################################################################
# libtv.so - src files
################################################################################
tv_SRCS  = \
	$(LOCAL_PATH)/libtv/tvutils/CFile.cpp \
	$(LOCAL_PATH)/libtv/tvutils/ConfigFile.cpp \
	$(LOCAL_PATH)/libtv/tvutils/CSerialPort.cpp \
	$(LOCAL_PATH)/libtv/tvutils/TvConfigManager.cpp \
	$(LOCAL_PATH)/libtv/tvutils/tvutils.cpp \
	$(LOCAL_PATH)/libtv/tvutils/zepoll.cpp \
	$(LOCAL_PATH)/libtv/tvutils/CTvLog.cpp \
	$(LOCAL_PATH)/libtv/CHDMIRxManager.cpp \
	$(LOCAL_PATH)/libtv/CTv.cpp \
	$(LOCAL_PATH)/libtv/CTvin.cpp \
	$(LOCAL_PATH)/libtv/CTvDevicesPollDetect.cpp \
	$(LOCAL_PATH)/libtv/CTvAudio.cpp \
	$(LOCAL_PATH)/libtv/CAmVideo.cpp \
	$(LOCAL_PATH)/libtv/CVpp.cpp \
	$(NULL)

################################################################################
# libtvclient.so - src files
################################################################################
tvclient_SRCS  = \
	$(LOCAL_PATH)/client/TvClient.cpp \
	$(LOCAL_PATH)/client/CTvClientLog.cpp \
	$(LOCAL_PATH)/client/TvClientWrapper.cpp \
	$(NULL)

tvclient_HEADERS = \
	$(LOCAL_PATH)/client/include \
	$(NULL)
################################################################################
# tvservice - src files
################################################################################
tvservice_SRCS  = \
	$(LOCAL_PATH)/service/main_tvservice.cpp \
	$(LOCAL_PATH)/service/TvService.cpp \
	$(NULL)

################################################################################
# tvtest - src files
################################################################################
tvtest_SRCS  = \
	$(LOCAL_PATH)/test/main_tvtest.c \
	$(NULL)

################################################################################
# tvin_fuse - src files
################################################################################
tvin_fuse_SRCS  = \
	$(LOCAL_PATH)/tvin_fuse/common/vdin.cpp \
	$(LOCAL_PATH)/tvin_fuse/hdmi/hdmirx_drv.cpp \
	$(LOCAL_PATH)/tvin_fuse/hdmi/hdmirx_fuse.cpp \
	$(LOCAL_PATH)/tvin_fuse/vbe/vbe_drv.cpp \
	$(LOCAL_PATH)/tvin_fuse/vbe/vbe_fuse.cpp \
	$(LOCAL_PATH)/tvin_fuse/vbi/vbi_drv.cpp \
	$(LOCAL_PATH)/tvin_fuse/vbi/vbi_fuse.cpp \
	$(LOCAL_PATH)/tvin_fuse/av/av_drv.cpp \
	$(LOCAL_PATH)/tvin_fuse/av/av_fuse.cpp \
	$(LOCAL_PATH)/tvin_fuse/pq/pq_drv.cpp \
	$(LOCAL_PATH)/tvin_fuse/pq/pq_fuse.cpp \
	$(LOCAL_PATH)/tvin_fuse/main.cpp \
	$(NULL)

################################################################################
# avtest - src files
################################################################################
avtest_SRCS  = \
	$(LOCAL_PATH)/tvin_fuse/av/av_test.cpp \
	$(NULL)

################################################################################
# vbitest - src files
################################################################################
vbitest_SRCS  = \
	$(LOCAL_PATH)/tvin_fuse/vbi/vbi_test.cpp \
	$(NULL)

################################################################################
# vbetest - src files
################################################################################
vbetest_SRCS  = \
	$(LOCAL_PATH)/tvin_fuse/vbe/vbe_test.cpp \
	$(NULL)

################################################################################
# hdmirxtest - src files
################################################################################
hdmirxtest_SRCS  = \
	$(LOCAL_PATH)/tvin_fuse/hdmi/hdmirx_test.cpp \
	$(NULL)

################################################################################
# pqfusetest - src files
################################################################################
pqfusetest_SRCS  = \
	$(LOCAL_PATH)/tvin_fuse/pq/pq_fuse_test.cpp \
	$(NULL)

# ---------------------------------------------------------------------
#  Build rules
BUILD_TARGETS  = libtvclient.so libtv.so tvservice tvtest
BUILD_TARGETS += tvin_fuse_app avtest vbitest vbetest hdmirxtest pqfusetest

.PHONY: all install clean

libtvclient.so: $(tvclient_SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC -I$(tvclient_HEADERS) \
	-o $@ $^ $(LDLIBS)

libtv.so: $(tv_SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC -I$(tvclient_HEADERS) \
	-I$(LOCAL_PATH)/libtv/tvutils -laudio_client -o $@ $^ $(LDLIBS)

tvservice: $(tvservice_SRCS) libtv.so
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LDFLAGS) -I$(tvclient_HEADERS) \
	-I$(LOCAL_PATH)/libtv -I$(LOCAL_PATH)/libtv/tvutils \
	-L$(LOCAL_PATH) -ltv -laudio_client -o $@ $^ $(LDLIBS)

tvtest: $(tvtest_SRCS) libtvclient.so
	$(CC) $(CFLAGS) -I$(tvclient_HEADERS) -L$(LOCAL_PATH) \
	-ltvclient $(LDFLAGS) -o $@ $^ $(LDLIBS)

tvin_fuse_app: $(tvin_fuse_SRCS) libtvclient.so
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LDFLAGS) -I$(tvclient_HEADERS) \
	-I$(LOCAL_PATH)/libtv -I$(LOCAL_PATH)/libtv/tvutils -I$(LOCAL_PATH)/tvin_fuse/include \
	-ltvclient -L$(LOCAL_PATH) -L$(LOCAL_PATH)/tvin_fuse/lib -lfuse3 -ltv -laudio_client \
	-L$(TARGET_DIR)/usr/lib -l_hal_pq \
	-o $@ $^ $(LDLIBS)

avtest: $(avtest_SRCS)
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LDFLAGS) -I$(tvclient_HEADERS) \
	-I$(LOCAL_PATH)/libtv -I$(LOCAL_PATH)/libtv/tvutils -I$(LOCAL_PATH)/tvin_fuse/include \
	-L$(LOCAL_PATH) -L$(LOCAL_PATH)/tvin_fuse/lib -lfuse3 -o $@ $^ $(LDLIBS)

vbitest: $(vbitest_SRCS)
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LDFLAGS) -I$(tvclient_HEADERS) \
	-I$(LOCAL_PATH)/libtv -I$(LOCAL_PATH)/libtv/tvutils -I$(LOCAL_PATH)/tvin_fuse/include \
	-L$(LOCAL_PATH) -L$(LOCAL_PATH)/tvin_fuse/lib -lfuse3 -o $@ $^ $(LDLIBS)

vbetest: $(vbetest_SRCS)
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LDFLAGS) -I$(tvclient_HEADERS) \
	-I$(LOCAL_PATH)/libtv -I$(LOCAL_PATH)/libtv/tvutils -I$(LOCAL_PATH)/tvin_fuse/include \
	-L$(LOCAL_PATH) -L$(LOCAL_PATH)/tvin_fuse/lib -lfuse3 -o $@ $^ $(LDLIBS)

hdmirxtest: $(hdmirxtest_SRCS)
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LDFLAGS) -I$(tvclient_HEADERS) \
	-I$(LOCAL_PATH)/libtv -I$(LOCAL_PATH)/libtv/tvutils -I$(LOCAL_PATH)/tvin_fuse/include \
	-L$(LOCAL_PATH) -L$(LOCAL_PATH)/tvin_fuse/lib -lfuse3 -o $@ $^ $(LDLIBS)

pqfusetest: $(pqfusetest_SRCS)
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LDFLAGS) -I$(tvclient_HEADERS) \
	-I$(LOCAL_PATH)/libtv -I$(LOCAL_PATH)/libtv/tvutils -I$(LOCAL_PATH)/tvin_fuse/include \
	-L$(LOCAL_PATH) -L$(LOCAL_PATH)/tvin_fuse/lib -lfuse3 -o $@ $^ $(LDLIBS)

all: $(BUILD_TARGETS)

clean:
	rm -f *.o $(BUILD_TARGETS)
	rm -rf $(STAGING_DIR)/usr/include/tvclient
	rm -rf $(STAGING_DIR)/usr/lib/libtvclient.so
	rm -rf $(TARGET_DIR)/usr/include/tvclient
	rm -rf $(TARGET_DIR)/usr/lib/libtvclient.so
	rm -rf $(TARGET_DIR)/usr/lib/libtv.so
	rm -rf $(TARGET_DIR)/usr/bin/avtest
	rm -rf $(TARGET_DIR)/usr/bin/tvservice
	rm -rf $(TARGET_DIR)/usr/bin/tvin_fuse_app
	rm -rf $(TARGET_DIR)/usr/bin/tvtest
	rm -rf $(TARGET_DIR)/usr/bin/vbitest
	rm -rf $(TARGET_DIR)/usr/bin/vbetest
	rm -rf $(TARGET_DIR)/usr/bin/hdmirxtest
	rm -rf $(TARGET_DIR)/usr/bin/pqfusetest

install:
	install -m 0644 libtvclient.so $(TARGET_DIR)/usr/lib
	install -m 0644 libtv.so $(TARGET_DIR)/usr/lib/
	install -m 0755 tvservice $(TARGET_DIR)/usr/bin/
	install -m 0755 tvtest $(TARGET_DIR)/usr/bin/
	install -m 0755 tvin_fuse_app $(TARGET_DIR)/usr/bin/
	install -m 0755 avtest $(TARGET_DIR)/usr/bin
	install -m 0755 vbitest $(TARGET_DIR)/usr/bin
	install -m 0755 vbetest $(TARGET_DIR)/usr/bin
	install -m 0755 hdmirxtest $(TARGET_DIR)/usr/bin
	install -m 0755 pqfusetest $(TARGET_DIR)/usr/bin
