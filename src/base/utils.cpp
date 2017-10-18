#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <stack>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include "co_routine.h"

void sockaddr_init(const char *ip, unsigned port, struct sockaddr_in &addr)
{
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	int nIP = 0;
	if (!ip || '\0' == *ip || 0 == strcmp(ip, "0") || 0 == strcmp(ip, "0.0.0.0") || 0 == strcmp(ip, "*"))
	{
		nIP = htonl(INADDR_ANY);
	}
	else
	{
		nIP = inet_addr(ip);
	}
	addr.sin_addr.s_addr = nIP;
}

int socket_create(unsigned port, const char *ip, bool reuse)
{
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd >= 0)
	{
		if (port != 0)
		{
			if (reuse)
			{
				int nReuseAddr = 1;
				setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &nReuseAddr, sizeof(nReuseAddr));
			}
			struct sockaddr_in addr;
			sockaddr_init(ip, port, addr);
			int ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
			if (ret != 0)
			{
				close(fd);
				return -1;
			}
		}
	}
	return fd;
}

int fd_set_nonblock(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);

	flags |= O_NONBLOCK;
	flags |= O_NDELAY;
	
	return fcntl(fd, F_SETFL, flags);
}

int fd_pool(int fd, int timeout) {
    struct pollfd pf = {0};

    pf.fd = fd;
    pf.events = (POLLIN | POLLERR | POLLHUP);
    
    return co_poll(co_get_epoll_ct(), &pf, 1, timeout);
}

int co_sleep(int timeout) {
    return fd_pool(-1, timeout);
}

int read_n(int sock, unsigned char* buf, int len) {
	int total = 0;
	int offset = 0;
	int remain = len;

	while(remain > 0) {
		int nr = fd_pool(sock, 10000);
		int ret = read(sock, buf+offset, remain);
		if(ret <= 0) {
			if(nr >= 0) {
				break;
			}
		}else{
			total += ret;
			offset += ret;
			remain -= ret;
		}
	}

	return total;
}

int write_n(int sock, unsigned char* buf, int len) {
	int total = 0;
	int offset = 0;
	int remain = len;

	while(remain > 0) {
		int ret = write(sock, buf+offset, remain);
		if(ret <= 0) {
			if(errno == EAGAIN) {
				int nret = fd_pool(sock, 10000);
			}else{
				break;
			}
		}else{
			total += ret;
			offset += ret;
			remain -= ret;
		}
	}

	return total;
}

