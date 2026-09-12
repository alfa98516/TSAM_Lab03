#include <arpa/inet.h>
#include <array>
#include <cerrno>
#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

namespace {

/**
 * @brief Parses a command line argument as an integer port number.
 * @param argument The command line argument to parse.
 * @param argument_number The index of the argument in argv[].
 * @return The parsed port number, or -1 if the argument is invalid.
 */
int parse_port(const char* argument, int argument_number) {
    try {
        return std::stoi(argument);
    } catch (const std::invalid_argument&) {
        std::cerr << "Argument " << argument_number << " must be an integer\n";
    } catch (const std::out_of_range&) {
        std::cerr << "Argument " << argument_number << " is too large\n";
    }
    return -1;
}
// TODO: Remove this and implement actual recieved function
// --------------------------------------
const std::array<std::string, 4> dummy_recieved = {
    "Greetings, adventurer, from S.E.C.R.E.T. (Sacred Elder Cipher Relay "
    "jafdslæfjasldkægjsa ",
    "The dark arts of network programming lead to powers some consider to "
    "dkfjasdælkgnadsælg ",
    "I am the guardian of the secret spell. The lords of the network do "
    "gahjfdagkjfaklægjdsæl",
    "Hail, traveler! I am D.R.A.G.O.N. - the Dwemer Relay Apparatus for "
    "lkdsgajgælkasjdglækasjdg"};

std::string dummy_recieve(int index) {

    return dummy_recieved[index];
}
// ----------------------------------------------------------------------------

/**
 * @brief Scans the given IPv4 address for open UDP ports in the specified
 * range.
 * @param ip_addr The IPv4 address to scan.
 * @param low_port The lower bound of the port range to scan.
 * @param high_port The upper bound of the port range to scan.
 * @return 0 on success, 1 on failure.
 */
std::string get_port_message(const char* ip_addr, int port_num) {
    const std::string payload = "TSAM_PAYLOAD";

    timeval timeout{};
    timeout.tv_sec = 1; // Time to wait in seconds.

    struct sockaddr_in dest_addr{}; // Destination IPv4 address.
    dest_addr.sin_family = AF_INET; // Set address family to IPv4.
    // Validate IP.
    int inet_pton_result = inet_pton(AF_INET, ip_addr, &dest_addr.sin_addr);

    // Check if the address is valid.
    if (inet_pton_result == 0) {
        std::cerr << "Invalid IPv4 address: " << ip_addr << '\n';
        return {};
    }

    // Check if there was an error in converting the IPv4 address.
    if (inet_pton_result < 0) {
        perror("inet_pton");
        return {};
    }

    // Create UDP socket.
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    // Validate socket creation.
    if (sock < 0) {
        perror("Error creating socket!");
        close(sock);
        return {};
    }
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) <
        0) {
        close(sock);
        return {};
    }

    char data_buffer[2048];
    struct sockaddr_in src_addr{};
    socklen_t src_addr_len = sizeof(src_addr);

    // Send 5 packets to the port.
    for (int i = 0; i < 5; i++) {
        if (sendto(sock, payload.c_str(), payload.length(), 0,
                   (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
            continue;
        }
    }
    std::string returned_message;
    while (true) {
        ssize_t n_bytes_received =
            recvfrom(sock, data_buffer, sizeof(data_buffer), 0,
                     (struct sockaddr*)&src_addr, &src_addr_len);
        if (n_bytes_received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            perror("recvfrom");
            continue;
        } else {
            returned_message = ntohs(src_addr.sin_port);
            break;
        }
    }
    close(sock);

    return returned_message;
}

/**
 * @brief Simple send recieve loop, sends 5 packets to an IP address and port,
 * and then returns the message recieved.
 *
 * @param ip_addr: The IP address you want to send to.
 * @param port: The port you want to send to.
 * @param msg: The message you want to send to the port.
 * @returns: A string
 */
std::string send_recv(sockaddr_in& ip_addr, int port, const std::string& msg) {
    if (port < 0 || port > 65535) {
        std::cerr << "Port numbers range between 0 and 65535\n";
        return "ERROR";
    }

    timeval timeout{};
    timeout.tv_sec = 1;

    struct sockaddr_in dest_addr{};
    dest_addr.sin_family = AF_INET;

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("Error Creating socket!");
        close(socket_fd);
        return "ERROR";
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                   sizeof(timeout)) < 0) {
        close(socket_fd);
        return "ERROR";
    }

    char data_buffer[2048];
    socklen_t src_addr_len = sizeof(ip_addr);

    for (int i = 0; i < 5; i++) {
        dest_addr.sin_port = htons(port);
        if (sendto(socket_fd, msg.c_str(), msg.length(), 0,
                   (struct sockaddr*)&dest_addr, src_addr_len) < 0) {
            continue;
        }
    }

    while (true) {
        ssize_t nbytes_recieved =
            recvfrom(socket_fd, data_buffer, sizeof(data_buffer), 0,
                     (struct sockaddr*)&ip_addr, &src_addr_len);
        if (nbytes_recieved < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cout << "No response from port " << port << '\n';
                break;
            }
            continue;
        } else {
            return std::string(data_buffer, nbytes_recieved);
        }
    }
    return "NO_RESPONSE";
}

/**
 * @brief Functions that assigns each port to the given problem
 * @return returns an array which is indexed with each problem,
 * example; the port with problem a is in index 0 of the array,
 * problem b is in index 1, etc.
 */
std::array<std::string, 4> assign_problems_to_port() {

    std::array<std::string, 4> port_messages = {
        "Greetings, adventurer, from S.E.C.R.E.T. (Sacred Elder Cipher Relay ",
        "The dark arts of network programming lead to powers some consider to ",
        "I am the guardian of the secret spell. The lords of the network do ",
        "Hail, traveler! I am D.R.A.G.O.N. - the Dwemer Relay Apparatus for "};

    // Initialize return array
    std::array<std::string, 4> problem_ports;

    for (int i = 0; i < 4; i++) {
        // TODO: needs changing to actual recieve from function thats yet to be
        // implemented
        std::string recieved = dummy_recieve(i);

        // If the there isn't a match, the find function will return the null
        // position,
        if (recieved.find(port_messages[i]) != std::string::npos) {
            problem_ports[i] = recieved;
        }
    }
    return problem_ports;
}

void solve_puzzle_a() {}
void solve_puzzle_b() {}
void solve_puzzle_c() {}
void solve_puzzle_d() {}

} // namespace

/**
 * @brief Main function that scans a given IPv4 address for open UDP ports in
 * the specified range.
 * @param argc Number of args, must be 4.
 * @param argv Argument vector, where: argv[1] is the IPv4 address, argv[2] is
 * the low port, and argv[3] is the high port.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, const char* argv[]) {
    if (argc != 6) {
        std::cerr << "Usage:" << argv[0]
                  << " <IPv4 address> <port 1> <port 2> <port 3> <port 4>\n";
        return 1;
    }

    const char* ip_addr = argv[1];

    std::array<int, 4> input_ports;

    for (int i = 0; i < 4; i++) {
        int curr_port = parse_port(argv[i], 2);
    }

    std::array<std::string, 4> problem_ports = assign_problems_to_port();
}
