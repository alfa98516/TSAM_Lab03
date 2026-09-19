#include <arpa/inet.h>
#include <array>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <ratio>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>

// For me, who is a macOs user 😭
#ifdef __APPLE__
#define iphdr ip

#define saddr ip_src.s_addr
#define daddr ip_dst.s_addr
#define tot_len ip_len
#define version ip_v
#define ihl ip_hl
#define tos ip_tos
#define id ip_id
#define frag_off ip_off
#define ttl ip_ttl
#define protocol ip_p
#define source uh_sport
#define dest uh_dport
#define len uh_ulen
#endif

uint32_t sigil;
uint8_t group_id;
std::vector<int> secret_ports;

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

/**
 * @brief Simple send recieve loop, sends 5 packets to an IP address and port,
 * and then returns the message recieved.
 *
 * @param ip_addr: The IP address you want to send to.
 * @param port: The port you want to send to.
 * @param msg: The message you want to send to the port.
 * @returns: A string
 */
std::string send_recv(sockaddr_in& ip_addr, int port, const char* msg,
                      size_t msg_length) {
    if (port < 0 || port > 65535) {
        std::cerr << "Port numbers range between 0 and 65535\n";
        return "ERROR";
    }
    if (msg == nullptr) {
        std::cerr << "Message is null\n";
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
                close(socket_fd);
                return "ERROR";
            }
            close(socket_fd);
            return std::string(data_buffer, nbytes_recieved);
        }
    }
    close(socket_fd);
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
void solve_puzzle_secret(sockaddr_in& ip_addr, const int port) {
    constexpr uint32_t secret_number = (1u << 31) - 1;
    // fun fact, this is a prime,
    // More formally, this is the eight Mersenne prime,
    // Where Mersenne prime is the collection of primes of the form 2^n - 1
    char payload_1[1024] = "S.E.C.R.E.T.:alfar24,gislih24,hlynurh24";
    int offset = 39;
    payload_1[offset] = (secret_number >> 24) & 0xFF;
    payload_1[offset + 1] = (secret_number >> 16) & 0xFF;
    payload_1[offset + 2] = (secret_number >> 8) & 0xFF;
    payload_1[offset + 3] = secret_number & 0xFF;

    std::string response = send_recv(ip_addr, port, payload_1, 43);
    const char* cstr = response.c_str();
    group_id = cstr[0];
    char challenge_chr[4];
    for (int i = 0; i < 4; i++) {
        challenge_chr[i] = cstr[i + 1];
    }

    uint32_t challenge_int = static_cast<uint8_t>(cstr[1]) << 24 |
                             static_cast<uint8_t>(cstr[2]) << 16 |
                             static_cast<uint8_t>(cstr[3]) << 8 |
                             static_cast<uint8_t>(cstr[4]);

    sigil = challenge_int ^ secret_number;

    char payload_2[1024];

    payload_2[0] = group_id;
    payload_2[1] = (sigil >> 24) & 0xFF;
    payload_2[2] = (sigil >> 16) & 0xFF;
    payload_2[3] = (sigil >> 8) & 0xFF;
    payload_2[4] = sigil & 0xFF;
    std::string msg = send_recv(ip_addr, port, payload_2, 5);
    const char* msgcstr = msg.c_str();

    char secret_port[5];
    for (int i = 0; i < 4; i++) {
        secret_port[i] = msgcstr[i + 69];
    }
    secret_port[4] = '\0';
    secret_ports.push_back(std::stoi(secret_port));
}

