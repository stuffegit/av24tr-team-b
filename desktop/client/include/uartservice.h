#ifndef UARTCOM_H
#define UARTCOM_H

#include <QThread>
#include "comservice.h"

class UARTService : public COMService, public QThread
{
    std::atomic<bool> end{false}; // Flag to stop the thread.

    void run() override;

public:
    UARTService()
    {
        start();
    }
    ~UARTService()
    {
        end = true;
        wait();
    }
};
#endif