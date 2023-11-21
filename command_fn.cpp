// 작성자: 윤정도

#include "command_fn.h"

void command_fn::echo_message(tcp_session* session, stream_buffer_abstract* buffer) {
    int str_len = buffer->read_int();
    std::string str = buffer->read_string(str_len);
    printf("echo_message: %s\n", str.c_str());

    stream_buffer<32> send_buffer;
    buffer->write_int(CMDID_ECHO_MESSAGE);
    buffer->write_int(str_len);
    buffer->write_string(str);
    session->send(send_buffer.readable_data(), send_buffer.get_write_pos());
}
