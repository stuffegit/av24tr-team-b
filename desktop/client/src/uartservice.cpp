#include "setting.h"
#include "uartservice.h"
#include <QSerialPort>
#include <mutex>

void UARTService::run()
{
    QSerialPort serial;
    uint8_t temp[sizeof(buffer)]{0};

    serial.setPortName("/dev/ttyUSB1");
    serial.setBaudRate(BAUDRATE);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    while (!end)
    {
        if (serial.open(QIODevice::ReadOnly))
        {
            while (!end && serial.isReadable())
            {
                (void)serial.clear();

                if (serial.waitForReadyRead(3 * Setting::INTERVAL))
                {
                    if (sizeof(temp) == serial.read(reinterpret_cast<char *>(temp), sizeof(temp)))
                    {
                        status = true;
                        std::scoped_lock<std::mutex> locker{mtx};
                        std::memcpy(buffer, temp, sizeof(buffer));
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
