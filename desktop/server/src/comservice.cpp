#include <climits>
#include "comservice.h"

void COMService::insert(uint32_t start, uint32_t length, uint32_t value)
{
  int pos = start % CHAR_BIT;
  int index = BUFLEN - 1 - (start / CHAR_BIT);
  std::scoped_lock<std::mutex> locker{mtx};

  for (size_t i = 0; i < length; i++)
  {
    uint8_t bit = (value >> i) & 1;
    if (bit == 1)
    {
      buffer[index] |= (1 << pos);
    }
    else
    {
      buffer[index] &= ~(1 << pos);
    }
    pos++;
    if (pos == CHAR_BIT)
    {
      pos = 0;
      index--;
    }
  }
}

void COMService::insert(uint32_t start, uint32_t length, int32_t value)
{
  insert(start, length, static_cast<uint32_t>(value));
}

void COMService::setBatteryLevel(uint32_t value)
{
  if (value >= static_cast<uint32_t>(signal["battery_level"].min) && value <= static_cast<uint32_t>(signal["battery_level"].max))
  {
    insert(signal["battery_level"].start, signal["battery_level"].length, value);
  }
}

void COMService::setTemperature(int32_t value)
{
  if (value >= static_cast<int32_t>(signal["temperature"].min) && value <= static_cast<int32_t>(signal["temperature"].max))
  {
    insert(signal["temperature"].start, signal["temperature"].length, value);
  }
}

void COMService::setLeftLight(bool value)
{
  if (value == 1 || value == 0)
  {
    insert(signal["left_light"].start, signal["left_light"].length, value);
  }
}

void COMService::setRightLight(bool value)
{
  if (value == 1 || value == 0)
  {
    insert(signal["right_light"].start, signal["right_light"].length, value);
  }
}

void COMService::setSpeed(uint32_t value)
{
  if (value >= static_cast<uint32_t>(signal["speed"].min) && value <= static_cast<uint32_t>(signal["speed"].max))
  {
    insert(signal["speed"].start, signal["speed"].length, value);
  }
}
