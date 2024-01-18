#include <iostream>
#include <cstdlib>
#include <iostream>
#include <cstring>

#include "socket.hpp"

#if defined(_WIN64) || defined(_WIN32)
Socket::Socket() : m_sock(0)
{
    // init winsock dll
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(1, 1);
    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        std::cout << "Failed to init winsock" << std::endl;
        exit(1);
    }
}

Socket::~Socket()
{
    if (is_valid())
        ::closesocket(m_sock);
}

// TCP
bool Socket::create()
{
    m_sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_sock < 0)
    {
        std::cout << "Failed to create Socket" << std::endl;
        exit(1);
    }
    return true;
}

// UDP
bool Socket::UDP_create()
{
    m_sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (m_sock < 0)
    {
        std::cout << "Failed to create Socket" << std::endl;
        exit(1);
    }
}

// connects to a port
bool Socket::bind(const int port)
{
    if (!is_valid())
        return false;

    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = INADDR_ANY;
    m_addr.sin_port = htons(port);
    int bind_return = ::bind(m_sock, (struct sockaddr *)&m_addr, sizeof(m_addr));
    if (bind_return == -1)
        return false;
    return true;
}

// tell socket to listen to connection requests
bool Socket::listen() const
{
    if (!is_valid())
        return false;
    int listen_return = ::listen(m_sock, Settings::MAXCONNECTIONS);
    if (listen_return == -1)
        return false;
    return true;
}

// blocks, until client connects
bool Socket::accept(Socket &new_socket) const
{
    int addr_length = sizeof(m_addr);
    new_socket.m_sock = ::accept(m_sock, (sockaddr *)&m_addr, (int *)&addr_length);
    if (new_socket.m_sock <= 0)
        return false;
    return true;
}

// connect to server
bool Socket::connect(const std::string host, const int port)
{
    if (!is_valid())
        return false;

    struct hostent *host_info;
    unsigned long addr = 0;
    memset(&m_addr, 0, sizeof(m_addr));

    // if addr is numerical ip addr
    if (inet_pton(AF_INET, host.c_str(), &addr) != INADDR_NONE)
    {
        memcpy((char *)&m_addr.sin_addr, &addr, sizeof(addr));
    }
    else
    {
        // translate hostname to ip addr
        // THIS IS AN OUTDATED FUNCTION - REWRITE THIS FOR IPv6 (Error suppressed by defines in win_socket.h)
        host_info = gethostbyname(host.c_str());
        if (NULL == host_info)
        {
            std::cout << "Unknown server" << std::endl;
            exit(1);
        }
        memcpy((char *)&m_addr.sin_addr, host_info->h_addr,
               host_info->h_length);
    }

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    int status = ::connect(m_sock,
                           (sockaddr *)&m_addr, sizeof(m_addr));
    if (status == 0)
        return true;
    return false;
}

// TCP
bool Socket::send(const std::string s) const
{
    int status = ::send(m_sock, s.c_str(), s.size(), 0);
    if (status == -1)
        return false;
    return true;
}

// TCP
int Socket::recv(std::string &s) const
{
    char buf[Settings::MAXRECV + 1];
    s = "";
    memset(buf, 0, Settings::MAXRECV + 1);
    int status = ::recv(m_sock, buf, Settings::MAXRECV, 0);

    if (status > 0 || status != SOCKET_ERROR)
    {
        s = buf;
        return status;
    }
    else
    {
        std::cout << "Socket::recv threw an error - data couldn't be received" << std::endl;
        return -1;
    }
}

// UDP
bool Socket::UDP_send(const std::string addr, const std::string s, const int port) const
{
    struct sockaddr_in addr_sento;
    struct hostent *h;
    int rc;
    h = gethostbyname(addr.c_str());
    if (h == NULL)
    {
        std::cout << "Unkown host?" << std::endl;
        exit(1);
    }

    addr_sento.sin_family = h->h_addrtype;
    memcpy((char *)&addr_sento.sin_addr.s_addr,
           h->h_addr_list[0], h->h_length);
    addr_sento.sin_port = htons(port);

    rc = sendto(m_sock, s.c_str(), s.size(), 0, (struct sockaddr *)&addr_sento, sizeof(addr_sento));
    if (rc == SOCKET_ERROR)
    {
        std::cout << "Data couldn't be sent - sendto()" << std::endl;
        exit(1);
    }
    return true;
}

// UDP
int Socket::UDP_recv(std::string &s) const
{
    struct sockaddr_in addr_recvfrom;
    int len, n;
    char buf[Settings::MAXRECV + 1];
    s = "";
    memset(buf, 0, Settings::MAXRECV + 1);
    len = sizeof(addr_recvfrom);

    n = recvfrom(m_sock, buf, Settings::MAXRECV, 0, (struct sockaddr *)&addr_recvfrom, &len);
    if (n == SOCKET_ERROR)
    {
        std::cout << "Failed in: recvfrom()" << std::endl;
        exit(1);
        return 0;
    }
    else
    {
        s = buf;
        return n;
    }
}

