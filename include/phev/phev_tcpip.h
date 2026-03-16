#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#ifndef _PHEV_TCPIP_H_
#define _PHEV_TCPIP_H_
#include <stdint.h>

#define TCP_READ_TIMEOUT 1000

int phev_tcpClientConnectSocket(const char *host, uint16_t port);

int phev_tcpClientDisconnectSocket(int soc);

int phev_tcpClientRead(int soc, uint8_t *buf, size_t len);

int phev_tcpClientWrite(int soc, uint8_t *buf, size_t len);

#endif