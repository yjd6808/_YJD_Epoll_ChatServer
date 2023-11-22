// 작성자: 윤정도

#include "tcp_server.h"
#include "command_fn.h"

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

tcp_server::tcp_server(int capacity) 
    : _session_container(capacity) 
    , _epoll(&_session_container, &_command_dispatcher)
{}

void tcp_server::setup(int port) {
    _fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_port = htons(short(port));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;

    const int enable = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        fprintf(stderr, "setsockopt(SO_REUSEADDR) error(%d):%s\n", errno, strerror(errno));
        return;
    }

    if (::bind(_fd, (sockaddr*)&addr, sizeof(sockaddr_in)) == - 1)  {
        fprintf(stderr, "bind error(%d):%s\n", errno, strerror(errno));
        return;
    }

    if (::listen(_fd, 5) == -1) {
        fprintf(stderr, "listen error(%d):%s\n", errno, strerror(errno));
        return;
    }


    if (!_epoll.enroll(_fd, EPOLLIN)) {
        return;
    }

    _command_dispatcher.register_event(CMDID_ECHO_MESSAGE, command_fn::echo_message);

    printf("server can poll events\n");
}

void tcp_server::poll_events() {
    _epoll.poll(_fd);
} 


void tcp_server::close() {
    _session_container.disconnect_all();
    _epoll.close();
    ::close(_fd);
    printf("server closed\n");
}

