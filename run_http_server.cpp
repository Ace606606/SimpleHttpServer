#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

constexpr int16_t PORT = 8080;
constexpr int16_t SIZE_BUFFER = 1024;

volatile sig_atomic_t stop_flag = 0;

void handle_sigint(int) { stop_flag = 1; }

void setup_signal_handler() {
    struct sigaction sa{};
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, nullptr) < 0) {
        std::cerr << "Error setting signal handler: " << strerror(errno)
                  << "(code: " << errno << ")\n";
        std::exit(1);
    }
}

int main() {
    setup_signal_handler();

    /*
    Create socket(endpoint):
    AF_INET - IPv4, AF_INET6 - IPv6
    SOCK_STREAM - tcp, SOCK_DGRAM - udp
    0 - default
    */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Error creating socket.\n";
        return 1;
    }

    // setting up the address server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // accept connections from any IP
    server_addr.sin_port = htons(PORT);  // Port 8080, htons() -> Big-endian

    // bind a socket to an addres and port
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) <
        0) {
        std::cerr << "Binding error.\n";
        close(server_fd);
        return 1;
    }

    // switch the socket to kistining mode
    if (listen(server_fd, 5) < 0) {  // size of queue of pending connections
        std::cerr << "Error listen.\n";
        close(server_fd);
        return 1;
    }

    std::cout << "Server listening on port 8080...\n";

    while (!stop_flag) {
        // accepting incoming connection
        struct sockaddr_in client_addr;
        int client_socket;
        socklen_t addr_client_len = sizeof(client_addr);

        client_socket =
            accept(server_fd, (struct sockaddr*)&client_addr, &addr_client_len);

        if (client_socket < 0) {
            if (errno == EINTR && stop_flag) {
                std::cout << "\nSignal received, shutting down...\n";
                break;
            }
            std::cerr << "Accept error: " << strerror(errno) << "\n";
            errno = 0;
            continue;
        }

        char buffer[SIZE_BUFFER] = {0};
        ssize_t bytes_read = read(client_socket, buffer, SIZE_BUFFER - 1);
        if (bytes_read < 0) {
            std::cerr << "Error reading from client socket.\n";
            close(client_socket);
            continue;
        } else if (bytes_read == 0) {
            std::cout << "Client disconnected.\n";
            continue;
        }
        buffer[bytes_read] = '\0';
        std::cout << "Recieve request:\n" << buffer << "\n";

        const char* body = "Hello, World!";
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: " +
            std::to_string(strlen(body)) +
            "\r\n"
            "\r\n" +
            body;

        send(client_socket, response.c_str(), response.length(), 0);

        close(client_socket);
    }

    std::cout << "\nServer shutdown...\n";
    close(server_fd);
}