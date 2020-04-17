#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "phev_core.h"
#include "msg_core.h"
#include "msg_utils.h"
#include "logger.h"

const static char *APP_TAG = "PHEV_CORE";

uint8_t phev_core_getXOR(const uint8_t *data)
{
    if (data[2] < 2)
    {
        return 0;
    }
    uint8_t xor = (data[2] & 0xfe) ^ ((data[0] & 0x01) ^ 1);

    return xor;
}
uint8_t phev_core_getType(const uint8_t *data)
{
    uint8_t xor = phev_core_getXOR(data);

    uint8_t type = data[2] ^ xor;

    return type;
}
uint8_t phev_core_validateCommand(const uint8_t command)
{
    for (int i = 0; i < sizeof(allowedCommands); i++)
    {
        if (command == allowedCommands[i])
        {
            return command;
        }
    }

    return 0;
}
uint8_t phev_core_getCommand(const uint8_t *data)
{
    uint8_t xor = phev_core_getXOR(data);

    uint8_t command = phev_core_validateCommand(data[0] ^ xor);

    if(command == 0)
    {
        LOG_E(APP_TAG,"Unknown command %02X",data[0] ^ xor);
    }
    
    return command;
}
uint8_t phev_core_getMessageLength(const uint8_t *data)
{
    uint8_t xor = phev_core_getXOR(data);
    uint8_t command = phev_core_getCommand(data);

    if ((command == 0xbb) || (command == 0xcd))
        return 4;
    uint8_t type = phev_core_getType(data);

    uint8_t length = data[1] ^ xor;

    return length;
}
uint8_t phev_core_getActualLength(const uint8_t *data)
{
    return phev_core_getMessageLength(data) + 2;
}
uint8_t phev_core_getActualLengthXOR(const uint8_t *data)
{
    return phev_core_getActualLength(data);
}
uint8_t phev_core_getDataLength(const uint8_t *data)
{
    return phev_core_getMessageLength(data) - 3;
}
uint8_t phev_core_getRegister(const uint8_t *data)
{
    uint8_t xor = phev_core_getXOR(data);

    uint8_t reg = data[3] ^ xor;
    return reg;
}
uint8_t * phev_core_getData(const uint8_t *data)
{
    uint8_t xor = phev_core_getXOR(data);
    uint8_t length = phev_core_getDataLength(data);
    uint8_t *decodedData = NULL;

    if(length > 0) 
    {
        decodedData = malloc(length);

        for (int i = 0; i < length; i++)
        {
            decodedData[i] = data[i + 4] ^ xor;
        }
    }

    return decodedData;
}
uint8_t phev_core_checksum(const uint8_t *data)
{
    uint8_t b = 0;
    int len = phev_core_getMessageLength(data) + 2;
    for (int i = 0;; i++)
    {
        if (i >= len - 1)
        {
            return b;
        }
        b = (uint8_t)(data[i] + b);
    }
}
uint8_t phev_core_getChecksum(const uint8_t *data)
{
    uint8_t xor = phev_core_getXOR(data);
    uint8_t checksumPos = phev_core_getActualLength(data) - 1;

    uint8_t checksum = data[checksumPos] ^ xor;
    return checksum;
}
uint8_t * phev_core_xorData(const uint8_t * data)
{
    uint8_t xor = phev_core_getXOR(data);

    uint8_t length = phev_core_getActualLength(data);
    uint8_t * decoded = malloc(length);

    for(int i=0;i<length;i++)
    {
        decoded[i] = data [i] ^ xor;
    }
    return decoded;
}
uint8_t * phev_core_xorDataWithValue(const uint8_t * data,uint8_t xor)
{

    uint8_t length = phev_core_getActualLength(data);
    uint8_t * decoded = malloc(length);

    for(int i=0;i<length;i++)
    {
        decoded[i] = data [i] ^ xor;
    }
    return decoded;
}
bool phev_core_validateChecksum(const uint8_t *data)
{
    uint8_t checksum = phev_core_getChecksum(data);
    
    uint8_t * decoded = phev_core_xorData(data);

    if(!decoded)
    {
        LOG_E(APP_TAG,"Decoded data is NULL");
        return false;
    }
    
    uint8_t expectedChecksum = phev_core_checksum(decoded);

    printf("Expected Checksum %02X Actual %02X\n",expectedChecksum,checksum);
    
    LOG_D(APP_TAG,"Expected Checksum %02X Actual %02X\n",expectedChecksum,checksum);
    
    free(decoded);

    return checksum == expectedChecksum;
}
phevMessage_t *phev_core_createMessage(const uint8_t command, const uint8_t type, const uint8_t reg, const uint8_t *data, const size_t length)
{
    LOG_V(APP_TAG, "START - createMessage");
    LOG_D(APP_TAG, "Data %d Length %d", data[0], length);
    phevMessage_t *message = malloc(sizeof(phevMessage_t));

    message->command = command;
    message->type = type;
    message->reg = reg;
    message->length = length;
    message->data = malloc(message->length);
    memcpy(message->data, data, length);
    message->XOR = 0;
    LOG_D(APP_TAG, "Message Data %d", message->data[0]);

    LOG_V(APP_TAG, "END - createMessage");

    return message;
}