void solve_puzzle_evil(sockaddr_in& target_addr, const int port) {
    int routing_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (routing_socket < 0) {
        perror("creating dummy socket did not work\n");
        return;
    }

    struct sockaddr_in route_probe_addr;
    std::memset(&route_probe_addr, 0, sizeof(route_probe_addr));
    route_probe_addr.sin_family = AF_INET;
    route_probe_addr.sin_port =
        htons(53); // I love DNS 😀 it allways works great for me
    int inet_pton_result =
        inet_pton(AF_INET, "8.8.8.8", &route_probe_addr.sin_addr);

    if (inet_pton_result == 0) {
        std::cerr << "Invalid IPv4 address: " << "8.8.8.8"
                  << '\n'; // This ain't gonna happen
        close(routing_socket);
        return;
    }
    if (inet_pton_result < 0) {
        perror("inet_pton");
        close(routing_socket);
        return;
    }

    if (connect(routing_socket, (const struct sockaddr*)&route_probe_addr,
                sizeof(route_probe_addr)) < 0) {
        close(routing_socket);
        perror("Connecting failed\n");
    }

    struct sockaddr_in local_source_addr;
    if (socklen_t local_addr_len = sizeof(local_source_addr);
        getsockname(routing_socket, (struct sockaddr*)&local_source_addr,
                    &local_addr_len) < 0) {
        perror("Error getting socket name\n");
    }

    // Creating socket for sending/reciving
    char auth_payload[6];

    auth_payload[0] = group_id;
    auth_payload[1] = (sigil >> 24) & 0xFF;
    auth_payload[2] = (sigil >> 16) & 0xFF;
    auth_payload[3] = (sigil >> 8) & 0xFF;
    auth_payload[4] = sigil & 0xFF;
    auth_payload[5] = '\0';

    int raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_socket < 0) {
        perror("Error Creating EVIL socket!");
        close(raw_socket);
        return;
    }
    if (int include_ip_header = 1;
        setsockopt(raw_socket, IPPROTO_IP, IP_HDRINCL, &include_ip_header,
                   sizeof(include_ip_header)) < 0) {
        perror("setsockopt IP_HDRINCL");
        close(raw_socket);
        return;
    }
    timeval timeout{};
    timeout.tv_sec = 1;
    if (setsockopt(raw_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                   sizeof(timeout)) < 0) {
        close(raw_socket);
        perror("setting timeout went wrong\n");
        return;
    }

    int packet_total_length =
        sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(auth_payload);

    auto packet = new char[packet_total_length];
    memset(packet, 0, packet_total_length);

    int source_port = 5043; // Perhaps bad practice to use a literal here,
    // should really be looking for unused ports on the machine.
    char* udp_payload_ptr = packet + sizeof(iphdr) + sizeof(udphdr);
    auto ip_header = (struct iphdr*)packet;
    auto udp_header = (struct udphdr*)(packet + sizeof(struct iphdr));
    memcpy(udp_payload_ptr, auth_payload, strlen(auth_payload));

    ip_header->version = 4;
    ip_header->ihl = 5;
    ip_header->tos = 0;
    ip_header->id = htons(4564);
    ip_header->frag_off =
        htons(0x8000);    // Flipping the evil bit 😈 MUUHAHAHHAHAHAH!
    ip_header->ttl = 255; // Average TTL of a packet in a router.
    ip_header->protocol = IPPROTO_UDP;
    ip_header->tot_len = htons(packet_total_length);
    ip_header->saddr = local_source_addr.sin_addr.s_addr;

    ip_header->daddr = target_addr.sin_addr.s_addr;

    uint32_t ip_header_checksum = 0;
    int ip_header_bytes_remaining = sizeof(iphdr);

    auto ip_header_word_ptr = (uint16_t*)ip_header;

    while (ip_header_bytes_remaining > 1) {
        ip_header_checksum += ntohs(*ip_header_word_ptr++);
        ip_header_bytes_remaining -= 2;
    }
    // execute here.
    if (ip_header_bytes_remaining > 0) {
        ip_header_checksum += *(uint8_t*)ip_header_word_ptr;
    }
    while (ip_header_checksum >> 16) {
        ip_header_checksum =
            (ip_header_checksum & 0xFFFF) + (ip_header_checksum >> 16);
    }

// we LOVE abusing the pre processor.
// needed cause one of our team members uses apple
// ip header struct and udp header struct work exactly the same for apple,
// they're just called different things stupid stupid stuff
#ifdef __APPLE__
    ip_header->ip_sum = htons(~ip_header_checksum);
#else
    ip_header->check = htons(~ip_header_checksum);
#endif
#ifdef __APPLE__
    udp_header->uh_sum = 0;
#else
    udp_header->check = 0; // No checksum needed for UDP
