#ifndef COMSERVICE_H
#define COMSERVICE_H

#include <mutex>
#include <atomic>
#include <cstdint>
#include "setting.h"

class COMService
{
  Setting::Signal &signal{Setting::Signal::handle()};

  // Inserts into buffer
  void insert(const uint32_t start, const uint32_t length, uint32_t value);
  void insert(const uint32_t start, const uint32_t length, int32_t value);

protected:
  std::mutex mtx;
  uint8_t buffer[BUFLEN]{};
  std::atomic<bool> status{false};

  // This will be defined in TCP/UART depending on protocol.
  virtual void run(void) = 0;

public:
  bool getStatus(void) { return status; };

  void setBatteryLevel(uint32_t value);

  void setTemperature(int32_t value);

  void setLeftLight(bool value);

  void setRightLight(bool value);

  void setSpeed(uint32_t value);

  virtual ~COMService() = default;
};

#endif