void Socket::cleanup() const
{
    WSACleanup();
}

// close and free winsock32.dll
bool Socket::close() const
{
    closesocket(m_sock);
    cleanup();
    return true;
}
#endif /* windows */

#ifdef __linux__
Socket::Socket() : m_sock(0) {}

Socket::~Socket()
{
    if (is_valid())
        ::close(m_sock);
}

// TCP
bool Socket::create()
{
    m_sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_sock < 0)
    {
        std::cout << "Failed to create Socket" << std::endl;
        exit(1);
    }

    int y = 1;
    setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
    return true;
}

// UDP
bool Socket::UDP_create()
{
    m_sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (m_sock < 0)
    {
        std::cout << "Failed to create Socket" << std::endl;
        exit(1);
    }
}

bool Socket::bind(const int port)
{
    if (!is_valid())
    {
        return false;
    }
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = INADDR_ANY;
    m_addr.sin_port = htons(port);

    int bind_return = ::bind(m_sock, (struct sockaddr *)&m_addr, sizeof(m_addr));
    if (bind_return == -1)
    {
        return false;
    }
    return true;
}

// connection requests by client
bool Socket::listen() const
{
    if (!is_valid())
    {
        return false;
    }
    int listen_return = ::listen(m_sock, Settings::MAXCONNECTIONS);
    if (listen_return == -1)
    {
        return false;
    }
    return true;
}

// accept() blocks until client connects
bool Socket::accept(Socket &new_socket) const
{
    int addr_length = sizeof(m_addr);
    new_socket.m_sock = ::accept(m_sock, (sockaddr *)&m_addr, (socklen_t *)&addr_length);
    if (new_socket.m_sock <= 0)
        return false;
    else
        return true;
}

// connect to server
bool Socket::connect(const std::string host, const int port)
{
    if (!is_valid())
        return false;

    struct hostent *host_info;
    unsigned long addr;
    memset(&m_addr, 0, sizeof(m_addr));
    if ((addr = inet_addr(host.c_str())) != INADDR_NONE)
    {
        // argv[1] is numerical IP addr
        memcpy((char *)&m_addr.sin_addr,
               &addr, sizeof(addr));
    }
    else
    {
        // translate server name to IP addr
        host_info = gethostbyname(host.c_str());
        if (NULL == host_info)
        {
            std::cout << "Unknown host" << std::endl;
            exit(1);
        }
        memcpy((char *)&m_addr.sin_addr, host_info->h_addr, host_info->h_length);
    }

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    int status = ::connect(m_sock,
                           (sockaddr *)&m_addr, sizeof(m_addr));
    if (status == 0)
        return true;
    else
        return false;
}

// TCP
bool Socket::send(const std::string s) const
{
    int status = ::send(m_sock, s.c_str(), s.size(), 0);
    if (status == -1)
    {
        return false;
    }
    else
    {
        return true;
    }
}

// TCP
int Socket::recv(std::string &s) const
{
    char buf[Settings::MAXRECV + 1];
    s = "";
    memset(buf, 0, Settings::MAXRECV + 1);
    int status = ::recv(m_sock, buf, Settings::MAXRECV, 0);
    if (status > 0 || status != -1)
    {
        s = buf;
        return status;
    }
    else
    {
        std::cout << "Failed in recv()" << std::endl;
        exit(1);
        return 0;
    }
}

// UDP
bool Socket::UDP_send(const std::string addr, const std::string s, const int port) const
{
    struct sockaddr_in addr_sento;
    struct hostent *h;
    int rc;
    h = gethostbyname(addr.c_str());
    if (h == NULL)
    {
        std::cout << "Unbekannter Host?" << std::endl;
        exit(1);
    }
    addr_sento.sin_family = h->h_addrtype;
    memcpy((char *)&addr_sento.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    addr_sento.sin_port = htons(port);
    rc = sendto(m_sock, s.c_str(), s.size(), 0, (struct sockaddr *)&addr_sento, sizeof(addr_sento));
    if (rc == -1)
    {
        std::cout << "Failed in sendto()" << std::endl;
        exit(1);
    }
    return true;
}

// UDP
int Socket::UDP_recv(std::string &s) const
{
    struct sockaddr_in addr_recvfrom;
    int len, n;
    char buf[Settings::MAXRECV + 1];
    s = "";
    memset(buf, 0, Settings::MAXRECV + 1);
    len = sizeof(addr_recvfrom);
    n = recvfrom(m_sock, buf, Settings::MAXRECV, 0,
                 (struct sockaddr *)&addr_recvfrom,
                 (socklen_t *)&len);
    if (n == -1)
    {
        std::cout << "Failed in recvfrom()" << std::endl;
        exit(1);
        return 0;
    }
    else
    {
        s = buf;
        return n;
    }
}

// for cp
void Socket::cleanup() const {}

// close socket
bool Socket::close() const
{
    ::close(m_sock);
    cleanup();
    return true;
}
#endif /* linux */