#endif
    udp_header->source = htons(source_port);
    udp_header->dest = htons(port);
    udp_header->len = htons(sizeof(struct udphdr) + strlen(auth_payload));

    for (int i = 0; i < 5; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (sendto(raw_socket, packet, packet_total_length, 0,
                   (struct sockaddr*)&target_addr, sizeof(target_addr)) < 0) {

            std::cerr << "Evil port is NOT happy\n";
        }
    }
    target_addr.sin_port = htons(source_port);

    char evil_response[2048];

    socklen_t sender_addr_len = sizeof(target_addr);
    while (true) {
        ssize_t bytes_received =
            recvfrom(raw_socket, evil_response, sizeof(evil_response), 0,
                     (struct sockaddr*)&target_addr, &sender_addr_len);

        if (bytes_received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cout << "No response from port " << port << '\n';
                break;
            }
            continue;
        } else {
            if (bytes_received >= 2048) {
                std::cout << "TOO many bytes, evil port is so evil :(\n";
                break;
            }
            auto response_packet = std::string(evil_response, bytes_received);
            if (response_packet.find(
                    "The dark arts of network programming lead to powers "
                    "some consider to be...unnatural. I am an evil port, I "
                    "shall speak only with evil entities! "
                    "(https://en.wikipedia.org/wiki/Evil_bit)") ==
                std::string::npos) {

                int response_length = response_packet.length();

                std::string evil_secret_port = response_packet.substr(
                    response_length - 4, response_length);
                secret_ports.push_back(std::stoi(evil_secret_port));
                break;
            }
        }
    }
    close(raw_socket);
    delete[] packet;
}

uint16_t calculate_internet_checksum(const uint8_t* data, size_t length) {
    // 1. Divide the payload and headers into 16-bit words: The payload and
    //    some of the headers (including some IP headers) are all divided into
    //    16-bit words.
    //
    // 2. Sum the 16-bit words: These words are then added together. Whenever
    //    one of those additions results in a carry, the value is wrapped
    //    around and you add one to the value again.
    //
    // 3. Handle overflow: Wrapping any overflow around. This effectively takes
    //    the carry bit of the 16-bit addition and adds it to the value.
    //
    // 4. Take the one’s complement: Lastly, the one’s complement of the
    //    resultant sum is taken. A one’s complement sum is performed on all
    //    the 16-bit values then the one’s complement (i.e., invert all bits)
    //    is taken of that value to populate the checksum field (with the extra
    //    condition that a calculated checksum of zero will be changed into all
    //    one-bits).

    // The sum of all the 16-bit words, stored in a 32-bit integer to handle
    // overflow.
    uint32_t sum = 0;
    // Add pairs of bytes as 16-bit (2-byte) big-endian words.
    while (length >= 2) {
        // Combine two bytes into a 16-bit word in big-endian order.
        uint16_t word = (static_cast<uint16_t>(data[0]) << 8) |
                        (static_cast<uint16_t>(data[1]));
        sum += word; // Add the 16-bit word to the sum.
        data += 2;   // Move to the next 16-bit word.
        length -= 2; // Decrease the length by 2 bytes.
    }

    // If there's one byte left, treat it as the “high byte” of a 16-bit word
    // whose “low byte” is 0.
    if (length == 1) {
        sum += (static_cast<uint16_t>(data[0]) << 8);
    }

    // Handle overflow: If the sum exceeds 16 bits, wrap around the overflow.
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    // Take the one’s complement of the sum to get the final checksum.
    return static_cast<uint16_t>(~sum);
}

