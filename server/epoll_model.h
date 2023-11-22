// 작성자: 윤정도

#pragma once

#include <sys/epoll.h>

#include "tcp_session_container.h"

class epoll_model {
public:
    epoll_model(tcp_session_container* container, command_dispatcher* dispatcher);
    ~epoll_model();

    bool enroll(int fd, uint32_t flag);
    bool modify(int fd, uint32_t flag);
    bool drop(int fd);

    int poll(int server_fd);

    void trigger_server_event(int server_fd);
    void trigger_event(const epoll_event& event);

    void close();
private:
    int _fd;

    epoll_event* _events;
    tcp_session_container* _session_container;
    command_dispatcher* _command_dispatcher;
};
