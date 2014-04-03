#ifndef FILESERVER_H
#define FILESERVER_H

#include <gollum2411.h>

const size_t MAX_REQ_SIZE = 128;

int start_fileserver(int tcp_port);

#endif