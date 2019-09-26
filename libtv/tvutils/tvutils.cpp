/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "tvserver"
#define LOG_TV_TAG "tvutils"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/prctl.h>
#include <stdlib.h>
//#include <json/json.h>

#include "tvutils.h"

#include <vector>
#include <map>
#include <string>

int writeSys(const char *path, const char *val) {
    int fd;
    if ((fd = open(path, O_RDWR)) < 0) {
        printf("writeSys, open %s error(%s)", path, strerror (errno));
        return -1;
    }

    printf("write %s, val:%s\n", path, val);

    int len = write(fd, val, strlen(val));
    close(fd);
    return len;
}

int readSys(const char *path, char *buf, int count) {
    int fd, len;

    if ( NULL == buf ) {
        printf("buf is NULL");
        return -1;
    }

    if ((fd = open(path, O_RDONLY)) < 0) {
        printf("readSys, open %s error(%s)", path, strerror (errno));
        return -1;
    }

    len = read(fd, buf, count);
    if (len < 0) {
        printf("read %s error, %s\n", path, strerror(errno));
        goto exit;
    }

    int i , j;
    for (i = 0, j = 0; i <= len -1; i++) {
        //change '\0' to 0x20(spacing), otherwise the string buffer will be cut off ,if the last char is '\0' should not replace it
        if (0x0 == buf[i] && i < len - 1) {
            buf[i] = 0x20;
        }
        /* delete all the character of '\n' */
        if (0x0a != buf[i]) {
            buf[j++] = buf[i];
        }
    }
    buf[j] = 0x0;

    //LOGI("read %s, result length:%d, val:%s\n", path, len, buf);

exit:
    close(fd);
    return len;
}

int tvReadSysfs(const char *path, char *value) {
    char buf[SYS_STR_LEN+1] = {0};
    int len = readSys(path, (char*)buf, SYS_STR_LEN);
    strcpy(value, buf);
    return len;
}

int tvWriteSysfs(const char *path, const char *value) {
    return writeSys(path, value);
}

int tvWriteSysfs(const char *path, int value, int base)
{
    char str_value[64] = {0};
    if (base == 16) {
        sprintf(str_value, "0x%-x", value);
    } else {
        sprintf(str_value, "%d", value);
    }
    printf("tvWriteSysfs, str_value = %s", str_value);
    return writeSys(path, str_value);
}

//check file exist or not
bool isFileExist(const char *file_name)
{
    struct stat tmp_st;

    return stat(file_name, &tmp_st) == 0;
}

int GetFileAttrIntValue(const char *fp, int flag)
{
    int fd = -1;
    int temp = -1;
    char temp_str[32];

    memset(temp_str, 0, 32);

    fd = open(fp, flag);

    if (fd <= 0) {
        printf("open %s ERROR(%s)!!\n", fp, strerror(errno));
        return -1;
    }

    if (read(fd, temp_str, sizeof(temp_str)) > 0) {
        if (sscanf(temp_str, "%d", &temp) >= 0) {
            printf("%s -> get %s value =%d!\n", "TV", fp, temp);
            close(fd);
            return temp;
        } else {
            printf("%s -> get %s value error(%s)\n", "TV", fp, strerror(errno));
            close(fd);
            return -1;
        }
    }

    close(fd);
    return -1;
}