void phev_core_destroyMessage(phevMessage_t *message)
{
    LOG_V(APP_TAG, "START - destroyMessage");

    if (message == NULL)
        return;

    if (message->data != NULL)
    {
        free(message->data);
    }
    free(message);
    LOG_V(APP_TAG, "END - destroyMessage");
}
int phev_core_validate_buffer(const uint8_t *msg, const size_t len)
{
    LOG_V(APP_TAG, "START - validateBuffer");

    uint8_t length = msg[1];
    uint8_t cmd = msg[0];

    for (int i = 0; i < sizeof(allowedCommands); i++)
    {
        if (cmd == allowedCommands[i])
        {
            //HACK to handle CD / CC
            if (cmd == 0xcd)
            {
                return 1;
            }
            if (length + 2 > len)
            {
                LOG_E(APP_TAG, "Valid command but length incorrect : command %02x length %dx expected %d", msg[0], length, len);
                return 0; // length goes past end of message
            }
            return 1; //valid message
        }
    }
    LOG_E(APP_TAG, "Invalid command %02x length %02x", msg[0], msg[1]);

    LOG_V(APP_TAG, "END - validateBuffer");

    return 0; // invalid command
}
uint8_t *phev_core_unscramble(const uint8_t *data, const size_t len)
{
    LOG_V(APP_TAG, "START - unscramble");
    uint8_t *decodedData = malloc(len);

    if (data[2] < 2)
    {
        LOG_D(APP_TAG, "unscramble not required");

        memcpy(decodedData, data, len);
        return decodedData;
    }
    const uint8_t xor = phev_core_getXOR(data);
    LOG_D(APP_TAG, "unscrambling");
    for (int i = 0; i < len; i++)
    {

        decodedData[i] = data[i] ^ xor;
    }
    LOG_V(APP_TAG, "END - unscramble");

    return decodedData;
}
bool phev_core_validateMessage(const uint8_t * data, const size_t len)
{
    LOG_V(APP_TAG, "START - validateMessage");

    uint8_t xor = phev_core_getXOR(data);
    uint8_t command;
    uint8_t length;
    uint8_t type;
    uint8_t reg;
    uint8_t checksum;
    uint8_t * messageData;

    if(phev_core_validateChecksum(data))
    {
        command = phev_core_getCommand(data);

        if(command == 0)
        {
            LOG_E(APP_TAG,"Invalid Command %02X",data[0]);
            return false;
        }
            
        length = phev_core_getDataLength(data);
        if(length > len)
        {
            LOG_E(APP_TAG,"Invalid Length %02X",length);
            return false;
        }
            
        type = phev_core_getType(data);

        if(type > 1)
        {
            LOG_E(APP_TAG,"Invalid Type %02X",data[2]);
            return false;
        }
        
        checksum = phev_core_getChecksum(data);
        reg = phev_core_getRegister(data);

        LOG_D(APP_TAG,"Valid message");
        LOG_D(APP_TAG, "Command %02x Length %d type %d reg %02x checksum %02X", command,length, type, reg,checksum);
        
        messageData = phev_core_getData(data);
        if (messageData != NULL && length > 0)
        {
            LOG_BUFFER_HEXDUMP(APP_TAG, messageData, length, LOG_DEBUG);
        }
        return true;    
    }
    return false;
}
int phev_core_decodeMessage(const uint8_t *data, const size_t len, phevMessage_t *msg)
{
    LOG_V(APP_TAG, "START - decodeMessage");
    
    if(!data)
    {
        LOG_E(APP_TAG,"Invalid pointer to data");
        return 0;
    }
    LOG_BUFFER_HEXDUMP(APP_TAG, data, len, LOG_VERBOSE);
    if(!msg) 
    {
        LOG_E(APP_TAG,"Invalid PhevMessage pointer");
        return 0;
    }
    
    bool ret = phev_core_validateMessage(data,len);

    if(ret) 
    {
       
        msg->command = phev_core_getCommand(data);
        msg->length = phev_core_getActualLength(data);
        msg->type = phev_core_getType(data);
        msg->reg = phev_core_getRegister(data);
        msg->checksum = phev_core_getChecksum(data);
        msg->XOR = phev_core_getXOR(data);
        msg->data = phev_core_getData(data);
        return 1;

    }
    LOG_E(APP_TAG,"Invalid message");    
    LOG_BUFFER_HEXDUMP(APP_TAG, data, len, LOG_ERROR);
    return 0;
}
message_t *phev_core_extractMessage(const uint8_t *data, const size_t len)
{
    LOG_V(APP_TAG, "START - extractMessage");

    //const uint8_t * unscrambled = phev_core_unscramble(data,len);

    uint8_t xor = phev_core_getXOR(data);

    message_t *message = msg_utils_createMsg(data, len);
    message_t *decoded = phev_core_XORInboundMessage(message, xor);
    if (decoded == NULL)
    {
        LOG_E(APP_TAG, "INVALID MESSAGE - Null returned from XOR Inbound message");
        LOG_V(APP_TAG, "END - extractMessage");
        return 0;
    }
    const uint8_t *unscrambled = decoded->data;
    if (phev_core_validate_buffer(unscrambled, len) != 0)
    {

        message_t *message = msg_utils_createMsg(data, unscrambled[1] + 2);

        LOG_V(APP_TAG, "END - extractMessage");

        return message;
    }
    else
    {
        LOG_E(APP_TAG, "Invalid Message");
        LOG_BUFFER_HEXDUMP(APP_TAG, data, len, LOG_ERROR);
        LOG_V(APP_TAG, "END - extractMessage");
        return NULL;
    }
}

