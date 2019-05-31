#include<stdio.h>
#include<string.h>
#include<Windows.h>
#include"services.h"
#include<string>

static const char test_msg_send[] = "msg from my_chat_client_kernel_test.";

static int num_msg_recv = 0;

void on_recv_msg(const char *msg)
{
	if (strcmp(test_msg_send, msg) != 0) {
		printf("FAILED: service %dth recv message is damaged.\n", num_msg_recv);
	}
	printf("SUCCESS: service %dth recv.\n", num_msg_recv++);
}

int main()
{
	if (services_init() == -1) {
		printf("FAILED: services init failed.\n");
		services_close();
		return -1;
	}
	printf("SUCCESS: services init succeed.\n");
	for (int i = 0; i < 5; ++i) {
		if (services_send_msg(test_msg_send) == -1) {
			printf("FAILED: services %dth send failed.\n", i);
			return -1;
		}
		printf("SUCCESS: services %dth send succeed.\n", i);
		Sleep(1000);
	}
	printf("Open another instance of current client to test recv now.\n");
	for (int i = 0; i < 5; ++i) {
		std::string msg_recv;
		for (;;) {
			while (services_get_msg_recv(msg_recv) == 0)
				Sleep(100);
			if (strcmp(test_msg_send, msg_recv.c_str()) == 0)
				break;
			printf("Damaged message received!!!\n");
		}
		printf("%d message received. Need 5 message in total.\n", i);
	}
	printf("PASS: test all pass.\n");
	services_close();
}