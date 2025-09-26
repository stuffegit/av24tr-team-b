#ifndef UARTCOM_H
#define UARTCOM_H

#include <QThread>
#include "comservice.h"
#include <QSerialPort>
#include <QMutex>

class UARTService : public COMService, public QThread
{
    std::atomic<bool> end{false}; // Flag to stop the thread.

    void run() override;
    bool sendBuffer(QSerialPort &serial);

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