int phev_core_encodeMessage(phevMessage_t *message, uint8_t **data)
{
    LOG_V(APP_TAG, "START - encodeMessage");

    LOG_V(APP_TAG, "encode XOR %02x", message->XOR);

    uint8_t *d = malloc(message->length + 5);

    d[0] = message->command;
    d[1] = (message->length + 3);
    d[2] = message->type;
    d[3] = message->reg;

    if (message->length > 0 && message->data != NULL)
    {

        memcpy(d + 4, message->data, message->length);
    }

    d[message->length + 4] = phev_core_checksum(d);

    if(message->XOR > 1)
    {
        uint8_t * out = malloc(phev_core_getActualLength(d));

        const uint8_t xorWithType = message->XOR ^ ((!d[2]) & 1);
        
        out = phev_core_xorDataWithValue(d,xorWithType);

        *data = out;

    } else 
    {
        *data = d;
    }

    LOG_D(APP_TAG, "Created message");
    LOG_BUFFER_HEXDUMP(APP_TAG, d, d[1] + 2, LOG_DEBUG);
    LOG_V(APP_TAG, "END - encodeMessage");

    return d[1] + 2;
}

phevMessage_t *phev_core_message(const uint8_t command, const uint8_t type, const uint8_t reg, const uint8_t *data, const size_t length)
{
    return phev_core_createMessage(command, type, reg, data, length);
}
phevMessage_t *phev_core_responseMessage(const uint8_t command, const uint8_t reg, const uint8_t *data, const size_t length)
{
    return phev_core_message(command, RESPONSE_TYPE, reg, data, length);
}
phevMessage_t *phev_core_requestMessage(const uint8_t command, const uint8_t reg, const uint8_t *data, const size_t length)
{
    return phev_core_message(command, REQUEST_TYPE, reg, data, length);
}
phevMessage_t *phev_core_commandMessage(const uint8_t reg, const uint8_t *data, const size_t length)
{
    if (phev_core_my18)
    {
        return phev_core_requestMessage(SEND_CMD, reg, data, length);
    }
    return phev_core_requestMessage(SEND_CMD, reg, data, length);
}
phevMessage_t *phev_core_simpleRequestCommandMessage(const uint8_t reg, const uint8_t value)
{
    const uint8_t data = value;
    if (phev_core_my18)
    {
        phev_core_requestMessage(SEND_CMD, reg, &data, 1);
    }
    return phev_core_requestMessage(SEND_CMD, reg, &data, 1);
}
phevMessage_t *phev_core_simpleResponseCommandMessage(const uint8_t reg, const uint8_t value)
{
    const uint8_t data = value;
    if (phev_core_my18)
    {
        phev_core_responseMessage(SEND_CMD, reg, &data, 1);
    }
    return phev_core_responseMessage(SEND_CMD, reg, &data, 1);
}
phevMessage_t *phev_core_ackMessage(const uint8_t command, const uint8_t reg)
{
    const uint8_t data = 0;
    return phev_core_responseMessage(command, reg, &data, 1);
}
phevMessage_t *phev_core_startMessage(const uint8_t *mac)
{
    uint8_t *data = malloc(7);
    data[6] = 0;
    memcpy(data, mac, 6);
    if (phev_core_my18)
    {
        phev_core_requestMessage(START_SEND_MY18, 0x01, data, 7);
    }
    return phev_core_requestMessage(START_SEND_MY18, 0x01, data, 7);
}
message_t *phev_core_startMessageEncoded(const uint8_t *mac)
{
    phevMessage_t *start = phev_core_startMessage(mac);
    phevMessage_t *startaa = phev_core_simpleRequestCommandMessage(0xaa, 0);
    message_t *message = msg_utils_concatMessages(
        phev_core_convertToMessage(start),
        phev_core_convertToMessage(startaa));
    phev_core_destroyMessage(start);
    phev_core_destroyMessage(startaa);
    return message;
}
phevMessage_t *phev_core_pingMessage(const uint8_t number)
{
    const uint8_t data = 0;
    if (phev_core_my18)
    {
        return phev_core_requestMessage(PING_SEND_CMD_MY18, number, &data, 1);
    }
    return phev_core_requestMessage(PING_SEND_CMD_MY18, number, &data, 1);
}
phevMessage_t *phev_core_responseHandler(phevMessage_t *message)
{
    uint8_t command = ((message->command & 0xf) << 4) | ((message->command & 0xf0) >> 4);
    phevMessage_t * response = phev_core_ackMessage(command, message->reg);
    response->XOR = message->XOR;
    return response;
}
message_t *phev_core_convertToMessage(phevMessage_t *message)
{
    LOG_V(APP_TAG, "START - convertToMessage");

    uint8_t *data = NULL;

    size_t length = phev_core_encodeMessage(message, &data);

    message_t *out = msg_utils_createMsg(data, length);

    free(data);

    LOG_V(APP_TAG, "END - convertToMessage");

    return out;
}

