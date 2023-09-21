
// cross platform
#ifdef _WIN32
#include "win_socket.h"
#endif
#ifdef _WIN64
#include "win_socket.h"
#endif
#ifdef __linux__
#include "linux_socket.h"
#endif

#include <iostream>
#include <string>

int main(int argc, char** argv)
{
    // validate argv
    if (argc != 2) {
        std::cout << "Usage: client <ip>";
        return 1;
    }

    Socket sock;
    sock.create();
    std::string ip = argv[1];
    sock.connect( ip, 15000 );

    std::cout << "Type a message to send: ";
    std::string s;
    getline(std::cin, s, '\n' );
    sock.send( s );
    sock.recv(s);
    std::cout << s;

    sock.close();

    return 0;
}