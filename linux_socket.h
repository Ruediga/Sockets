#pragma once

#include <sys/types.h> // include before socket.h
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>

namespace Settings
{
    // max connections with the server
    const int MAXCONNECTIONS = 5;
    // max data that can be received at once in bytes (?)
    const int MAXRECV = 1024;
}

class Socket
{
public:
    Socket();
    virtual ~Socket();
    // TCP
    bool create();
    // UDP
    bool UDP_create();
    bool bind( const int port );
    bool listen() const;
    bool accept( Socket& ) const;
    bool connect ( const std::string host, const int port );
    // TCP
    bool send ( const std::string ) const;
    int recv ( std::string& ) const;
    // UDP
    bool UDP_send( const std::string, const std::string,
    const int port ) const;
    int UDP_recv( std::string& ) const;
    // close socket
    bool close() const;
    // WSAcleanup()
    void cleanup() const;
    bool is_valid() const { return m_sock != -1; }

private:
    // socket descriptor
    int m_sock;
    // struct sockaddr_in
    sockaddr_in m_addr;
};