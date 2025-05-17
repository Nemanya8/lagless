#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <string>
#include <vector>

class UdpSender {
public:
    UdpSender(const std::string& ip, int port);
    ~UdpSender();

    bool Initialize();
    bool Send(const std::vector<uint8_t>& data);

private:
    std::string m_ip;
    int m_port;
    SOCKET m_socket;
    sockaddr_in m_addr;
    bool m_initialized;
};
