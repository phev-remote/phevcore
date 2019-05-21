#ifndef _PHEV_TCPIP_H_
#define _PHEV_TCPIP_H_
#include <stdint.h>

int phev_tcpClientConnectSocket(const char *host, uint16_t port);

int phev_tcpClientRead(int soc, uint8_t *buf, size_t len);

int phev_tcpClientWrite(int soc, uint8_t *buf, size_t len);

#endif