/*e�{l�;�.
�g�"�KY���.?�Wr�4eT�Anv1V9�O�l݆I am the guardian of the secret spell. The lords
of the network do not want us to use these newer scrolls (IPv6), but I found a
way.
Respond in the same manner and tell who you are! Send me a 5-byte message with
your group id and sigil you got from S.E.C.R.E.T.
Make sure to wrap the scrolls the same way I did and don't forget to address
them accordingly.*/
void solve_puzzle_guardian(sockaddr_in& ip_addr, const int port) {
    constexpr size_t ipv6_header_size = 40;
    constexpr size_t packet_payload_size = 5;
    // ------------------------------------------------------------------------
    // Get the Guardian's message.
    // ------------------------------------------------------------------------
    // The returned guardian message/data payload.
    const std::string guardian_message = send_recv(ip_addr, port, "payload", 7);

    // ------------------------------------------------------------------------
    // Extract the Guardian's IPv6 header that it gave us.
    // ------------------------------------------------------------------------
    // Verify the size.
    if (guardian_message.size() < ipv6_header_size) {
        std::cerr << "Guardian mesage size is too small for an IPv6 header";
        return;
    }

    // The IPv6 header from the Guardian's message.
    struct ip6_hdr guardian_ipv6_header{};
    //
    // Copy the IPv6 header bytes into guardian_ipv6_header.
    std::memcpy(&guardian_ipv6_header,
                guardian_message.data(), /*← Start at the char buffer.*/
                sizeof(guardian_ipv6_header));

    // ------------------------------------------------------------------------
    // Construct IPv6 header for the packet.
    // ------------------------------------------------------------------------
    constexpr size_t packet_size =
        sizeof(struct ip6_hdr) + sizeof(struct udphdr) + packet_payload_size;

    // The packet we'll send to the Guardian.
    char packet[packet_size]{};

    // The packet's IPv6 header.
    auto* packet_ipv6_header = reinterpret_cast<struct ip6_hdr*>(packet);
    // The packet's UDP header.
    auto* packet_udp_header =
        reinterpret_cast<struct udphdr*>(packet + sizeof(struct ip6_hdr));
    // The packet's payload that we'll use to authenticate ourselves with the
    // Guardian. Will contain the group ID and the sigil we got from Secret.
    char* packet_payload =
        packet + sizeof(struct ip6_hdr) + sizeof(struct udphdr);

    // An IPv6 header is always 40B (320b) in size.
    // --- Flow label ---
    // - Version(4b) = 6,
    // - Traffic Class(8b) = 0,
    // - Flow Label(20b) = 0,
    packet_ipv6_header->ip6_flow = htonl(6u << (8 + 20));

    // - Payload Length(16b) = The *payload* length, ∴ this excludes the
    //   header. Here, the payload is a UDP datagram = UDP header + UDP payload.
    packet_ipv6_header->ip6_plen =
        htons(sizeof(struct udphdr) + packet_payload_size);

    // - Next Header(8b) = “Which protocol header comes next after the IPv6
    //   header?”
    packet_ipv6_header->ip6_nxt = IPPROTO_UDP;

    // - Hop Limit(8b) = Like if TTL (Time To Live) actually had a good name
    //   that made sense. “Time To Live” has nothing to do with time.
    packet_ipv6_header->ip6_hlim = 64; // A common convention/default.

    // - Source IPv6 addrress (128b) = The destination address (us) we got from
    //   the IPv6 Guardian header.
    packet_ipv6_header->ip6_src = guardian_ipv6_header.ip6_dst;

    // - Destination IPv6 address (128b) = The source address (the server) we
    //   got from the IPv6 Guardian header.
    packet_ipv6_header->ip6_dst = guardian_ipv6_header.ip6_src;

    // ------------------------------------------------------------------------
    // Extract the Guardian's UDP header that it gave us.
    // ------------------------------------------------------------------------
    // The UDP header from the guardian's message.
    struct udphdr guardian_udp_header{};

    // Copy the UDP header bytes into guardian_udp_header.
    std::memcpy(&guardian_udp_header,
                guardian_message.data() + sizeof(struct ip6_hdr),
                /*↑ Start at the char buffer, after the IPv6 header.*/
                sizeof(struct udphdr));

    // ------------------------------------------------------------------------
    // Construct UDP header for the packet.
    // ------------------------------------------------------------------------
    // NOTE: We don't use htons() here, since these are already raw bytes↓.
    // Set the datagram's source port the the Guardian's destination port.
    packet_udp_header->source = guardian_udp_header.dest;
    // Set the datagram's destination port the the Guardian's source port
    packet_udp_header->dest = guardian_udp_header.source;
    // Set the datagram's total length (UDP datagram header + UDP datagram
    // payload).
    uint16_t datagram_length =
        htons(sizeof(struct udphdr) + packet_payload_size);
    packet_udp_header->len = datagram_length;
    // ---------- UDP checksum ----------
    // For IPv6, UDP normally requires a checksum. It's calculated over the
    // IPv6 pseudo-header + UDP header + UDP payload.
    /* Calculatd over this sequence:
    ┌─────────────────────────────────┐
    │       IPv6 pseudo-header        │
    │ IPv6 source address        16 B │
    │ IPv6 destination address   16 B │
    │ UDP length                  4 B │
    │ zero                        3 B │
    │ Next Header (= UDP)         1 B │
    ├─────────────────────────────────┤
    │            The rest             │
    │ UDP source port             2 B │
    │ UDP destination port        2 B │
    │ UDP length                  2 B │
    │ UDP checksum = 0            2 B │
    │ UDP data                    5 B │
    │ padding                     1 B │
    └─────────────────────────────────┘
    */
    packet_udp_header->check = 0; // Make sure it's 0 before we start.
    // ----- 1. Gather the data -----
    // --- IPv6 pseudo-header ---
    // Size of the checksum data buffer.
    constexpr size_t checksum_data_size =
        ipv6_header_size + sizeof(struct udphdr) + packet_payload_size;
    // The temporary checksum data buffer.
    char checksum_data[checksum_data_size]{};
    // Copy IPv6 source address.
    std::memcpy(checksum_data, &packet_ipv6_header->ip6_src, 16);
    // IPv6 destination address.
    std::memcpy(checksum_data + 16, &packet_ipv6_header->ip6_dst, 16);
    // Copy the UDP (datagram) length.
    std::memcpy(checksum_data + 32, &datagram_length, sizeof(datagram_length));
    // zero: Next three bytes are already 0.
    // Set Next Header (= UDP).
    checksum_data[39] = IPPROTO_UDP;
    // --- The rest ---
    // --- 2. Populate the payload. ---
    // Copy the group ID.
    packet_payload[0] = group_id;
    // Copy the sigil.
    std::memcpy(packet_payload + 1, &sigil, sizeof(sigil));
    // Copy the UDP header.
    std::memcpy(checksum_data + ipv6_header_size, packet_udp_header,
                sizeof(struct udphdr));
    // Copy UDP data (here we add our payload).
    std::memcpy(checksum_data + ipv6_header_size + sizeof(struct udphdr),
                packet_payload, packet_payload_size);

    // ----- 3. Calculate the checksum -----
    uint16_t udp_checksum = calculate_internet_checksum(
        reinterpret_cast<const uint8_t*>(checksum_data), sizeof(checksum_data));
    // For the rare edge case. If the UDP checksum = 0, that has a special
    // meaning, so we set it to 0xFFFF instead.
    if (udp_checksum == 0) {
        udp_checksum = 0xFFFF;
    }
    // ----- 4. Set the checksum -----
    packet_udp_header->check = htons(udp_checksum);

    // ------------------------------------------------------------------------
    // Put the IPv6 header + UDP datagram *inside* of the packet's payload.
    // ------------------------------------------------------------------------
    const std::string guardian_response =
        send_recv(ip_addr, port, packet, packet_size);
    std::cout << "\nGuardian's response:\n" << guardian_response << '\n';

    // ------------------------------------------------------------------------
    // Send the packet (well, ackthually, it's a datagram, since it's UDP ☝️🤓).
    //
    // Not funny I didn't laugh. Your joke is so bad I would have preferred the
    // joke went over my head and you gave up re-telling me the joke. To be
    // honest this is a horrid attempt at trying to get a laugh out of me. Not a
    // chuckle, not a hehe, not even a subtle burst of air out of my esophagus.
    // Science says before you laugh your brain preps your face muscles but I
    // didn't even feel the slightest twitch. 0/10 this joke is so bad I cannot
    // believe anyone legally allowed you to be creative at all. The amount of
    // brain power you must have put into that joke has the potential to power
    // every house on Earth. Get a personality and learn how to make jokes, read
    // a book. I'm not saying this to be funny I genuinely mean it on how this
    // is just bottom barrel embarrassment at comedy. You've single handedly
    // killed humor and every comedic act on the planet. I'm so disappointed
    // that society has failed as a whole in being able to teach you how to be
    // funny. Honestly if I put in all my power and time to try and make your
    // joke funny it would require Einstein himself to build a device to strap
    // me into so I can be connected to the energy of a billion stars to do it,
    // and even then all that joke would get from people is a subtle scuff.
    // You're lucky I still have the slightest of empathy for you after telling
    // that joke otherwise I would have committed every war crime in the book
    // just to prevent you from attempting any humor ever again. We should put
    // that joke in text books so future generations can be wary of becoming
    // such an absolute comedic failure. Im disappointed, hurt, and outright
    // offended that my precious time has been wasted in my brain understanding
    // that joke. In the time that took I was planning on helping kids who have
    // been orphaned, but because of that you've waisted my time explaining the
    // obscene integrity of your terrible attempt at comedy. Now those kids are
    // suffering without meals and there's nobody to blame but you. I hope
    // you're happy with what you have done and I truly hope you can move on and
    // learn from this piss poor attempt
    // ------------------------------------------------------------------------

    // ------------------------------------------------------------------------
    // Check if we got the right response message.
    // ------------------------------------------------------------------------
}