phevMessage_t *phev_core_copyMessage(phevMessage_t *message)
{
    phevMessage_t *out = malloc(sizeof(phevMessage_t));
    out->data = malloc(message->length);
    out->command = message->command;
    out->reg = message->reg;
    out->type = message->type;
    out->length = message->length;
    out->XOR = message->XOR;
    memcpy(out->data, message->data, out->length);

    return out;
}
message_t *phev_core_XOROutboundMessage(const message_t *message, const uint8_t xor)
{
    if (xor < 2)
        return (message_t *) msg_utils_copyMsg((message_t *) message);

    message_t *encoded = malloc(sizeof(message_t));
    encoded->data = malloc(message->length);
    encoded->length = message->length;
    const uint8_t xorWithType = xor ^ ((!message->data[2]) & 1);
    encoded->data = phev_core_xorDataWithValue(message->data,xorWithType);

    LOG_I(APP_TAG, "XOR message");
    return encoded;
}
message_t *phev_core_XORInboundMessage(const message_t *message, const uint8_t xor)
{
    if (xor < 2)
        return (message_t *) msg_utils_copyMsg((message_t *) message);

    
    uint8_t type = phev_core_getType(message->data);

    uint8_t len = phev_core_getActualLength(message->data);

    message_t *decoded = malloc(sizeof(message_t));
    decoded->length = len;
    decoded->data = phev_core_xorData(message->data);


    return decoded;
}
