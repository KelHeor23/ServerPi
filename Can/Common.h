#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <cstdint>
#include <bitset>

namespace Can {
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
}

#endif // COMMON_H
