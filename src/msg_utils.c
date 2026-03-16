#include <stdlib.h>
#include <string.h>
#include "msg_utils.h"

message_t * msg_utils_concatMessages(message_t * message1, message_t * message2)
{
    size_t totalSize = message1->length + message2->length;

    uint8_t * data = malloc(totalSize);

    memcpy(data, message1->data, message1->length);
    memcpy(data + message1->length, message2->data, message2->length);
    
    message_t * out = msg_utils_createMsg(data,totalSize);
     
    return msg_utils_copyMsg(out);
}
