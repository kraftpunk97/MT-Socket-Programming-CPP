/*
Socket class member function definitions
*/

#include "Socket.hpp"
#include <stdio.h>
#include <cstring>


Socket::Socket() {
    m_socket = -1;
    memset(&m_socketAddress, 0, sizeof(m_socketAddress));
}


Socket::~Socket() {
    if (isValid())
        ::close(m_socket);
}


bool Socket::create() {

    m_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Check the validity of the socket object
    if (!isValid())
        return false;

    // If we can not set the socket option for reusable local address, 
    // then return false
    int option = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1) {
        perror("setsockopt");
        return false;
    }

    return true;
}


bool Socket::bind(int port) {
    // If the socket is not valid, exit.
    if (!isValid())
        return false;
    
    // Set the address parameters
    m_socketAddress.sin_family = AF_INET;
    m_socketAddress.sin_addr.s_addr = INADDR_ANY;
    m_socketAddress.sin_port = htons(port);

    if (::bind(m_socket, (struct sockaddr*) &m_socketAddress, sizeof(m_socketAddress)) == -1) {
        perror("bind");
        return false;
    }

    return true;
}

bool Socket::listen() {
    if (!isValid()) {
        return false;
    }

    if (::listen(m_socket, MAXCONNECTIONS) == -1) {
        perror("listen");
        return false;
    }
    return true;
}


bool Socket::accept(Socket newSocket) {
    if (!isValid()) {
        return false;
    }

    socklen_t* p_addressLen = new socklen_t;
    *p_addressLen = sizeof(m_socketAddress);
    newSocket.m_socket = ::accept(m_socket, (struct sockaddr*) &m_socketAddress, p_addressLen);

    if (newSocket.m_socket < 0) {
        perror("accept");
        return false;
    }
    return true;
}


