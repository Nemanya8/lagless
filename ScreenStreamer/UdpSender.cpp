#include "UdpSender.h"
#include <iostream>

UdpSender::UdpSender(const std::string& ip, int port)
    : m_ip(ip)
    , m_port(port)
    , m_socket(INVALID_SOCKET)
    , m_initialized(false)
{
    Initialize();
}

UdpSender::~UdpSender() {
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
    }
    WSACleanup();
}

bool UdpSender::Initialize() {
    if (m_initialized) return true;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return false;
    }

    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        return false;
    }

    // Enable broadcast
    int broadcast = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, 
                   reinterpret_cast<char*>(&broadcast), sizeof(broadcast)) == SOCKET_ERROR) {
        std::cerr << "Failed to enable broadcast" << std::endl;
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return false;
    }

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(m_port);
    inet_pton(AF_INET, m_ip.c_str(), &m_addr.sin_addr);

    m_initialized = true;
    return true;
}

bool UdpSender::Send(const std::vector<uint8_t>& data) {
    if (!m_initialized || m_socket == INVALID_SOCKET) {
        return false;
    }

    int result = sendto(m_socket, 
                       reinterpret_cast<const char*>(data.data()),
                       static_cast<int>(data.size()),
                       0,
                       reinterpret_cast<sockaddr*>(&m_addr),
                       sizeof(m_addr));

    return result != SOCKET_ERROR;
}
