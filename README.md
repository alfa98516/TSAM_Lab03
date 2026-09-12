# TSAM_Lab03

## NOTE (just for us, delete before turning in)
Naming conventions:

| Thing               | Convention             | Naming rule                                                                | Examples                                                |
| ------------------- | ---------------------- | -------------------------------------------------------------------------- | ------------------------------------------------------- |
| Classes / structs   | `PascalCase`           | Noun or noun phrase                                                        | `UdpScanner`, `PacketHeader`, `TcpConnection`           |
| Enums               | `PascalCase`           | Noun describing the category/state                                         | `ScanResult`, `ConnectionState`                         |
| Enum values         | `PascalCase`           | Short value/state name                                                     | `ScanResult::Open`, `ConnectionState::Closed`           |
| Functions / methods | `snake_case`           | Verb or verb phrase describing the action                                  | `scan_port()`, `send_packet()`, `calculate_checksum()`  |
| Local variables     | `snake_case`           | Descriptive noun or noun phrase                                            | `port_number`, `socket_fd`, `packet_size`               |
| Parameters          | `snake_case`           | Descriptive noun or noun phrase                                            | `ip_address`, `low_port`, `timeout_ms`                  |
| Booleans            | `snake_case`           | Yes/no statement, often beginning with `is_`, `has_`, `can_`, or `should_` | `is_connected`, `has_data`, `can_retry`, `should_close` |
| Collections         | `snake_case`           | Plural noun describing the elements                                        | `open_ports`, `packets`, `connections`                  |
| Class members       | `snake_case_`          | Descriptive noun; trailing `_` marks member state                          | `socket_fd_`, `timeout_ms_`, `remote_address_`          |
| Constants           | `kPascalCase`          | Descriptive noun; include units where useful                               | `kDefaultTimeoutMs`, `kMaxPacketSize`                   |
| Namespaces          | `snake_case`           | Short noun describing the logical module/domain                            | `network_utils`, `packet_parser`                        |
| Source/header files | `snake_case`           | Name after the main type or responsibility                                 | `udp_scanner.cpp`, `packet_parser.hpp`                  |
| Macros              | `SCREAMING_SNAKE_CASE` | Descriptive name; reserve this style for preprocessor macros               | `DEBUG_LOG`, `ENABLE_TRACING`                           |
