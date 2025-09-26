#include "comservice.h"

void COMService::extract(uint32_t start, uint32_t length, uint32_t &value) const
{
    // Create a 32-bit integer from the buffer using big-endian order.
    uint32_t bits = 0;
    for (int i = 0; i < BUFLEN; ++i)
    {
        bits |= static_cast<uint32_t>(buffer[i]) << ((BUFLEN - 1 - i) * 8);
    }

    // Create a mask for the desired field.
    uint32_t mask = ((1u << length) - 1u);

    // Shift the bits right so that the bits for the field are at the LSB positions,
    // then apply the mask.
    value = (bits >> start) & mask;
}

void COMService::extract(uint32_t start, uint32_t length, int32_t &value) const
{
    uint32_t temp = 0;
    extract(start, length, temp);

    // Check the most significant bit (the sign bit) of the field.
    if ((temp >> (length - 1)) & 0x01)
    {
        // Sign extend: Fill the upper bits with ones.
        temp |= (~0u << length);
    }
    value = static_cast<int32_t>(temp);
}

uint32_t COMService::getBatteryLevel(void)
{
    uint32_t value;
    extract(signal["battery_level"].start, signal["battery_level"].length, value);
    return value;
}

int32_t COMService::getTemperature(void)
{
    int32_t value;
    extract(signal["temperature"].start, signal["temperature"].length, value);
    return value;
}

bool COMService::getLeftLight(void)
{
    uint32_t value;
    extract(signal["left_light"].start, signal["left_light"].length, value);
    return value;
}

bool COMService::getRightLight(void)
{
    uint32_t value;
    extract(signal["right_light"].start, signal["right_light"].length, value);
    return value;
}

uint32_t COMService::getSpeed(void)
{
    uint32_t value;
    extract(signal["speed"].start, signal["speed"].length, value);
    return value;
}