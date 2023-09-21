
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
    Socket server;
    server.create();
    server.bind(15000);
    server.listen();
    
    while (true) {
        Socket client;
        server.accept(client);
        std::string s;
        client.recv(s);
        std::cout << "Message received by client: ";
        std::cout << s << std::endl;
        s = "Server has received the data!\n";
        client.send(s);
        client.close();
    }
    server.close();
   
    return 0;
}