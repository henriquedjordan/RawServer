#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080

#define BUFFER 40000

int main() {

    int server_fd, new_socket;

    struct sockaddr_in address;

    socklen_t address_len = sizeof(address);

    int opt = 1;

    int sn_server_response;

    char server_response[BUFFER];

    char client_request[BUFFER];

    const char *html;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("SOCKET CREATION FAILED!:\n");
        exit(EXIT_FAILURE);
    }

    if ((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) < 0) {
        perror("IP/PORT CONFLICT!:\n");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if ((bind(server_fd, (struct sockaddr*)&address, address_len)) < 0) {
        perror("BIND FAIED!:\n");
        exit(EXIT_FAILURE);
    }

    if ((listen(server_fd, 3)) < 0) {
        perror("LISTEN FAILED!:\n");
        exit(EXIT_FAILURE);
    }

    std::cout << "\nListen on port 8080!\n" << std::endl;

    html = 
    "<!DOCTYPE html>"
    "<html>"
    "<head><title>Manual HTTP</title></head>"
    "<body><h4>HTTP server running at port 8080!</h4></body>"
    "</html>";

    sn_server_response = snprintf(server_response, sizeof(server_response), 
    "HTTP/1.1 200 OK\r\n"
    "Content-type: text/html; charset=utf-8\r\n"
    "Content-length: %zu\r\n"
    "\r\n"
    "%s",
    strlen(html), html);

    if (sn_server_response < 0) {
        perror("FORMATING ERROR!:\n");
        exit(EXIT_FAILURE);
    }

    while (true) {

        std::cout << "Waiting for connections...\n" << std::endl;

        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &address_len)) < 0) {
            perror("ACCEPT FAILED!:\n");
            exit(EXIT_FAILURE);
        }

        memset(client_request, 0, sizeof(client_request));

        if(recv(new_socket, client_request, sizeof(client_request), 0) < 0) {
            perror("RECEIVE FAILED!:\n");
            exit(EXIT_FAILURE);
        }

        std::cout << "RECEIVED REQUEST:\n" << client_request << std::endl;

        std::string request(client_request);

        size_t line_end = request.find("\n\r");
        std::string request_line = request.substr(0, line_end);

        size_t first_space = request_line.find(" ");
        size_t second_space = request_line.find(" ", first_space + 1);

        std::string method = request_line.substr(0, first_space);
        std::string path = request_line.substr(first_space + 1, second_space - first_space - 1);
        std::string http_version = request_line.substr(second_space + 1);

        if (send(new_socket, server_response, strlen(server_response), 0) < 0) {
            perror("SEND FAILED!:\n");
            exit(EXIT_FAILURE);
        }
    }

    close(server_fd);

    return 0;
}