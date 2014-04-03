#ifndef FILECLIENT_H
#define FILECLIENT_H

#include <iostream>

#include <gollum2411.h>

void do_ping(gollum2411::Socket::ptr sock, std::string ip, int port);
void get_filelist(std::string ip, int port);
void get_file(std::string filename, std::string ip, int port);

#endif
