#ifndef SOCKET_CODE__
#define SOCKET_CODE__

#include <iostream>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>

const int MAXHOSTNAME = 200;
const int MAXCONNECTIONS = 2;

class Socket {
private:
    int m_socket;
    sockaddr_in m_socketAddress;

public:
    Socket();
    ~Socket();

    bool create();
    bool bind(int port);
    bool listen();
    bool accept(Socket newSocket);
    bool read(char* targetBuffer, int bufferSize);
    bool send(char* message, int messageSize);
    bool isValid() const {return m_socket == -1;} ;

};

#endif