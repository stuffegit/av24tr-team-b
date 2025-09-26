#include "tcpservice.h"
#include "comservice.h"

#include <QDebug>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>

void TCPService::run(void)
{
    sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(sockaddr_in));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(TCIP::PORT);
    servaddr.sin_addr.s_addr = inet_addr(TCIP::IP);
    while (!end)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

        if (sockfd > -1)
        {
            if (0 == connect(sockfd, (sockaddr *)&servaddr, sizeof(servaddr)))
            {
                uint8_t temp[sizeof(buffer)]{0};

                while (!end)
                {
                    status = true;

                    if (sizeof(temp) != read(sockfd, temp, sizeof(temp)))
                    {
                        status = false;
                        break;
                    }
                    else
                    {
                        std::scoped_lock<std::mutex> locker(mtx);
                        std::memcpy(buffer, temp, sizeof(buffer));
                    }
                }
            }
            close(sockfd);
        }
    }
}