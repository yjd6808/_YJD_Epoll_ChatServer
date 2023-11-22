// 작성자: 윤정도
 
#pragma once

#include "tcp_session.h"

struct command_fn 
{
    static void echo_message(tcp_session* session, stream_buffer_abstract* buffer);
};
