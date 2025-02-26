#ifndef VOLTAGEREGULATORS_H
#define VOLTAGEREGULATORS_H

#include <cstdint>
#include <string>
class VoltageRegulators
{

#pragma pack(push, 1) // Отключаем выравнивание
    struct VoltageRegulatorsData{
        uint32_t    canID;              // 0x1FF1210-0x1FF1217 (Младший бит - номер регулятора))
        uint8_t     inputVoltageHP;     // Входное напряжение (0-4095), старшая часть, вольт
        uint8_t     inputVoltageLP;     // Входное напряжение младшая часть, последние 4 бита - Входное напряжение (0-9), сотни мили-вольт
        uint8_t     electricCurrent;    // Ток (0 - 255), ампер
        uint16_t    controlPWM;         // Управляющий ШИМ, (0-2000), микро-секунды
        uint8_t     averageVoltageA;    // Среднее напряжение на фазе A (0-255), вольты/10
        uint8_t     averageVoltageB;    // Среднее напряжение на фазе B (0-255), вольты/10
        uint8_t     averageVoltageC;    // Среднее напряжение на фазе C (0-255), вольты/10
    };
#pragma pack(pop) // Восстанавливаем предыдущее значение выравнивания

public:
    VoltageRegulators();

    std::string generateMsg(uint8_t canID);

private:
    VoltageRegulatorsData data;
};

#endif // VOLTAGEREGULATORS_H
