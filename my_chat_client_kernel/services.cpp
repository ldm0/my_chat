#include<WinSock2.h>
#include<WS2tcpip.h>
#include<Windows.h>
#include<stdint.h>
#include<string.h>
#include<queue>			// for message received queue
#include<string>
#include "services.h"

#define SERVER_ADDR 
#define SERVER_PORT 9999
#define MSG_RECV_BUFFER_LENGTH 1000

#pragma comment(lib, "Ws2_32.lib")

static SOCKET main_socket_fd = INVALID_SOCKET;
static std::queue<std::string> msg_recv_queue;

void main_socket_close(void)
{
	if (main_socket_fd != INVALID_SOCKET)
		closesocket(main_socket_fd);
	main_socket_fd = INVALID_SOCKET;
	WSACleanup();
}

int main_socket_init(void)
{
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data)) {
		//fprintf(stderr, "WSAStartup failed.\n");
		return -1;
	}

	main_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (main_socket_fd == INVALID_SOCKET) {
		//fprintf(stderr, "socket creation failed.\n");
		main_socket_close();
		return -1;
	}


	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = ntohs(SERVER_PORT);
	if (inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr) == -1) {
		//fprintf(stderr, "inet_pton failed.\n");
		main_socket_close();
		return -1;
	}
	if (connect(main_socket_fd, (struct sockaddr *) & server_addr, sizeof(server_addr)) == -1) {
		//fprintf(stderr, "connect to server failed.\n");
		main_socket_close();
		return -1;
	}

	// set non-blocking after connection for convenience
	u_long b_non_blocking = 1;
	if (ioctlsocket(main_socket_fd, FIONBIO, &b_non_blocking) != NO_ERROR) {
		//fprintf(stderr, "ioctlsocket failed.\n");
		main_socket_close();
		return -1;
	}

	//printf("connection success.\n");
	return 0;
}

int services_get_msg_recv(std::string & msg_recv)
{
	if (msg_recv_queue.empty())
		return 0;
	msg_recv = msg_recv_queue.front();
	msg_recv_queue.pop();
	return 1;
}

int services_send_msg(const char *input)
{
	int input_length = strlen(input);
	if (send(main_socket_fd, input, input_length, 0) == -1)
		return -1;
	return 0;
}

static int b_another_thread_to_exit = 1;
static HANDLE is_another_thread_exit;

DWORD WINAPI service_recv_msg(LPVOID param)
{
	char recv_msg_buffer[MSG_RECV_BUFFER_LENGTH];
	while (!b_another_thread_to_exit) {
		int recv_length = recv(main_socket_fd, recv_msg_buffer, MSG_RECV_BUFFER_LENGTH - 1, 0);
		if (recv_length > 0) {
			recv_msg_buffer[recv_length] = 0;
			msg_recv_queue.push(recv_msg_buffer);
		}
		Sleep(100);
	}
	SetEvent(is_another_thread_exit);
	return 0;
}

int services_init()
{
	if (main_socket_init() == -1)
		return -1;
	b_another_thread_to_exit = 0;
	is_another_thread_exit = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (is_another_thread_exit == NULL)
		return -1;
	if (CreateThread(NULL, 0, service_recv_msg, 0, 0, NULL) == NULL)
		return -1;
	return 0;
}

void services_close()
{
	b_another_thread_to_exit = 1;
	WaitForSingleObject(is_another_thread_exit, INFINITE);
	CloseHandle(is_another_thread_exit);
	main_socket_close();
}

