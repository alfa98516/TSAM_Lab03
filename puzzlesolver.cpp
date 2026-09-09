#include <iostream>
#include <string>

namespace
{
} // namespace

/**
 * @brief Main function that scans a given IPv4 address for open UDP ports in
 * the specified range.
 * @param argc Number of args, must be 4.
 * @param argv Argument vector, where: argv[1] is the IPv4 address, argv[2] is
 * the low port, and argv[3] is the high port.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, const char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage:" << argv[0]
                  << " <IPv4 address> <low port> <high port>\n";
        return 1;
    }

    const std::string msg_secret_port =
        "Greetings, adventurer, from S.E.C.R.E.T. (Sacred Elder Cipher Relay ";
    const std::string msg_evil_port =
        "The dark arts of network programming lead to powers some consider to ";
    const std::string msg_ipv6_secret =
        "I am the guardian of the secret spell. The lords of the network do ";
    const std::string msg_dragon_port_knocking =
        "Hail, traveler! I am D.R.A.G.O.N. - the Dwemer Relay Apparatus for ";
}
