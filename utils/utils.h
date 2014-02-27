#ifndef _UTILS_H
#define _UTILS_H

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

void printerr(const char* fmt, ...);
void reporterr(int ret_val, int _errno);
void _debug(const char* filename, int line, FILE *desc, const char* fmt, ...);
int setup_listener_socket(int *server_desc, const uint32_t port);
int connect_to(const char *ip_addr, const uint32_t port);

#define debug(fmt, ...) _debug(__FILE__, __LINE__, stdout, fmt, ##__VA_ARGS__)

#endif