/*
Hail, traveler! I am D.R.A.G.O.N. - the Dwemer Relay Apparatus for Guarded
Online Networks. What aid do you seek?
Provide me with a list of secret ports, separated by commas, and I shall guide
you through the exact sequence of "knocks" required to open the portal.
How to wield D.R.A.G.O.N.:
Each "knock" must contain your group ID and S.E.C.R.E.T. sigil followed by the
secret phrase.
A word of counsel: To uncover the hidden ports and the phrases bound to them,
begin by completing the trials upon the ports revealed by your port-scanning
scrying.
Happy hunting, adventurer - and may your path lead to Sovngarde!
*/
void solve_puzzle_dragon() {}

/**
 * @brief Functions that assigns each port to the given problem
 * @return returns an array which is indexed with each problem,
 * example; the port with problem a is in index 0 of the array,
 * problem b is in index 1, etc.
 */
void assign_puzzle_to_port(sockaddr_in& ip_addr,
                           const std::array<int, 4>& ports) {
    bool is_secret_done = false;
    bool evil_done = false;

    std::array<std::string, 4> port_messages = {
        "Greetings, adventurer, from S.E.C.R.E.T. (Sacred Elder Cipher "
        "Relay ",
        "The dark arts of network programming lead to powers some consider "
        "to ",
        "I am the guardian of the secret spell. The lords of the network "
        "do ",
        "Hail, traveler! I am D.R.A.G.O.N. - the Dwemer Relay Apparatus "
        "for "};

    std::map<std::string, int> puzzle_ports;
    for (int i = 0; i < 4; i++) {

        std::string recieved_msg = send_recv(ip_addr, ports[i], "payload", 7);
        if (recieved_msg == "ERROR" || recieved_msg == "NO_RESPONSE") {
            continue;
        }
        // If the there isn't a match, then find function will return the
        // null position.
        if (recieved_msg.find(port_messages[0]) != std::string::npos) {
            puzzle_ports.insert({"SECRET", ports[i]});
        } else if ((recieved_msg.find(port_messages[1]) != std::string::npos)) {
            puzzle_ports.insert({"EVIL", ports[i]});
        } else if ((recieved_msg.find(port_messages[2]) != std::string::npos)) {
            puzzle_ports.insert({"GUARDIAN", ports[i]});
        } else if ((recieved_msg.find(port_messages[3]) != std::string::npos)) {
            puzzle_ports.insert({"DRAGON", ports[i]});
        }
    }
    solve_puzzle_secret(ip_addr, puzzle_ports.find("SECRET")->second);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    solve_puzzle_evil(ip_addr, puzzle_ports.find("EVIL")->second);

    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    solve_puzzle_guardian(ip_addr, puzzle_ports.find("GUARDIAN")->second);
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
