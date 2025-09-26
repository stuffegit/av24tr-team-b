#include "tcpservice.h"
#include "comservice.h"
#include "setting.h"
#include <QDebug>
#include <arpa/inet.h>
#include <cstring>
#include <chrono>
#include <thread>
#include <unistd.h>

TCPService::TCPService()
{
    while (true)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sockfd != -1)
        {
            int tmp = 1;
            sockaddr_in servaddr;
            memset(&servaddr, 0, sizeof(sockaddr_in));

            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(TCIP::PORT);
            servaddr.sin_addr.s_addr = inet_addr(TCIP::IP);
            if (0 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(int)))
            {
                if (0 == bind(sockfd, (sockaddr *)&servaddr, sizeof(servaddr)))
                {
                    if (0 == listen(sockfd, 1))
                    {
                        break;
                    }
                }
            }

            close(sockfd);
        }

        qDebug() << "Wait to initialize the communication ...";

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void TCPService::run(void)
{

    sockaddr_in cli{};
    socklen_t len = sizeof(cli);
    int client_fd;

    while (!end)
    {
        client_fd = accept(sockfd, (sockaddr *)&cli, &len);
        if (client_fd != -1)
        {
            uint8_t temp[sizeof(buffer)]{0};
            while (!end)
            {
                status = true;
                {
                    std::scoped_lock<std::mutex> locker(mtx);
                    memcpy(temp, buffer, sizeof(buffer));
                }

                if (sizeof(temp) != write(client_fd, temp, sizeof(temp)))
                {
                    status = false;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(Setting::INTERVAL));
            }

            close(client_fd);
        }
    }
}
