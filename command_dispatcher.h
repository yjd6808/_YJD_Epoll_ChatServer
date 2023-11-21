// 작성자: 윤정도

#pragma once

#include <unordered_map>
#include <functional>

class tcp_session;
class stream_buffer_abstract;

using t_command_event = std::function<void(tcp_session*, stream_buffer_abstract*)>;
using t_command_id = int;
using t_command_len = int;

class command_dispatcher 
{
public:
    void register_event(t_command_id cmd_id, const t_command_event& command_event);
    void run_event(t_command_id cmd_id, tcp_session* session);
private:
    std::unordered_map<t_command_id, t_command_event> _event_map;
};

