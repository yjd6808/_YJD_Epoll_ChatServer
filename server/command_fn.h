// 작성자: 윤정도
 
#pragma once

#include "tcp_session.h"

#define CMDID_ECHO_MESSAGE      0

struct command_fn 
{
    static void echo_message(tcp_session* session, stream_buffer_abstract* buffer);

};
