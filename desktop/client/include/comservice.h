#ifndef COMSERVICE_H
#define COMSERVICE_H

#include <mutex>
#include <atomic>
#include <cstdint>
#include "setting.h"

class COMService
{
    void extract(uint32_t start, uint32_t length, int32_t &value) const;
    void extract(uint32_t start, uint32_t length, uint32_t &value) const;

    Setting::Signal &signal{Setting::Signal::handle()};

protected:
    std::mutex mtx;
    uint8_t buffer[BUFLEN]{};
    std::atomic<bool> status{false};

    virtual void run(void) = 0;

public:
    COMService() = default;

    bool getStatus() const { return status; }

    uint32_t getBatteryLevel(void);

    int32_t getTemperature(void);

    bool getLeftLight(void);

    bool getRightLight(void);

    uint32_t getSpeed(void);

    virtual ~COMService() = default;
};

#endif