#pragma once

#ifndef _MY_CHAT_CLIENT_H_
#define _MY_CHAT_CLIENT_H_

#include<string>

// return 0 means no message, return 1 means get message successfully
int services_get_msg_recv(std::string& msg_recv);

int services_send_msg(const char *msg);

int services_init(void);

void services_close(void);

#endif