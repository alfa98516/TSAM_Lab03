#include <arpa/inet.h>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

/**
 * @brief Main function that scans a given IPv4 address for open UDP ports in
 * the specified range.
 * @param argc Number of args, must be 4.
 * @param argv Argument vector, where: argv[1] is the IPv4 address, argv[2] is
 * the low port, and argv[3] is the high port.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, const char* argv[]) {
    // Make sure we have exactly four arguments
    if (argc != 4) {
        std::cerr << "Usage:" << argv[0]
                  << " <IPv4 address> <low port> <high port>\n";
        return 1;
    }

    const std::string payload = "hi"; // Some example payload we'll send.

    const char* ip_addr = argv[1];
    // Try to convert low_port and high_port to integers, catch if not possible.
    const char* argv2 = argv[2];
    // Lambda function to convert argv2 to an integer and handle exceptions.
    const int low_port = [argv2]() {
        try {
            return std::stoi(argv2);
        } catch (const std::invalid_argument&) {
            std::cerr << argv2 << "Argument 2 must be of type integer\n";
        }
        return -1;
    }();

    const char* argv3 = argv[3];
    const int high_port = [argv3]() {
        try {
            return std::stoi(argv3);
        } catch (const std::invalid_argument&) {
            std::cerr << argv3 << "Argument 3 must be of type integer\n";
        }
        return -1;
    }();

    // Check if the port numbers are in the valid range.
    if ((low_port < 0 || low_port > 65535) ||
        (high_port > 65535 || high_port < 0)) {
        std::cerr << "Error: Port numbers must be in range [0, 65535]\n";
        if (low_port < 0 || low_port > 65535) {
            std::cerr << "  Low port " << low_port << " is out of range\n";
        }
        if (high_port < 0 || high_port > 65535) {
            std::cerr << "  High port " << high_port << " is out of range\n";
        }
        return 1;
    }

    if (low_port > high_port) {
        std::cerr
            << "First specified port number must be lower than the second\n";
        std::cerr << "Did you mean: " << high_port << " " << low_port << "?\n";
        return 1;
    }

    timeval timeout{};
    timeout.tv_sec = 1; // Time to wait in seconds for each scan before timeout.

    const int port_count = high_port - low_port + 1;
    bool open_ports[port_count]; // The set of found open ports.
    memset(open_ports, 0, sizeof(open_ports));

    struct sockaddr_in dest_addr{}; // The destination address struct.

    dest_addr.sin_family = AF_INET; // Set its address family to IPv4.
    // Validate IPv4 address and convert it to binary form.
    int inet_pton_result = inet_pton(AF_INET, ip_addr, &dest_addr.sin_addr);

    // Check if the address is valid.
    if (inet_pton_result == 0) {
        std::cerr << "Invalid IPv4 address: " << ip_addr << '\n';
        return 1;
    }

    // Check if there was an error in converting the IPv4 address.
    if (inet_pton_result < 0) {
        perror("inet_pton");
        return 1;
    }

    // Create a UDP socket with the IPv4 address family.
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    // Check if the socket was created successfully.
    if (socket_fd < 0) {
        perror("Error creating socket!");
        close(socket_fd);
        return 1;
    }
    // Set the receive timeout for the socket.
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                   sizeof(timeout)) < 0) {
        close(socket_fd);
        return 1;
    }

    // Set socket options.
    char data_buffer[2048]; // Buffer to hold received data, source address,
                            // and its length.
    struct sockaddr_in src_addr{}; // The source address struct.
    socklen_t src_addr_len = sizeof(src_addr);
    // Scan given IP address for open UDP ports in the given range.
    for (int curr_port = low_port; curr_port <= high_port; curr_port++) {
        // Print current progress with a progress bar on the same line.
        double progress = 100.0 * (curr_port - low_port + 1) / port_count;

        dest_addr.sin_port = htons(curr_port);

        // Send the payload to the destination address and port 5 times to
        // increase the chance of getting a response.
        for (int i = 0; i < 5; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (sendto(socket_fd, payload.c_str(), payload.length(), 0,
                       (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
                continue;
            }
        }
    }

    // Receive responses from the scanned ports until a timeout occurs.
    while (true) {
        ssize_t n_bytes_received =
            recvfrom(socket_fd, data_buffer, sizeof(data_buffer), 0,
                     (struct sockaddr*)&src_addr, &src_addr_len);
        // Check if the recvfrom() call was successful.
        if (n_bytes_received < 0) {
            // Check if the error was due to a timeout or a non-blocking
            // socket.
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            perror("recvfrom"); // A non-timeout error.
            continue;
        } else {
            // Got a successful response.
            open_ports[ntohs(src_addr.sin_port) - low_port] = true;
            continue;
        }
    }
    close(socket_fd);
    // Print the list of open ports found during the scan.
    for (int i = 0; i < port_count; i++) {
        if (open_ports[i]) {
            std::cout << i + low_port << '\n';
        }
    }
}
