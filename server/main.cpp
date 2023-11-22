// 작성자: 윤정도

#include <iostream>
#include <thread>

#include "tcp_server.h"

#define SERVER_PORT                 9999
#define SERVER_SESSION_COUNT        50

volatile bool running = true;
tcp_server server(SERVER_SESSION_COUNT);

void cli_routine() {
    for (;;) {
        std::string read;
        std::cin >> read;
        if (read == "exit") {
            running = false;
            break;
        }
    }
}

int main() {
    std::thread cli_thread(cli_routine);
    server.setup(SERVER_PORT);
    while (running) {
        server.poll_events();
    }
    cli_thread.join();
    server.close();
    return 0;
}
