#include <array>
#include <iostream>
#include <string>

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
    std::array<int, 4> input_ports;

    for (int i = 0; i < 4; i++) {
        int curr_port = parse_port(argv[i], 2);
    }

    std::array<std::string, 4> port_messages = {
        "Greetings, adventurer, from S.E.C.R.E.T. (Sacred Elder Cipher Relay ",
        "The dark arts of network programming lead to powers some consider to ",
        "I am the guardian of the secret spell. The lords of the network do ",
        "Hail, traveler! I am D.R.A.G.O.N. - the Dwemer Relay Apparatus for "};

    std::array<std::string, 4> problem_ports;

    for (int i = 0; i < 4; i++) {
        // TODO: needs changing to actual recieve from function thats yet to be implemented
        std::string recieved = recieved_from(i);
        if (recieved.find(port_messages[i]) != std::string::npos) {
            problem_ports[i] = recieved;
        }
        
    }
    // for (int i = 2; i < 6; i++) {
    //  TODO: Decide which puzzle the port is for using the received message
    //  and the above constant strings, then call the appropriate puzzle
    //  function.
    //}
}

const std::array<std::string, 4> dummy_recieved = {
        "Greetings, adventurer, from S.E.C.R.E.T. (Sacred Elder Cipher Relay jafdslæfjasldkægjsa ",
        "The dark arts of network programming lead to powers some consider to dkfjasdælkgnadsælg ",
        "I am the guardian of the secret spell. The lords of the network do gahjfdagkjfaklægjdsæl",
        "Hail, traveler! I am D.R.A.G.O.N. - the Dwemer Relay Apparatus for lkdsgajgælkasjdglækasjdg"};

std::string recieved_from(int index) {

    return dummy_recieved[index];
}

void solve_puzzle_a() {}
void solve_puzzle_b() {}
void solve_puzzle_c() {}
void solve_puzzle_d() {}
