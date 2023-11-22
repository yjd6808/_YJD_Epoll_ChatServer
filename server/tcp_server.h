// 작성자: 윤정도

#pragma once
#include "epoll_model.h"

class tcp_server 
{ 
public:
    tcp_server(int capacity);
    void setup(int port);
    void poll_events();
    void close();

    void broadcast(stream_buffer_abstract* buffer);
private:
    int _fd;
    tcp_session_container _session_container;
    command_dispatcher _command_dispatcher;
    epoll_model _epoll;
};
