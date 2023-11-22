#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <thread>

#include "../server/stream_buffer.h"
#include "../server/command_dispatcher.h"
#include "../server/cmdlist.h"

volatile bool running = true;
int fd = -1;

stream_buffer<6000> recv_buffer;
stream_buffer<6000> send_pending_buffer;

void print_menu();
bool select_menu(); // 메뉴 선택실패 혹은 프로그램 종료 선택시 false
void run_command(int cmd, stream_buffer_abstract* buffer); 
void send_command(int cmd);
void send_pending(); // 보내지 못했던 데이터 마저 전송
void recv_thread();
bool recv_command(); // 연결끊어진 경우 false 반환
void parse_command();

int main() {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    sockaddr_in endpoint;
    memset(&endpoint, 0, sizeof(sockaddr_in));
    endpoint.sin_port = htons(9999);
    endpoint.sin_family = AF_INET;
    endpoint.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (::connect(fd, (sockaddr*)&endpoint, sizeof(sockaddr_in)) == -1) {
        fprintf(stderr, "connect error(%d): %s\n", errno, strerror(errno));
        return -1;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        fprintf(stderr, "fcntl(F_GETFL) error(%d):%s\n", errno, strerror(errno));
        close(fd);
        return -2;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        fprintf(stderr, "fcntl(F_SETFL) error(%d):%s\n", errno, strerror(errno));
        close(fd);
        return -3;
    }

    std::thread recv_th(recv_thread);

    for (;;) {
        print_menu();
        
        if (!select_menu()) {
            break;
        }
    }

    running = false;
    recv_th.join();
    close(fd);
    printf("program terminated\n");
    return 0;
}


void print_menu() {
    printf("0. echo message\n");
    printf("1. chat message\n");
    printf("9. termniate program\n");
    printf("select> ");
}

bool select_menu() {
    int selected_menu;
    if (!(std::cin >> selected_menu) || selected_menu == 9) {
        return false;
    }

    send_command(selected_menu);
    return true;
}

void run_command(int cmd, stream_buffer_abstract* buffer) {
    if (cmd == CMDID_ECHO_MESSAGE) {
        int str_len = buffer->read_int();
        std::string str = buffer->read_string(str_len);

        printf("echo message: %s\n", str.c_str());
    } else if (cmd == CMDID_CHAT_MESSAGE) {
        int str_len = buffer->read_int();
        std::string str = buffer->read_string(str_len);

        printf("chat message: %s\n", str.c_str());
    }
}

void send_command(int cmd) {
    int cmd_len = -1;
    stream_buffer<1024> send_buffer;
    send_buffer.write_int(cmd);
    if (cmd == CMDID_ECHO_MESSAGE) {
        std::string msg;
        printf("write echo message> ");
        std::cin >> msg;

        cmd_len = sizeof(int) + msg.length();
        send_buffer.write_int(cmd_len);
        send_buffer.write_int(msg.length());
        send_buffer.write_string(msg);
    } else if (cmd == CMDID_CHAT_MESSAGE) {
        std::string msg;
        printf("write chat message> ");
        std::cin >> msg;

        cmd_len = sizeof(int) + msg.length();
        send_buffer.write_int(cmd_len);
        send_buffer.write_int(msg.length());
        send_buffer.write_string(msg);
    } else {
        fprintf(stderr, "invalid command id(%d)\n", cmd);
        return;
    }

    send_pending();

    for (;;) {
        char* cur_data = send_buffer.readable_data();
        int cur_send_len = send_buffer.readable_size();

        if (cur_send_len <= 0) {
            printf("send %d bytes\n", send_buffer.get_read_pos());
            break;
        }

        int send_bytes = ::send(fd, cur_data, cur_send_len, 0);
        if (send_bytes == -1) {
            if (errno == EWOULDBLOCK) {
                send_pending_buffer.write_bytes(cur_data, cur_send_len);
            } else {
                fprintf(stderr, "send error(%d):%s\n", errno, strerror(errno));
            }
            break;
        }
        send_buffer.move_read_pos(send_bytes);
    }
}

void send_pending() {
    if (send_pending_buffer.get_write_pos() == 0)
        return;

    int total_send_bytes = 0;
    for (;;) {
        char* readable_data = send_pending_buffer.readable_data(); 
        int readable_size = send_pending_buffer.readable_size();
        
        if (readable_size <= 0) {
            break;
        }

        int send_bytes = ::send(fd, readable_data, readable_size, 0);
        if (send_bytes == -1) {
            if (errno == EWOULDBLOCK) {
                send_pending_buffer.pop_reads();
                break;
            } else {
                fprintf(stderr, "send pending error(%d):%s\n", errno, strerror(errno));
                return;
            }
        }
        send_pending_buffer.move_read_pos(send_bytes);
        total_send_bytes += send_bytes;
    }

    if (total_send_bytes > 0) {
        printf("send %d bytes [pending]\n", total_send_bytes);
    }
}

void recv_thread() {
    while (running) {
        if (recv_command()) {
            parse_command();
        } else {
            break;
        }
    }

    printf("recv thread terminated\nenter any key to exit program.\n");
}

bool recv_command() {
    int total_recv_bytes = 0;

    for (;;) {
        char* buf = recv_buffer.writeable_data();
        int buf_size = recv_buffer.writeable_size();

        if (buf_size <= 0) {
            break;
        }

        int recv_bytes = ::recv(fd, buf, buf_size, 0);
        if (recv_bytes == 0) { return false; }
        if (recv_bytes == -1) {
            if (errno != EWOULDBLOCK) {
                fprintf(stderr, "recv error(%d):%s\n", errno, strerror(errno));
                return false;
            } else {
                break;
            }        
        }

        recv_buffer.move_write_pos(recv_bytes);
        total_recv_bytes += recv_bytes;
    }
         
    if (total_recv_bytes > 0) {
        printf("recv %d bytes\n", total_recv_bytes);
    }

    return true;
}


void parse_command() {
    int dispatched_command_count = 0;

    for (;;) {
        if (recv_buffer.readable_size() < HEADER_SIZE)
            break;

        t_command_len cmd_len = -1;
        memcpy(&cmd_len, recv_buffer.readable_data() + sizeof(t_command_id), sizeof(t_command_len));
        if (recv_buffer.readable_size() < HEADER_SIZE + cmd_len) {
            break;
        }
         
        t_command_id cmd_id = recv_buffer.read_int();
        if (cmd_id < 0) {
            fprintf(stderr, "dispatch_command error: unknown command id");
            recv_buffer.reset_pos();
            break;
        }

        recv_buffer.move_read_pos(sizeof(t_command_len));
        int expected_read_pos = recv_buffer.get_read_pos() + cmd_len;
        run_command(cmd_id, &recv_buffer);
        dispatched_command_count++;

        if (expected_read_pos != recv_buffer.get_read_pos()) {
            fprintf(stderr, "dispatch_command error: read_pos != expected_read_pos, please check command(%d) parser read buffer correctly\n", cmd_id);
            recv_buffer.set_read_pos(expected_read_pos);
        }
    }

    if (dispatched_command_count > 0) {
        if (recv_buffer.readable_size() == recv_buffer.writeable_size()) {
            recv_buffer.reset_pos();
        } else {
            recv_buffer.pop_reads();
        }
    }

}
