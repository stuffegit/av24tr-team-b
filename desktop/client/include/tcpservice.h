#ifndef TCPCOM_H
#define TCPCOM_H

#include "comservice.h"
#include "setting.h"
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <atomic>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <mutex>

class TCPService : public COMService
{
    int sockfd;                               // TCP socket file descriptor.
    std::atomic<bool> end{false};             // Flag to stop the thread.
    std::thread thrd{&TCPService::run, this}; // Thread for TCP communications.

    // This method is called in the dedicated thread.
    // It makes the connection and then continuously receives data.
    void run() override;

    // Called from run() to update COMService's buffer with incoming data.

public:
    TCPService() = default;
    ~TCPService()
    {
        end = true;
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        thrd.join();
    }
};

#endif // TCPCOM_H