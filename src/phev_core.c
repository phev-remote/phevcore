#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "phev_core.h"
#include "msg_core.h"
#include "msg_utils.h"
#include "logger.h"

const static char *APP_TAG = "PHEV_CORE";

uint8_t *phev_core_xorDataWithValue(const uint8_t *data, const uint8_t xor)
{
    LOG_V(APP_TAG, "START - xorDataWithValue");

    size_t length = (data[1] ^ xor) + 2;

    uint8_t *decoded = malloc(length);

    LOG_D(APP_TAG, "Decoding data with length %d with XOR %02X", length, xor);

    for (int i = 0; i < length; i++)
    {
        decoded[i] = data[i] ^ xor;
    }

    LOG_BUFFER_HEXDUMP(APP_TAG, decoded, length, LOG_DEBUG);
    LOG_V(APP_TAG, "END - xorDataWithValue");

    return decoded;
}
bool phev_core_checkIncomingCommand(const uint8_t command)
{
    switch (command)
    {
    case 0x3f:
        return true;
    case 0x6f:
        return true;
    case 0x4e:
        return true;
    case 0x5e:
        return true;
    case 0xBB:
        return true;
    case 0xCC:
        return true;
    case 0x2f:
        return true;
    default:
        return false;
    }
}
bool phev_core_checkOutgoingCommand(const uint8_t command)
{
    switch (command)
    {
    case 0xf3:
        return true;
    case 0xf6:
        return true;
    case 0xe4:
        return true;
    case 0xe5:
        return true;
    case 0xBB:
        return true;
    case 0xCC:
        return true;
    case 0xf2:
        return true;
    default:
        return false;
    }
}

bool phev_core_validateChecksum(const uint8_t *data)
{
    uint8_t length = data[1] + 2;

    uint8_t messageChecksum = data[length - 1];
    uint8_t calculatedChecksum = phev_core_checksum(data);

    if (calculatedChecksum == messageChecksum)
    {
        LOG_D(APP_TAG, "Valid checksum %02X", messageChecksum);
        return true;
    }
    else
    {
        LOG_D(APP_TAG, "Invalid checksum %02X expected %02X", messageChecksum, calculatedChecksum);
        return false;
    }
}
bool phev_core_validateChecksumXOR(const uint8_t *data, const uint8_t xor)
{
    uint8_t *decodedData = phev_core_xorDataWithValue(data, xor);
    
    return phev_core_validateChecksum(decodedData);
}
message_t *phev_core_unencodedIncomingMessage(const uint8_t *data)
{
    uint8_t command = data[0];
    uint8_t length = data[1] + 2;

    if(phev_core_validateChecksum(data))
    {
        switch (data[0])
        {
        case 0x4e:
        {
            LOG_D(APP_TAG, "Start (4E) unencoded");
            return msg_utils_createMsg(data, length);
        }
        case 0x5e:
        {
            LOG_D(APP_TAG, "Start (5E) unencoded");
            return msg_utils_createMsg(data, length);
        }
        case 0x3f:
        {
            LOG_D(APP_TAG, "Ping response unencoded");
            return msg_utils_createMsg(data, length);
        }
        case 0x6f:
        {
            LOG_D(APP_TAG, "Command unencoded");
            return msg_utils_createMsg(data, length);
        }
        }
    }
    LOG_E(APP_TAG,"Unknown unencoded command %02X", command);
    return NULL;
}
message_t *phev_core_unencodedOutgoingMessage(const uint8_t *data)
{
    uint8_t command = data[0];
    uint8_t length = data[1] + 2;

    if(phev_core_validateChecksum(data))
    {
        switch (data[0])
        {
        case 0xe4:
        {
            LOG_D(APP_TAG, "Start (E4) unencoded");
            return msg_utils_createMsg(data, length);
        }
        case 0xe5:
        {
            LOG_D(APP_TAG, "Start (E5) unencoded");
            return msg_utils_createMsg(data, length);
        }
        case 0xf3:
        {
            LOG_D(APP_TAG, "Ping response unencoded");
            return msg_utils_createMsg(data, length);
        }
        case 0xf6:
        {
            LOG_D(APP_TAG, "Command unencoded");
            return msg_utils_createMsg(data, length);
        }
        }
    }
    LOG_E(APP_TAG,"Unknown unencoded command %02X", command);
    return NULL;
}

