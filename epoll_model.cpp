// 작성자: 윤정도

#include "epoll_model.h"
#include <unistd.h>

epoll_model::epoll_model(tcp_session_container* container, command_dispatcher* command_dispatcher) {
    _fd = epoll_create(container->capacity());
    _events = (epoll_event*)malloc(container->capacity() * sizeof(epoll_event));
    _session_container = container;
    _command_dispatcher = command_dispatcher;
}

epoll_model::~epoll_model() {
    free(_events);
}

bool epoll_model::enroll(int fd, uint32_t flag) {
    epoll_event event;
    event.data.fd = fd;
    event.events = flag;
    int ret = epoll_ctl(_fd, EPOLL_CTL_ADD, fd, &event);

    if (ret == -1) {
        fprintf(stderr, "epoll_ctl(EPOLL_CTL_ADD) error(%d):%s\n", errno, strerror(errno));
        return false;
    }

    return true;
}

bool epoll_model::modify(int fd, uint32_t flag) {
    epoll_event event;
    event.data.fd = fd;
    event.events = flag;
    int ret = epoll_ctl(_fd, EPOLL_CTL_MOD, fd, &event);

    if (ret == -1) {
        fprintf(stderr, "epoll_ctl(EPOLL_CTL_MOD) error(%d):%s\n", errno, strerror(errno));
        return false;
    }
    return true;
}

bool epoll_model::drop(int fd) {
    epoll_ctl(_fd, EPOLL_CTL_DEL, fd, nullptr);
    return true;
}

int epoll_model::poll(int server_fd) {
    int triggered_event_count = epoll_wait(_fd, _events, _session_container->capacity(), -1);

    for (int i = 0; i < triggered_event_count; ++i) {
        if (_events[i].data.fd == server_fd) {
            trigger_server_event(server_fd);
        } else {
            trigger_event(_events[i]);
        }
    }

    return triggered_event_count;
}

void epoll_model::trigger_server_event(int server_fd) {
    tcp_session* accepted_session = _session_container->accept(server_fd);
    if (accepted_session == nullptr) {
        return;
    }
    enroll(accepted_session->get_fd(), EPOLLIN | EPOLLRDHUP);
}

void epoll_model::trigger_event(const epoll_event& event) {
    int fd = event.data.fd;
    uint32_t flag = event.events;

    tcp_session* session = _session_container->get(fd);
    if (session == nullptr) {
        fprintf(stderr, "trigger_event error: cannot find session\n");
        return;
    }
    
    if (flag & EPOLLRDHUP) {
        fprintf(stderr, "EPOLLRDHUP: %s disconnected", session->get_addr_string().c_str());
        _session_container->disconnect(fd);
        return;
    }

    if (flag & EPOLLHUP) {
        fprintf(stderr, "EPOLLHUP: %s disconnected", session->get_addr_string().c_str());
        _session_container->disconnect(fd);
        return;
    }

    if (flag & EPOLLIN) {
        int processed_bytes = session->recv(); 
        if (processed_bytes == -1) { 
            fprintf(stderr, "EPOLLIN: %s disconnected", session->get_addr_string().c_str());
            _session_container->disconnect(fd);
            return;
        }

        session->dispatch_command(_command_dispatcher);
    }

    if (flag & EPOLLOUT) {
        int processed_bytes = session->send_pending();
        if (processed_bytes == -1) { 
            fprintf(stderr, "EPOLLOUT: %s disconnected", session->get_addr_string().c_str());
            _session_container->disconnect(fd);
            return;
        }

        if (session->has_send_pending()) {
           modify(fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP);
        }
    }

}

void epoll_model::close() {
    ::close(_fd);
    _fd = -1;
}



















