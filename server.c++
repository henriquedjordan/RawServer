#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080

#define BUFFER 40000

bool ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size()
    &&
    str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int main() {

    int server_fd, new_socket;

    struct sockaddr_in address;

    socklen_t address_len = sizeof(address);

    int opt = 1;

    char client_request[BUFFER];

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

    while (true) {

        std::cout << "Waiting for connections...\n" << std::endl;

        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &address_len)) < 0) {
            perror("ACCEPT FAILED!:\n");
            exit(EXIT_FAILURE);
        }

        memset(client_request, 0, sizeof(client_request));

        if (recv(new_socket, client_request, sizeof(client_request), 0) < 0) {
            perror("RECEIVE FAILED!:\n");
            exit(EXIT_FAILURE);
        }

        std::cout << "RECEIVED REQUEST:\n" << client_request << std::endl;

        std::string request(client_request);

        size_t line_end = request.find("\r\n");
        std::string request_line = request.substr(0, line_end);

        size_t first_space = request_line.find(" ");
        size_t second_space = request_line.find(" ", first_space + 1);

        std::string method = request_line.substr(0, first_space);
        std::string path = request_line.substr(first_space + 1, second_space - first_space - 1);

        if (method == "GET") {

            std::string file_path;

            if (path == "/")
                file_path = "index.html";
            else
                file_path = "." + path;

            std::ifstream file(file_path, std::ios::binary);

            if (!file.is_open()) {

                perror("NOT FOUND!:");
                const char *status_404 = 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-type: text/plain\r\n"
                "Content-length: 13\r\n"
                "\r\n"
                "404 Not Found!";

                if ((send(new_socket, status_404, strlen(status_404), 0)) < 0) {
                    perror("SEND FAILED!:\n");
                    exit(EXIT_FAILURE);
                }

                continue;
            }

            std::ostringstream buffer_stream;
            buffer_stream << file.rdbuf();
            std::string body = buffer_stream.str();
            file.close();

            std::string content_type = "text/plain";
            if (ends_with(file_path, ".html")) content_type = "text/html";
            else if (ends_with(file_path, ".css")) content_type = "text/css";
            else if (ends_with(file_path, ".js")) content_type = "application/javascript";
            else if (ends_with(file_path, ".json")) content_type = "application/json";
            else if (ends_with(file_path, ".png")) content_type = "image/png";
            else if (ends_with(file_path, ".jpg") || ends_with(file_path, ".jpeg")) content_type = "image/jpeg";

            std::string response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-type: " + content_type + "\r\n"
            "Content-length: " + std::to_string(body.size()) + "\r\n"
            "\r\n" + body;

            if ((send(new_socket, response.c_str(), response.size(), 0)) < 0) {
                perror("SEND FAILED!:");
                exit(EXIT_FAILURE);
            }   
        }

        const char* status_405 = 
        "HTTP/1.1 405 Method Not Allowed\r\n"
        "Content-type: text/plain\r\n"
        "Content-length: 23\r\n"
        "\r\n"
        "405 Method Not Allowed!";

        if ((send(new_socket, status_405, strlen(status_405), 0)) < 0) {
            perror("SEND FAILED!:\n");
            exit(EXIT_FAILURE);
        }

        close(new_socket);
    }

    close(server_fd); 
    return 0;
}