#include "base_nio_handler.hpp"
#include "service_log.hpp"
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

int base_nio_handler::readn_timeout(int fd, std::string &content, int buf_len) {
    char buf[READ_BUF_SIZE+1];
    timeval timeout = {
        0,
        recv_timeout * 1000
    };
    int n,len;
    int left = buf_len;

    while (left > 0)
    {
        len = left>READ_BUF_SIZE?READ_BUF_SIZE:left;
        if ((n = read_data(fd, buf, len,&timeout)) <= 0)
            return buf_len - left;

        buf[n] = '\0';
        content += buf;
        left -= n;
    }

    return buf_len;
}

int base_nio_handler::readn_timeout(int fd, char* content, int need_to_read, timeval* timeout) {
	char buf[READ_BUF_SIZE + 1];
	int n, left;
	int len;
    int ptr = 0;
	left = need_to_read;
	while (left > 0) {
		len = left > READ_BUF_SIZE ? READ_BUF_SIZE : left;
		n = read_data(fd, buf, len, timeout);
		if (n <= 0 ) {
			if(n == 0)
				return -2;
			return need_to_read - left;
		}
		buf[n] = '\0';
        memcpy(content + ptr, buf, len);
        ptr = ptr + n;
		left = left - n;
	}
	return need_to_read;
}

int base_nio_handler::read_data(int fd, void * buf, int buf_len, timeval* timeout) {
    
    if (!(timeout->tv_sec >= 0 && timeout->tv_usec >=0))
        return -1;    

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
        
    pollfd read_fd;
    read_fd.fd = fd;
    read_fd.events = POLLIN;
    int poll_ret = poll(&read_fd, 1, timeout->tv_sec * 1000 + timeout->tv_usec / 1000);
    if (poll_ret <= 0 || !(read_fd.revents & POLLIN)) {
        fprintf(stderr, "rd ret=%d, errno=%d\n", poll_ret, errno);
        if (read_fd.revents & POLLIN) {
            fprintf(stderr, "POLLIN\n");
        }
        if (read_fd.revents & POLLPRI) {
            fprintf(stderr, "POLLPRI\n");
        }
        if (read_fd.revents & POLLOUT) {
            fprintf(stderr, "POLLOUT\n");
        }
        if (read_fd.revents & POLLRDHUP) {
            fprintf(stderr, "POLLRDHUP\n");
        }
        if (read_fd.revents & POLLERR) {
            fprintf(stderr, "POLLERR\n");
        }
        if (read_fd.revents & POLLHUP) {
            fprintf(stderr, "POLLHUP\n");
        }
        if (read_fd.revents & POLLNVAL) {
            fprintf(stderr, "POLLNVAL\n");
        }
        if (read_fd.revents & POLLRDNORM) {
            fprintf(stderr, "POLLRDNORM\n");
        }
        if (read_fd.revents & POLLRDBAND) {
            fprintf(stderr, "POLLRDBAND\n");
        }
        if (read_fd.revents & POLLWRNORM) {
            fprintf(stderr, "POLLWRNORM\n");
        }
        if (read_fd.revents & POLLWRBAND) {
            fprintf(stderr, "POLLWRBAND\n");
        }
        return -1; 
    }
    int len = read(fd,buf,buf_len);
    gettimeofday(&end_time,NULL);
    long cost = 1000000 * (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec);
    if (timeout->tv_sec > 0) {
        timeout->tv_usec += timeout->tv_sec * 1000000;
        timeout->tv_sec = 0;
    }
    timeout->tv_usec -= cost; 
    //fprintf(stderr,"ori tv_sec=%ld,tv_usec=%ld,cost=%ld\n",timeout->tv_sec,timeout->tv_usec,cost);
    if (len == 0) {
        _INFO("read_len=0,disconnect by peer");
    }
    return len;
}

int base_nio_handler::writen_timeout(int fd, const void *buf, int buf_len, int to) {
	int left = buf_len;
	int n;
    //fd_set wset;
    pollfd write_fd;
    write_fd.fd = fd;
    write_fd.events = POLLOUT;
    int poll_ret;

	int real_to = to == -1 ? send_timeout : to;
    timeval timeout = {
        0,
        real_to * 1000
    };

    while (left > 0) {
        poll_ret = poll(&write_fd, 1, timeout.tv_sec * 1000 + timeout.tv_usec / 1000);
        if (poll_ret <= 0 || !(write_fd.revents & POLLOUT)) {
            fprintf(stderr, "wr ret=%d, errno=%d\n", poll_ret, errno);
            if (write_fd.revents & POLLIN) {
                fprintf(stderr, "POLLIN\n");
            }
            if (write_fd.revents & POLLPRI) {
                fprintf(stderr, "POLLPRI\n");
            }
            if (write_fd.revents & POLLOUT) {
                fprintf(stderr, "POLLOUT\n");
            }
            if (write_fd.revents & POLLRDHUP) {
                fprintf(stderr, "POLLRDHUP\n");
            }
            if (write_fd.revents & POLLERR) {
                fprintf(stderr, "POLLERR\n");
            }
            if (write_fd.revents & POLLHUP) {
                fprintf(stderr, "POLLHUP\n");
            }
            if (write_fd.revents & POLLNVAL) {
                fprintf(stderr, "POLLNVAL\n");
            }
            if (write_fd.revents & POLLRDNORM) {
                fprintf(stderr, "POLLRDNORM\n");
            }
            if (write_fd.revents & POLLRDBAND) {
                fprintf(stderr, "POLLRDBAND\n");
            }
            if (write_fd.revents & POLLWRNORM) {
                fprintf(stderr, "POLLWRNORM\n");
            }
            if (write_fd.revents & POLLWRBAND) {
                fprintf(stderr, "POLLWRBAND\n");
            }
            return -1;
        }
        /*
        FD_ZERO(&wset);
        FD_SET(fd, &wset);
        if (select(fd + 1, NULL, &wset, NULL, &timeout) <= 0)
            return -1;
        */
		if ((n = write(fd, buf, left)) <= 0)
			return buf_len - left;

		buf = (char *)buf + n;
		left -= n;
    }
    return buf_len;
}

int base_nio_handler::setSocket(int& fd) {
    if (fd <= 0) {
        _ERROR("error socket fd=%d", fd);
        return -1;
    }
    int options;
    options = SOCKET_SND_BUF_SIZE;
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &options, sizeof(int));
    options = SOCKET_RCV_BUF_SIZE;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &options, sizeof(int));
    options = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(int));
    options = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, options | O_NONBLOCK);
    int on = 1;
    int ret = -1;
    ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
    return ret;
}

int base_nio_handler::connectSocket(sockaddr_in* address) {
    int fd = -1;
    if (address == NULL) {
        _ERROR("null address");
        return -1;
    }
    _INFO("begin to connect to %s:%d", inet_ntoa(address->sin_addr), ntohs(address->sin_port));
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        _ERROR("create socket fd error on %s:%d", inet_ntoa(address->sin_addr), ntohs(address->sin_port));
        return -1;
    }
    if (connect(fd, (struct sockaddr*)address, sizeof(*address)) < 0) {
        _ERROR("connect socket fail on %s:%d", inet_ntoa(address->sin_addr), ntohs(address->sin_port));
        return -1;
    }
    _INFO("connect to %s:%d success", inet_ntoa(address->sin_addr), ntohs(address->sin_port));
    return fd;
}

