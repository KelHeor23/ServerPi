#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <cstdint>
#include <bitset>

namespace Can {

#pragma pack(push, 1) // Выравнивание по 1 байту
struct CanRequestMsg{
    uint8_t opcode;
    uint16_t status_msg_id;
    uint16_t upload_period_ms;
    uint8_t reserved[3];
};
#pragma pack(pop)

struct CanFrameHeader
{
    uint8_t priority;   // 5 bits
    uint16_t dataID;    // 16 bits
    bool frameType;     // 1 bit
    uint8_t nodeID;     // 7 bits

    static CanFrameHeader unpack(uint32_t packed) {
        CanFrameHeader f;
        f.priority = (packed >> 24) & 0x1F;
        f.dataID = (packed >> 8) & 0xFFFF;
        f.frameType = ((packed >> 7) & 0x01) != 0;
        f.nodeID = packed & 0x7F;
        return f;
    }
};

struct EscStatusInfo1{
    int32_t  speed;      // 24-bit RPM motor speed (sign-extended to 32 bits)
    uint16_t recv_pwm;  // 0.1us units (little-endian)
    uint16_t comm_pwm;  // 0.1us units (little-endian)

    static EscStatusInfo1 unpack(const char buffer[8]) {
        EscStatusInfo1 result;

        // Парсинг 24-битного скорости (speed)
        uint32_t raw_speed = (static_cast<uint32_t>(static_cast<uint8_t>(buffer[2])) << 16) |
                             (static_cast<uint32_t>(static_cast<uint8_t>(buffer[1])) << 8) |
                             static_cast<uint8_t>(buffer[0]);
        result.speed = static_cast<int32_t>(raw_speed << 8) >> 8; // Знаковое расширение

        // Парсинг recv_pwm (uint16_t, little-endian)
        result.recv_pwm = static_cast<uint16_t>(static_cast<uint8_t>(buffer[3])) |
                          (static_cast<uint16_t>(static_cast<uint8_t>(buffer[4])) << 8);

        // Парсинг comm_pwm (uint16_t, little-endian)
        result.comm_pwm = static_cast<uint16_t>(static_cast<uint8_t>(buffer[5])) |
                          (static_cast<uint16_t>(static_cast<uint8_t>(buffer[6])) << 8);

        return result;
    }
};

struct EscStatusInfo2{
    uint16_t voltage;    // 0.1V Bus voltage (little-endian)
    int16_t bus_current; // 0.1A Bus current (little-endian)
    int16_t current;     // 0.1A Motor line current (little-endian)

    static EscStatusInfo2 unpack(const char buffer[8]) {
        EscStatusInfo2 result;

        // Парсинг voltage (uint16_t, little-endian)
        result.voltage =
            static_cast<uint16_t>(static_cast<uint8_t>(buffer[0])) |
            (static_cast<uint16_t>(static_cast<uint8_t>(buffer[1])) << 8);

        // Парсинг bus_current (int16_t, little-endian)
        result.bus_current =
            static_cast<int16_t>(
                static_cast<uint16_t>(static_cast<uint8_t>(buffer[2])) |
                (static_cast<uint16_t>(static_cast<uint8_t>(buffer[3])) << 8)
                );

        // Парсинг current (int16_t, little-endian)
        result.current =
            static_cast<int16_t>(
                static_cast<uint16_t>(static_cast<uint8_t>(buffer[4])) |
                (static_cast<uint16_t>(static_cast<uint8_t>(buffer[5])) << 8)
                );

        return result;
    }
};

struct EscStatusInfo3{
    uint16_t temp;           // Хуй знает что. В документации производителя проеб
    uint8_t cap_temp;        // Temperature in °C (беззнаковое)
    uint8_t mcu_temp;        // Temperature in °C (беззнаковое)
    uint8_t motor_temp;      // Temperature in °C (беззнаковое)
    uint16_t reserved;       // 0.1v Bus voltage (little-endian)

    static EscStatusInfo3 unpack(const char buffer[8]) {
        EscStatusInfo3 result;

        result.temp =
            static_cast<uint16_t>(static_cast<uint8_t>(buffer[0])) |
            (static_cast<uint16_t>(static_cast<uint8_t>(buffer[1])) << 8);

        // Парсинг температур (прямое присваивание)
        result.cap_temp = static_cast<uint8_t>(buffer[2]);
        result.mcu_temp = static_cast<uint8_t>(buffer[3]);
        result.motor_temp = static_cast<uint8_t>(buffer[4]);

        // Парсинг резервных байт (buffer[5] и buffer[6]) или другой running_error
        result.reserved =
            static_cast<uint16_t>(static_cast<uint8_t>(buffer[5])) |
            (static_cast<uint16_t>(static_cast<uint8_t>(buffer[6])) << 8);

        return result;
    }
};
}

#endif // COMMON_H
