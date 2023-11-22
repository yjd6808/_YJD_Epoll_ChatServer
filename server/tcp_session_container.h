// 작성자: 윤정도

#pragma once
#include <unordered_map>
#include "tcp_session.h"

class tcp_session_container
{
public:
    tcp_session_container(int capacity) 
        : _capacity(capacity)
    {}

    tcp_session* get(int fd); 
    tcp_session* accept(int server_fd);

    void disconnect_all();
    void disconnect(int fd);

    int capacity() const { return _capacity; }
private:
    int _capacity; 
    std::unordered_map<int, tcp_session*> _session_map;
};
