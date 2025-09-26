#ifndef TCPCOM_H
#define TCPCOM_H

#include "comservice.h"
#include <thread>
#include <sys/socket.h>
#include <unistd.h>

class TCPService : public COMService
{
private:
    int sockfd; ///< Server socket descriptor
    std::thread thrd{&TCPService::run, this};
    std::atomic<bool> end{false}; ///< Flag to stop the communication loop safely

    void run() override; ///< Main communication loop

public:
    TCPService(); ///< Constructor: sets up server and starts thread

    ~TCPService()
    {
        end = true;
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        thrd.join();
    }
};

#endif // TCPCOM_H
