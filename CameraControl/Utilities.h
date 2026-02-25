#pragma once
#include <string>
#include <string_view>
#include <cstring>      // memcpy
#include <WS2tcpip.h>   // Windows; on Linux use <arpa/inet.h>




namespace Utils {
    struct NetIP {
        std::string_view orig;   // non‑owning view (no allocation)
        char octet1{};
        char octet2{};
        char octet3{};
        char octet4{};
        in_addr binary_ip{};     // 32‑bit network‑order IPv4

    };

    inline BOOL Ping(const CString& _IP);
    void EnableVirtualTerminal();
    void Console(const std::string& _msg);
    void ConsoleFFMPEG(const std::string& _msg);
    const std::string RED = "\033[31m";
    const std::string RESET = "\033[0m";
    bool ConvertStringToNetIP(std::string_view ip, NetIP& out) noexcept;
}
