#include "EngineSensors.h"

#include <cmath>

#include "Constants.h"

EngineSensors::EngineSensors() {}

std::string EngineSensors::generateMsg(uint32_t canID)
{
    data.canID = data.canID = Protocol_numbers::ENGINE_SENSORS | (canID & 0x7);;
    data.speed = 0;//rand() % (65536);
    data.temperature = 0;//static_cast<int8_t>(rand() % (256) - 128);
    data.runoutAngle = 0;//rand() % (360);
    data.runoutAmplitude = 0;//rand() % (65536);

    std::string str(reinterpret_cast<char *> (&data), sizeof(data));

    return str;
}
