#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

int main() {
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
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // accept connections from any IP
    address.sin_port = htons(8080);        // Port 8080, htons() -> Big-endian

    // bind a socket to an addres and port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Binding error.\n";
        return 1;
    }

    // switch the socket to kistining mode
    if (listen(server_fd, 5) < 0) {  // size of queue of pending connections
        std::cerr << "Error listen.\n";
        return 1;
    }

    std::cout << "Server listening on port 8080...\n";

    while (true) {
        // accepting incoming connection
        int client_socket;
        socklen_t addrlen = sizeof(address);
        client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (client_socket < 0) {
            std::cerr << "Error accept.\n";
            continue;
        }

        char buffer[1024] = {0};
        read(client_socket, buffer, 1024);
        std::cout << "Recieve request:\n" << buffer << "\n";

        const char* response =
            "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nHello, World!";
        send(client_socket, response, strlen(response), 0);

        close(client_socket);
    }
    close(server_fd);
}