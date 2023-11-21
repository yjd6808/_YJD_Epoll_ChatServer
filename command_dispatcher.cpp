// 작성자: 윤정도

#include "command_dispatcher.h"
#include "tcp_session.h"

void command_dispatcher::register_event(t_command_id cmd_id, const t_command_event& command_event) {
    if (_event_map.insert(std::make_pair(cmd_id, command_event)).second == false) {
        fprintf(stderr, "register_event failed: command id %d already exist\n", cmd_id);
        return;
    }
}

void command_dispatcher::run_event(t_command_id cmd_id, tcp_session* session) {
    auto it = _event_map.find(cmd_id);
    if (it == _event_map.end()) {
        fprintf(stderr, "run_event failed: cannot find command id %d\n", cmd_id);
        return;
    }

    it->second(session, session->get_recv_buffer());
}
