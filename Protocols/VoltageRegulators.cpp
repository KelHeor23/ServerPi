#include "VoltageRegulators.h"

#include "Constants.h"

VoltageRegulators::VoltageRegulators() {}

std::string VoltageRegulators::generateMsg(uint8_t canID)
{
    data.canID = Protocol_numbers::VOLTAGE_REGULATORS | (canID & 0x7);
    data.inputVoltageHP = 0xFF;     // Входное напряжение (0-4095), старшая часть, вольт
    data.inputVoltageLP = 0x00000001;     // Входное напряжение младшая часть, последние 4 бита - Входное напряжение (0-9), сотни мили-вольт
    data.electricCurrent = 100;    // Ток (0 - 255), ампер
    data.controlPWM = 101;         // Управляющий ШИМ, (0-2000), микро-секунды
    data.averageVoltageA = 1;    // Среднее напряжение на фазе A (0-255), вольты/10
    data.averageVoltageB = 1;    // Среднее напряжение на фазе B (0-255), вольты/10
    data.averageVoltageC = 1;    // Среднее напряжение на фазе C (0-255), вольты/10

    std::string str(reinterpret_cast<char *> (&data), sizeof(data));

    return str;
}
