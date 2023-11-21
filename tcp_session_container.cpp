// 작성자: 윤정도

#include "tcp_session_container.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

tcp_session* tcp_session_container::get(int fd) {
    auto it = _session_map.find(fd);
    if (it == _session_map.end())
        return nullptr;

    return it->second;
}

tcp_session* tcp_session_container::accept(int server_fd) {
    if (_session_map.size() >= size_t(_capacity)) {
        fprintf(stderr, "accept failed: container is full(%d)\n", _capacity);
        return nullptr;
    }

    sockaddr_in addr;
    socklen_t addr_size = sizeof(sockaddr_in);
    
    int accepted_fd = ::accept(server_fd, (sockaddr*)&addr, &addr_size);

    if (accepted_fd == -1) {
        fprintf(stderr, "accept error(%d):%s\n", errno, strerror(errno));
        return nullptr;
    }

    tcp_session* session = new tcp_session(accepted_fd);
    session->set_addr(addr);
    printf("accept: %s connected", session->get_addr_string().c_str());
    _session_map.insert(std::make_pair(accepted_fd, session));
    return session;
}

void tcp_session_container::disconnect(int fd) {
    tcp_session* session = get(fd);
    if (session == nullptr) {
        fprintf(stderr, "disconnect failed: cannot find session\n");
        return;
    }

    _session_map.erase(fd);
    session->disconnect();
    printf("disconnect: %s disconnected", session->get_addr_string().c_str());
    delete session;
}

void tcp_session_container::disconnect_all() {
    for (auto it = _session_map.begin(); it != _session_map.end(); ++it) {
        tcp_session* session = it->second;
        session->disconnect();
        delete session;
    }

    _session_map.clear();
}
