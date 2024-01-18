#pragma once

#if defined(_WIN64) || defined(_WIN32)
#define NOMINMAX
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h> // include before <windows.h>
#include <ws2tcpip.h> // more modern (API WARNINGS DEPRECATED)
#include <windows.h>
#include <io.h>
#endif
#ifdef __linux__
#include <sys/types.h> // include before socket.h
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <string>

namespace Settings
{
    // max connections with the server
    const int MAXCONNECTIONS = 5;
    // max data that can be received at once
    const int MAXRECV = 1024;
}

class Socket
{
public:
    Socket();
    virtual ~Socket();
    bool bind(const int port);
    bool listen() const;
    bool accept(Socket &) const;
    bool connect(const std::string host, const int port);

    // TCP
    bool create();
    bool send(const std::string) const;
    int recv(std::string &) const;

    // UDP
    bool UDP_create();
    bool UDP_send(const std::string, const std::string, const int port) const;
    int UDP_recv(std::string &) const;
    
    bool close() const;
    void cleanup() const;
    bool is_valid() const { return m_sock != -1; }
    int get_m_sock() const { return m_sock; }
    void set_m_sock(int nr) { m_sock = nr; }

private:
    // socket descriptor
    int m_sock;
    // struct sockaddr_in
    sockaddr_in m_addr;
};