message_t *phev_core_encodedIncomingMessage(const uint8_t *data)
{
    uint8_t xor = data[2];
    uint8_t command = data[0] ^ xor;
    uint8_t length = (data[1] ^ xor) + 2;

    if (phev_core_checkIncomingCommand(command) && phev_core_validateChecksumXOR(data, xor))
    {
        message_t * message = phev_core_createMsgXOR(data,length,xor);
        return message;
    }

    xor ^= 1;
    command = data[0] ^ xor;
    length = (data[1] ^ xor) + 2;

    if (phev_core_checkIncomingCommand(command) && phev_core_validateChecksumXOR(data, xor))
    {
        return phev_core_createMsgXOR(data, length, xor);
    }

    LOG_E(APP_TAG,"Unknown encoded command %02X or %02X", command, command ^ 1);

    return NULL;
}
message_t *phev_core_encodedOutgoingMessage(const uint8_t *data)
{
    uint8_t xor = data[2];
    uint8_t command = data[0] ^ xor;
    uint8_t length = (data[1] ^ xor) + 2;

    if (phev_core_checkOutgoingCommand(command) && phev_core_validateChecksumXOR(data, xor))
    {
        message_t * message = phev_core_createMsgXOR(data,length,xor);
        return message;
    }

    xor ^= 1;
    command = data[0] ^ xor;
    length = (data[1] ^ xor) + 2;

    if (phev_core_checkOutgoingCommand(command) && phev_core_validateChecksumXOR(data, xor))
    {
        return phev_core_createMsgXOR(data, length, xor);
    }

    LOG_E(APP_TAG,"Unknown encoded command %02X or %02X", command, command ^ 1);

    return NULL;
}
message_t * phev_core_extractIncomingMessageAndXOR(const uint8_t *data)
{
    LOG_V(APP_TAG, "START - extractIncomingMessageAndXOR");

    message_t *message = NULL;

    if (phev_core_checkIncomingCommand(data[0]))
    {
        message = phev_core_unencodedIncomingMessage(data);
    }
    else
    {
        message = phev_core_encodedIncomingMessage(data);
    }

    LOG_V(APP_TAG, "END - extractIncomingMessageAndXOR");

    return message;
}
message_t * phev_core_extractOutgoingMessageAndXOR(const uint8_t *data)
{
    LOG_V(APP_TAG, "START - extractOutgoingMessageAndXOR");

    message_t *message = NULL;

    if (phev_core_checkOutgoingCommand(data[0]))
    {
        message = phev_core_unencodedOutgoingMessage(data);
    }
    else
    {
        message = phev_core_encodedOutgoingMessage(data);
    }

    LOG_V(APP_TAG, "END - extractOutgoingMessageAndXOR");

    return message;
}
uint8_t phev_core_getMessageXOR(const message_t * message)
{
    if(message != NULL && message->ctx != NULL)
    {
        uint8_t * xor = (uint8_t *) message->ctx;
        return xor[0];
    }

    return 0;
}
message_t * phev_core_createMsgXOR(const uint8_t * data, const size_t length, const uint8_t xor)
{
    uint8_t * ctx = malloc(1);

    ctx[0] = xor;

    message_t * message = msg_utils_createMsgCtx(data, length, ctx);

    return message;
}
message_t * phev_core_extractAndDecodeIncomingMessageAndXOR(const uint8_t *data)
{
    LOG_V(APP_TAG, "START - extractAndDecodeIncomingMessageAndXOR");

    message_t * message = phev_core_extractIncomingMessageAndXOR(data);

    if(message == NULL)
    {
        LOG_W(APP_TAG,"Cannot extract message");
        return NULL;
    }

    uint8_t xor = phev_core_getMessageXOR(message);

    uint8_t * decodedData = phev_core_xorDataWithValue(message->data, xor);

    message_t * decoded = phev_core_createMsgXOR(decodedData,message->length,xor);

    free(decodedData);
    //msg_utils_destroyMsg(message);

    LOG_V(APP_TAG, "END - extractAndDecodeIncomingMessageAndXOR");

    return decoded;
}
message_t * phev_core_extractAndDecodeOutgoingMessageAndXOR(const uint8_t *data)
{
    LOG_V(APP_TAG, "START - extractAndDecodeOutgoingMessageAndXOR");

    message_t * message = phev_core_extractOutgoingMessageAndXOR(data);

    if(message == NULL)
    {
        LOG_W(APP_TAG,"Cannot extract message");
        return NULL;
    }

    uint8_t xor = phev_core_getMessageXOR(message);

    uint8_t * decodedData = phev_core_xorDataWithValue(message->data, xor);

    message_t * decoded = phev_core_createMsgXOR(decodedData,message->length,xor);

    free(decodedData);
    //msg_utils_destroyMsg(message);

    LOG_V(APP_TAG, "END - extractAndDecodeOutgoingMessageAndXOR");

    return decoded;
}
uint8_t phev_core_getXOR(const uint8_t *data, const uint8_t xor)
{
    LOG_V(APP_TAG, "START - getXOR");

    uint8_t newXOR = xor;

    const uint8_t command = data[0] ^ data[2];

    LOG_D(APP_TAG, "Command is %02x with decoded XOR and %02X with passed XOR", command, data[0] ^ xor);

    if (command == 0xBB)
    {
        newXOR = data[2];
    }
    if (command == 0XBA)
    {
        newXOR = data[2] ^ 1;
    }
    if (command == 0x6f)
    {
        newXOR = data[2];
    }
    if (command == 0x6e)
    {
        newXOR = data[2] ^ 1;
    }
    if (command == 0x3f)
    {
        newXOR = data[2];
    }
    if (command == 0x3e)
    {
        newXOR = data[2] ^ 1;
    }
    if (command == 0xcd)
    {
        newXOR = data[2] ^ 1;
    }
    if (command == 0x2f)
    {
        newXOR = data[2];
    }
    if (command == 0x4e)
    {
        newXOR = 0;
    }

    LOG_D(APP_TAG, "Returning new XOR of %02x", newXOR);
    LOG_V(APP_TAG, "END - getXOR");
    return newXOR;
}
uint8_t phev_core_getType(const uint8_t *data)
{
    uint8_t type = data[2];

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
/*
uint8_t phev_core_getCommand(const uint8_t *data)
{
    
    uint8_t command = phev_core_validateCommand(data[0]);

    if (command == 0)
    {
        LOG_E(APP_TAG, "Unknown command %02X", data[0]);
    }

    return command;
}
uint8_t phev_core_getRegister(const uint8_t *data)
{
    uint8_t xor = 0; //phev_core_getXOR(data);

    uint8_t reg = data[3] ^ xor;
    return reg;
} */
uint8_t * phev_core_getData(const uint8_t *data)
{
    uint8_t length = data[1] - 3;
    
    if(length == 0) 
    {
        LOG_D(APP_TAG,"No data in message");
        return NULL;
    }
    uint8_t * messageData = malloc(length);

    memcpy(messageData, data + 4, length);

    return messageData;
}
uint8_t phev_core_checksum(const uint8_t *data)
{
    uint8_t b = 0;
    int len = data[1] + 2;
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
    uint8_t checksum = data[data[1] + 1];

    return checksum;
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
    const uint8_t xor = 0; //phev_core_getXOR(data);
    LOG_D(APP_TAG, "unscrambling");
    for (int i = 0; i < len; i++)
    {

        decodedData[i] = data[i] ^ xor;
    }
    LOG_V(APP_TAG, "END - unscramble");

    return decodedData;
}
int phev_core_decodeMessage(const uint8_t *data, const size_t len, phevMessage_t *msg)
{
    LOG_V(APP_TAG, "START - decodeMessage");

    if (!data)
    {
        LOG_E(APP_TAG, "Invalid pointer to data");
        return 0;
    }
    LOG_BUFFER_HEXDUMP(APP_TAG, data, len, LOG_VERBOSE);
    if (!msg)
    {
        LOG_E(APP_TAG, "Invalid PhevMessage pointer");
        return 0;
    }

    message_t * message = phev_core_extractAndDecodeIncomingMessageAndXOR(data);

    if (message)
    {

        msg->command = message->data[0];
        msg->length = message->data[1] - 3;
        msg->type = message->data[2];
        msg->reg = message->data[3];
        msg->checksum = phev_core_getChecksum(message->data);
        msg->data = phev_core_getData(message->data);
        msg->XOR = phev_core_getMessageXOR(message);

        return 1;
    }
    LOG_E(APP_TAG, "Invalid message");
    LOG_BUFFER_HEXDUMP(APP_TAG, data, len, LOG_ERROR);
    return 0;
}
message_t *phev_core_extractMessage(const uint8_t *data, const size_t len, uint8_t xor)
{
    LOG_V(APP_TAG, "START - extractMessage");

    uint8_t *encoded = phev_core_xorDataWithValue(data, xor);

    message_t *decoded = msg_utils_createMsg(encoded, encoded[1] + 2);

    LOG_BUFFER_HEXDUMP("DECODED", decoded->data, decoded->data[1] + 2, LOG_DEBUG);

    return decoded;
}

int phev_core_encodeMessage(phevMessage_t *message, uint8_t **data)
{
    LOG_V(APP_TAG, "START - encodeMessage");

    LOG_D(APP_TAG, "encode XOR %02x", message->XOR);

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

    *data = d;

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

    return phev_core_requestMessage(SEND_CMD, reg, data, length);
}
phevMessage_t *phev_core_simpleRequestCommandMessage(const uint8_t reg, const uint8_t value)
{
    const uint8_t data = value;

    return phev_core_requestMessage(SEND_CMD, reg, &data, 1);
}
phevMessage_t *phev_core_simpleResponseCommandMessage(const uint8_t reg, const uint8_t value)
{
    const uint8_t data = value;

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

    return phev_core_requestMessage(PING_SEND_CMD_MY18, number, &data, 1);
}
phevMessage_t *phev_core_responseHandler(phevMessage_t *message)
{
    uint8_t command = ((message->command & 0xf) << 4) | ((message->command & 0xf0) >> 4);
    phevMessage_t *response = phev_core_ackMessage(command, message->reg);
    response->XOR = message->XOR;
    return response;
}
message_t *phev_core_convertToMessage(phevMessage_t *message)
{
    LOG_V(APP_TAG, "START - convertToMessage");

    uint8_t *data = NULL;

    size_t length = phev_core_encodeMessage(message, &data);

    message_t *out = phev_core_createMsgXOR(data, length,message->XOR);

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
uint8_t *phev_core_xorDataOutbound(const uint8_t *data, uint8_t xor)
{
    uint8_t length = data[1] + 2;

    uint8_t *decoded = malloc(length);

    for (int i = 0; i < length; i++)
    {
        decoded[i] = data[i] ^ xor;
    }
    return decoded;
}
message_t *phev_core_XOROutboundMessage(const message_t *message, const uint8_t xor)
{
    LOG_V(APP_TAG, "START - XOROutboundMessage");

    uint8_t * data = phev_core_xorDataOutbound(message->data, xor);

    message_t * encoded = msg_utils_createMsg(data,message->data[1] + 2);

    LOG_V(APP_TAG, "END - XOROutboundMessage");
    return encoded;
}
message_t *phev_core_XORInboundMessage(const message_t *message, const uint8_t xor)
{
    LOG_V(APP_TAG, "START - XORInboundMessage");

    message_t * out = phev_core_extractMessage(message->data,message->length,xor);
    
    LOG_V(APP_TAG, "END - XORInboundMessage");

    return out;

}
