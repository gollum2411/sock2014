#ifndef DISCOVER_H
#define DISCOVER_H

#include <iostream>
#include <gollum2411.h>

void missing_cfg();
int start_discover_server(int udp_port, int tcp_port);
int look_for_servers(int udp_port, int timeout);
std::string get_node_name();

void print_recv(struct sockaddr_in host, std::string msg);

#endif // DISCOVER_H