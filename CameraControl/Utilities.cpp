
#include "pch.h" // Use "pch.h" or "stdafx.h" if your project uses precompiled headers
#include "Utilities.h"
#include <iostream>


namespace Utils {
    inline BOOL Ping(const CString& _IP)
    {
        CStringA ipA(_IP);  // convert CString to char*
        CStringA cmd;
        cmd.Format("ping -n 1 -w 700 %s >nul", ipA.GetString());
        int result = system(cmd);
        return (result == 0); // 0 = success
    }

    /* Enable ANSI escape codes on terminal */
    void EnableVirtualTerminal()
    {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE) return;

        DWORD mode = 0;
        if (!GetConsoleMode(hOut, &mode)) return;

        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, mode);
    }



    bool ConvertStringToNetIP(std::string_view ip, NetIP& out) noexcept
    {

        // IPv4 dotted-decimal max length = 15 ("255.255.255.255")
        if (ip.empty() || ip.size() > 15)
            return false;

        // Copy into fixed buffer with null terminator
        char buf[16]{};
        std::memcpy(buf, ip.data(), ip.size());
        buf[ip.size()] = '\0';

        // Convert string → binary IPv4
        in_addr addr{};
        if (inet_pton(AF_INET, buf, &addr) != 1)
            return false;

        // Store original view
        out.orig = ip;
        out.binary_ip = addr;

#if defined(_WIN32)
        // Windows byte access
        out.octet1 = addr.S_un.S_un_b.s_b1;
        out.octet2 = addr.S_un.S_un_b.s_b2;
        out.octet3 = addr.S_un.S_un_b.s_b3;
        out.octet4 = addr.S_un.S_un_b.s_b4;
#else

        // Linux/Unix byte access
        auto* p = reinterpret_cast<unsigned char*>(&addr.s_addr);
        out.octet1 = p[0];
        out.octet2 = p[1];
        out.octet3 = p[2];
        out.octet4 = p[3];
#endif

        return true;  // success

    }
}