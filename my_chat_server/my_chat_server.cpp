#include<sys/socket.h>
#include<sys/epoll.h>
#include<netinet/in.h>	// for struct sockadddr_in
#include<arpa/inet.h>	// for inet_pton()
#include<errno.h>		// for errno
#include<string.h>		// for strerr()
#include<stdio.h>		// for log
#include<unistd.h>		// for close()
#include<stdint.h>		// for uint16_t(port)
#include<vector>
#include<string>

#define SERVER_PORT 9999

#define CONNECTION_QUEUE_LENGTH 10
#define EVENT_BUFFER_LENGTH 10
#define EPOLL_TIMEOUT 3000
#define MSG_BUFFER_LENGTH 5000

static int main_socket_fd = -1;
static int epoll_fd = -1;
static std::vector<int> clients;
static std::vector<std::string> records;

int init_socket(uint16_t port)
{
	main_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (main_socket_fd == -1)
		return -1;
	struct sockaddr_in listen_addr;
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = ntohs(port);
	listen_addr.sin_addr.s_addr = ntohl(INADDR_ANY);
	if (bind(main_socket_fd, (struct sockaddr *) & listen_addr, sizeof(listen_addr)) == -1) {
		close(main_socket_fd);
		main_socket_fd = -1;
		return -1;
	}
	if (listen(main_socket_fd, CONNECTION_QUEUE_LENGTH) == -1) {
		close(main_socket_fd);
		main_socket_fd = -1;
		return -1;
	}
	return 0;
}

int init_epoll(void)
{
	epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
		return -1;
	struct epoll_event tmp_event;
	tmp_event.events = EPOLLIN | EPOLLET;
	tmp_event.data.fd = main_socket_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, main_socket_fd, &tmp_event) == -1) {
		close(epoll_fd);
		return -1;
	}
	return 0;
}

int client_pushback(int client_fd)
{
	struct epoll_event new_event;
	new_event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
	new_event.data.fd = client_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &new_event) == -1) {
		return -1;
	}
	clients.push_back(client_fd);
	return 0;
}

int client_erase(int client_fd)
{
	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL) == -1)
		return -1;
	int clients_size = (int)clients.size();
	for (int i = 0; i < clients_size; ++i) {
		if (clients[i] == client_fd) {
			clients.erase(clients.begin() + i);
			return 0;
		}
	}
	return -1;
}

int process_client_msg(int client_fd)
{
	char buffer[MSG_BUFFER_LENGTH];
	ssize_t msg_length = recv(client_fd, buffer, MSG_BUFFER_LENGTH - 1, 0);
	if (msg_length == -1) {
		return -1;
	}
	buffer[msg_length] = 0;
	printf("client message: %s\n", buffer);
	records.push_back(buffer);
	int clients_size = (int)clients.size();
	for (int i = 0; i < clients_size; ++i) {
		if (clients[i] == client_fd)
			continue;
		if (send(clients[i], buffer, msg_length, 0) == -1) {
			return -1;
		}
	}
	printf("message sent back.\n");
	return 0;
}

int send_msg_record(int client_fd)
{
	int records_size = (int)records.size();
	for (int i = 0; i < records_size; ++i) {
		if (send(client_fd, records[i].c_str(), records[i].length(), 0) == -1) {
			return -1;
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (init_socket(SERVER_PORT) != 0) {
		fprintf(stderr, "socket initialization failed.%s\n", strerror(errno));
		return -1;
	}
	printf("socket initialization successfully.\n");
	if (init_epoll() != 0) {
		fprintf(stderr, "epoll initialization failed.%s\n", strerror(errno));
		return -1;
	}
	printf("epoll initialization successfully.\n");
	for (;;) {
		struct epoll_event events[EVENT_BUFFER_LENGTH];
		int num_event = epoll_wait(epoll_fd, events, EVENT_BUFFER_LENGTH, EPOLL_TIMEOUT);
		if (num_event == -1) {
			fprintf(stderr, "epoll wait failed.%s\n", strerror(errno));
			return -1;
		}
		for (int i = 0; i < num_event; ++i) {
			if (events[i].data.fd == main_socket_fd) {
				struct sockaddr_in client_socket;
				socklen_t socket_length = (socklen_t)sizeof(client_socket);
				int new_client_socket_fd = accept(main_socket_fd, (struct sockaddr *) & client_socket, &socket_length);
				if (new_client_socket_fd == -1) {
					fprintf(stderr, "main socket accept failed.%s\n", strerror(errno));
					goto end;
				}
				if (client_pushback(new_client_socket_fd) == -1) {
					fprintf(stderr, "add new client to epoll failed.%s\n", strerror(errno));
					goto end;
				}
				send_msg_record(new_client_socket_fd);
				printf("client added. fd: %d\n", new_client_socket_fd);
			} else {
				if (events[i].events & EPOLLRDHUP) {
					if (client_erase(events[i].data.fd)) {
						fprintf(stderr, "client erase failed.%s\n", strerror(errno));
						goto end;
					}
					printf("client removed. fd: %d\n", events[i].data.fd);
				} else if (events[i].events & EPOLLIN) {
					if (process_client_msg(events[i].data.fd) == -1) {
						fprintf(stderr, "process client message failed.%s\n", strerror(errno));
						goto end;
					}
					printf("received client(fd: %d) message.\n");
				} else {
					fprintf(stderr, "unknown event.\n");
				}
			}
		}
	}
end:
	if (epoll_fd != -1)
		close(epoll_fd);
	if (main_socket_fd != -1)
		close(main_socket_fd);
	return 0;
}