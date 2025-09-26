#ifndef SETTING_H
#define SETTING_H

#define BAUDRATE 1048576
#define BUFLEN 3 // Number of bytes required for the signals

#ifdef __cplusplus
#define SIGNALS                             \
    {                                       \
        {{8, 0, 0, 240}, "speed"},          \
        {{7, 8, -60, 60}, "temperature"},   \
        {{7, 15, 0, 100}, "battery_level"}, \
        {{1, 22, 0, 1}, "left_light"},      \
        {{1, 23, 0, 1}, "right_light"}}

#include <map>
#include <tuple>
#include <string>

namespace Setting
{
    class Signal
    {
        struct value_t
        {
            int length, start, min, max;
        };
        using key_t = std::string;

        std::map<key_t, value_t> signal;
        Signal()
        {
            const std::tuple<value_t, key_t> list[] = SIGNALS;
#undef SIGNALS
            for (const auto &elem : list)
            {
                signal.insert({std::get<key_t>(elem), std::get<value_t>(elem)});
            }
        }

    public:
        const value_t &operator[](const key_t &key)
        {
            return signal.at(key);
        }

        static Signal &handle(void)
        {
            static Signal instance;
            return instance;
        }
    };

    constexpr int INTERVAL{40};
}


namespace TCIP
{
    constexpr int PORT{12345};

    const char IP [] = { "127.0.0.1" };
}
#endif // __cplusplus
#endif // SETTING_H