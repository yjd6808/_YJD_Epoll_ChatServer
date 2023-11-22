// 작성자: 윤정도

#include "tcp_session.h"

#include <fcntl.h>
#include <errno.h>
#include <sstream>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

tcp_session::tcp_session(int fd)
    : _fd(fd)
{}

int tcp_session::send(char* data, int send_len) {
    int total_send_bytes = 0;

    for (;;) {
        char* cur_data = data + total_send_bytes;
        int cur_send_len = send_len - total_send_bytes;

        int send_bytes = ::send(_fd, cur_data, cur_send_len, 0);
        if (send_bytes == -1) {
            if (errno == EWOULDBLOCK) {
                _send_pending_buffer.write_bytes(cur_data, cur_send_len);
                break;
            } else {
                fprintf(stderr, "send error(%d):%s\n", errno, strerror(errno));
                return -1;
            }
        }
        total_send_bytes += send_bytes;

        if (total_send_bytes >= send_len)
            break;
    }
    return total_send_bytes;
}

int tcp_session::send_pending() {
    if (has_send_pending())
        return 0;

    int total_send_bytes = 0;

    for (;;) {
        char* readable_data = _send_pending_buffer.readable_data(); 
        int readable_size = _send_pending_buffer.readable_size();
        
        if (readable_size <= 0) {
            break;
        }

        int send_bytes = ::send(_fd, readable_data, readable_size, 0);
        if (send_bytes == -1) {
            if (errno == EWOULDBLOCK) {
                _send_pending_buffer.pop_reads();
                break;
            } else {
                fprintf(stderr, "send pending error(%d):%s\n", errno, strerror(errno));
                return -1;
            }
        }

        total_send_bytes += send_bytes;
        _send_pending_buffer.move_read_pos(send_bytes);
    }

    return total_send_bytes;
}

int tcp_session::recv() {
    int total_recv_bytes = 0;

    for (;;) {
        char* buf = _recv_buffer.writeable_data();
        int buf_size = _recv_buffer.writeable_size();

        if (buf_size <= 0) {
            break;
        }

        int recv_bytes = ::recv(_fd, buf, buf_size, 0);
        if (recv_bytes == 0) { return -1; }
        if (recv_bytes == -1) {
            if (errno != EWOULDBLOCK) {
                fprintf(stderr, "recv error(%d):%s\n", errno, strerror(errno));
            }
            break;
        }

        _recv_buffer.move_write_pos(recv_bytes);
        total_recv_bytes += recv_bytes;
    }
        
    return total_recv_bytes;
}

void tcp_session::dispatch_command(command_dispatcher* dispatcher) {
    int dispatched_command_count = 0;

    for (;;) {
        if (_recv_buffer.readable_size() < HEADER_SIZE)
            break;

        t_command_len cmd_len = -1;
        memcpy(&cmd_len, _recv_buffer.readable_data() + sizeof(t_command_id), sizeof(t_command_len));
        if (_recv_buffer.readable_size() < HEADER_SIZE + cmd_len) {
            break;
        }
         
        t_command_id cmd_id = _recv_buffer.read_int();
        if (cmd_id < 0) {
            fprintf(stderr, "dispatch_command error: unknown command id");
            _recv_buffer.reset_pos();
            break;
        }

        _recv_buffer.move_read_pos(sizeof(t_command_len));
        int expected_read_pos = _recv_buffer.get_read_pos() + cmd_len;
        dispatcher->run_event(cmd_id, this);
        dispatched_command_count++;

        if (expected_read_pos != _recv_buffer.get_read_pos()) {
            fprintf(stderr, "dispatch_command error: read_pos != expected_read_pos, please check command(%d) parser read buffer correctly\n", cmd_id);
            _recv_buffer.set_read_pos(expected_read_pos);
        }
    }

    if (dispatched_command_count > 0) {
        if (_recv_buffer.readable_size() == _recv_buffer.writeable_size()) {
            _recv_buffer.reset_pos();
        } else {
            _recv_buffer.pop_reads();
        }
    }
}

void tcp_session::disconnect() {
    close(_fd);
    _fd = -1;
}

bool tcp_session::has_send_pending() {
    return _send_pending_buffer.readable_size() > 0;
}

void tcp_session::set_addr(const sockaddr_in& addr) {
    _addr = addr;
}

std::string tcp_session::get_addr_string() {
    char addr_buf[128];
    inet_ntop(AF_INET, &_addr.sin_addr, addr_buf, sizeof(addr_buf));
    std::stringstream ss;
    ss << addr_buf;
    ss << ':';
    ss << ntohs(_addr.sin_port);
    return ss.str();
}

bool tcp_session::set_non_bloking(bool enabled) {
     int flags = fcntl(_fd, F_GETFL, 0);
     if (flags == -1) return false;
     flags = enabled ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
     return (fcntl(_fd, F_SETFL, flags) == 0) ? true : false;
}



