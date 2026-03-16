#ifndef _LOGGER_H_
#define _LOGGER_H_
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include "msg_core.h"


#define LOG_NONE    0
#define LOG_ERROR   1      /*!< Critical errors, software module can not recover on its own */
#define LOG_WARN    2       /*!< Error conditions from which recovery measures have been taken */
#define LOG_INFO    3       /*!< Information messages which describe normal flow of events */
#define LOG_DEBUG   4      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
#define LOG_VERBOSE 5

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_NONE
#endif
static void nop(char * tag, ...) {}

#ifndef __XTENSA__

static void hexdump(const char * tag, const unsigned char * buffer, const int length, const int level);

#if LOG_LEVEL != LOG_NONE
#define LOG printf
#else
#define LOG nop
#endif
#define LOG_I(TAG, FORMAT , ...) LOG("INFO - %s: " FORMAT "\n", TAG, ##__VA_ARGS__) 
#define LOG_V(TAG, FORMAT , ...) LOG ("VERBOSE - %s: " FORMAT "\n", TAG, ##__VA_ARGS__)
#define LOG_D(TAG, FORMAT , ...) LOG ("DEBUG - %s: " FORMAT "\n", TAG, ##__VA_ARGS__)
#define LOG_E(TAG, FORMAT , ...) LOG ("ERROR - %s: " FORMAT "\n", TAG, ##__VA_ARGS__)
#define LOG_W(TAG, FORMAT , ...) LOG ("WARNING - %s: " FORMAT "\n", TAG, ##__VA_ARGS__)
#define LOG_BUFFER_HEXDUMP(TAG, BUFFER, LENGTH, LEVEL) hexdump(TAG, BUFFER, LENGTH,LEVEL)
#define LOG_MSG_BUNDLE(TAG, BUNDLE) nop

#else
#include "esp_log.h"

#ifndef LOG_DEBUG
#define LOG_DEBUG ESP_LOG_DEBUG
#endif
#ifndef LOG_INFO
#define LOG_INFO ESP_LOG_INFO
#endif
#ifndef LOG_WARNING
#define LOG_WARNING ESP_LOG_WARNING
#endif
#ifndef LOG_ERROR
#define LOG_ERROR ESP_LOG_ERROR
#endif
#define LOG_I(...) ESP_LOGI(__VA_ARGS__)
#define LOG_V(...) ESP_LOGV(__VA_ARGS__)
#define LOG_D(...) ESP_LOGD(__VA_ARGS__)
#define LOG_E(...) ESP_LOGE(__VA_ARGS__)
#define LOG_W(...) ESP_LOGW(__VA_ARGS__)
#define LOG_BUFFER_HEXDUMP(...) ESP_LOG_BUFFER_HEXDUMP(__VA_ARGS__)
#define LOG_MSG_BUNDLE(TAG, BUNDLE) nop
#define LOG_BUFFER_HEX(...) ESP_LOG_BUFFER_HEX(__VA_ARGD__)
#endif

static void hexdump(const char * tag, const unsigned char * buffer, const int length, const int level)
{
#if LOG_LEVEL != LOG_NONE
    if(length <= 0 || buffer == NULL) return;

    char out[17];
    memset(&out,'\0',17);
    printf("%s: ",tag);
    int i = 0;
    for(i=0;i<length;i++)
    {
        printf("%02x ",buffer[i]);
        out[i % 16] = (isprint(buffer[i]) ? buffer[i] : '.');
        if((i+1) % 8 == 0) printf(" ");
        if((i+1) % 16 ==0) {
            out[16] = '\0';
            printf(" | %s |\n%s: ",out,tag);
        }
    }
    if((i % 16) + 1 != 0)
    {
        int num = (16 - (i % 16)) * 3;
        num = ((i % 16) < 8 ? num + 1 : num);
        out[(i % 16)] = '\0';
        char padding[(16 * 3) + 2];
        memset(&padding,' ',num+1);
        padding[(16-i)*3] = '\0';
        printf("%s | %s |\n",padding,out);
    }
    printf("\n");
#endif
}
#endif
