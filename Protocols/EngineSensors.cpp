#include "EngineSensors.h"

#include <cmath>

EngineSensors::EngineSensors() {}

std::string EngineSensors::generateMsg(uint32_t canID)
{
    data.canID = canID;
    data.speed = rand() % (65536);
    data.temperature = static_cast<int8_t>(rand() % (256) - 128);
    data.runoutAngle = rand() % (360);
    data.runoutAmplitude = rand() % (65536);

    std::string str(reinterpret_cast<char *> (&data), sizeof(data));

    return str;
}
