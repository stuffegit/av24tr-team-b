#include <QDebug>
#include "setting.h"
#include <QSerialPort>
#include "uartservice.h"
#include <mutex>

void UARTService::run()
{
    QSerialPort serial;
    uint8_t temp[sizeof(buffer)]{0};

    serial.setPortName("/dev/ttyUSB0");
    serial.setBaudRate(BAUDRATE);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    while (!end)
    {
        if (serial.open(QIODevice::WriteOnly))
        {
            (void)serial.clear();

            while (!end && serial.isWritable())
            {
                {
                    std::scoped_lock<std::mutex> locker{mtx};
                    std::memcpy(temp, buffer, sizeof(temp));
                }

                if (sizeof(temp) == serial.write(reinterpret_cast<const char *>(temp), sizeof(temp)))
                {
                    if (serial.flush())
                    {
                        status = true;
                        msleep(Setting::INTERVAL);
                    }
                    else
                    {
                        status = false;
                        break;
                    }
                }
                else
                {
                    status = false;
                    break;
                }
            }
        }
        else
        {
            status = false;
        }

        if (serial.isOpen())
        {
            serial.close();
        }
    }
}