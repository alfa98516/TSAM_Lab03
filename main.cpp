#include <arpa/inet.h>
#include <array>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <utility>

uint32_t sigil;
uint8_t group_id;

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
std::string send_recv(sockaddr_in& ip_addr, int port, const char* msg) {
    constexpr std::size_t max_msg_length = 2048;
    if (port < 0 || port > 65535) {
        std::cerr << "Port numbers range between 0 and 65535\n";
        return "ERROR";
    }
    if (msg == nullptr) {
        std::cerr << "Message is null\n";
        return "ERROR";
    }

    const std::size_t msg_length = strnlen(msg, max_msg_length);
    if (msg_length == max_msg_length) {
        std::cerr << "Message is not null-terminated within the allowed size\n";
        return "ERROR";
    }

    timeval timeout{};
    timeout.tv_sec = 1;

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
        ip_addr.sin_port = htons(port);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (sendto(socket_fd, msg, msg_length, 0, (struct sockaddr*)&ip_addr,
                   src_addr_len) < 0) {
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
            if (nbytes_recieved >= 2048) {
                return "ERROR";
            }
            return std::string(data_buffer, nbytes_recieved);
        }
    }
    return "NO_RESPONSE";
}

/*
Greetings, adventurer, from S.E.C.R.E.T. (Sacred Elder Cipher Relay for
Enchanted Transmissions)!
Here are the rites required to gain access to the secret I guard:
1. Forge a 32-bit secret number, and keep it safe for later use.
2. Send me a message beginning with "S.E.C.R.E.T.:", followed by
   a comma-separated list of the names of every member of your
   adventuring party as your institution named them,
   and your secret number.
   Your secret number must occupy the final 4 bytes of the message.
3. I shall answer with a 5-byte missive: the first byte bears your group ID,
   while the remaining 4 bytes contain a challenge number.
4. Combine this challenge with your secret number using the XOR spell.
   The result shall be your 4-byte enchanted sigil.
5. Now send a 5-byte message: place your group number in the first byte,
   followed by the 4-byte sigil.
6. Should your sigil prove true, I shall reveal a hidden secret. May the
   Divines favor your quest!
7. Keep your group ID and sigil safe for future trials, for other ports
   shall require them. But beware: do not write them in stone!
*/
void solve_puzzle_secret(sockaddr_in& ip_addr, int port) {
    constexpr uint32_t secret_number = (1u << 31) - 1;
    // fun fact, this is a prime,
    // More formally, this is the eight Mersenne prime,
    // Where Mersenne prime is the collection of primes of the form 2^n - 1
    char payload[1024] = "S.E.C.R.E.T.:alfar24,gislih24,hlynurh24";
    int offset = 39;
    payload[offset] = (secret_number >> 24) & 0xFF;
    payload[offset + 1] = (secret_number >> 16) & 0xFF;
    payload[offset + 2] = (secret_number >> 8) & 0xFF;
    payload[offset + 3] = secret_number & 0xFF;

    std::string response = send_recv(ip_addr, port, payload);
    const char* cstr = response.c_str();
    group_id = cstr[0];
    std::cout << response.length() << '\n';
    std::cout << (int)group_id << '\n';
    char challenge_chr[4];
    uint32_t challenge_int;
    challenge_chr[0] = cstr[1];
    challenge_chr[1] = cstr[2];
    challenge_chr[2] = cstr[3];
    challenge_chr[3] = cstr[4];
    memcpy(&challenge_int, challenge_chr, 4);
    std::cout << challenge_int << '\n';
    sigil = challenge_int ^ secret_number;
    const char* sigil_chr = reinterpret_cast<const char*>(&sigil);
    char payload2[1024];

    payload2[0] = group_id;
    payload2[1] = sigil_chr[0];
    payload2[2] = sigil_chr[1];
    payload2[3] = sigil_chr[2];
    payload2[4] = sigil_chr[3];
    payload2[5] = '\0';
    std::string msg = send_recv(ip_addr, port, payload);
    std::cout << msg << '\n';
}

void solve_puzzle_evil() {}
void solve_puzzle_guardian() {}
void solve_puzzle_dragon() {}

/**
 * @brief Functions that assigns each port to the given problem
 * @return returns an array which is indexed with each problem,
 * example; the port with problem a is in index 0 of the array,
 * problem b is in index 1, etc.
 */
void assign_puzzle_to_port(sockaddr_in& ip_addr,
                           const std::array<int, 4>& ports) {

    std::array<std::string, 4> port_messages = {
        "Greetings, adventurer, from S.E.C.R.E.T. (Sacred Elder Cipher Relay ",
        "The dark arts of network programming lead to powers some consider to ",
        "I am the guardian of the secret spell. The lords of the network do ",
        "Hail, traveler! I am D.R.A.G.O.N. - the Dwemer Relay Apparatus for "};

    // Initialize return array

    std::array<std::pair<std::string, int>, 4> puzzle_ports;

    for (int i = 0; i < 4; i++) {
        // TODO: needs changing to actual recv_from function that's yet to be
        // implemented.

        std::string recieved_msg =
            send_recv(ip_addr, ports[i], "fartss"); // ssss
        if (recieved_msg == "ERROR" || recieved_msg == "NO_RESPONSE") {
            continue;
        }
        // If the there isn't a match, then find function will return the null
        // position.
        if (recieved_msg.find(port_messages[0]) != std::string::npos) {
            solve_puzzle_secret(ip_addr, ports[i]);
        } else if (recieved_msg.find(port_messages[1]) != std::string::npos) {
            solve_puzzle_evil();
        }
    }
}

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

    for (int i = 2; i < 6; i++) {
        int curr_port = parse_port(argv[i], 2);
        input_ports[i - 2] = curr_port;
    }

    struct sockaddr_in dest_addr{};
    dest_addr.sin_family = AF_INET;

    int inet_pton_result = inet_pton(AF_INET, ip_addr, &dest_addr.sin_addr);
    if (inet_pton_result == 0) {
        std::cerr << "Invalid IPv4 address: " << ip_addr << '\n';
        return 1;
    }
    if (inet_pton_result < 0) {
        perror("inet_pton");
        return 1;
    }

    // std::array<std::pair<std::string, int>, 4> puzzle_ports =
    assign_puzzle_to_port(dest_addr, input_ports);
}
