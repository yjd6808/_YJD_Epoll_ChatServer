// 작성자: 윤정도

#pragma once

#include "stream_buffer.h"
#include "command_dispatcher.h"

#include <arpa/inet.h>

class tcp_session 
{
public:
    tcp_session(int fd);
    int send(char* data, int send_len);
    int send_pending();

    int recv();
    void dispatch_command(command_dispatcher* dispatcher);

    void disconnect();
    bool has_send_pending();
    void set_addr(const sockaddr_in& addr);
    std::string get_addr_string();

    bool set_non_bloking(bool enabled);
    int get_fd() const { return _fd; }

    stream_buffer_abstract* get_recv_buffer() { return &_recv_buffer; }
private:
    int _fd;
    sockaddr_in _addr;

    stream_buffer<6000> _recv_buffer;
    stream_buffer<6000> _send_pending_buffer;
};
