#include "EngineSensors.h"

#include <cmath>
#include <iostream>

#include "Constants.h"
#include "../Can/Common.h"

EngineSensors::EngineSensors() {}

std::string EngineSensors::generateMsg(uint32_t canID)
{
    data.canID = data.canID = Protocol_numbers::ENGINE_SENSORS | (canID & 0x7);;
    data.speed = rand() % (65536);
    data.temperature = static_cast<int8_t>(rand() % (256) - 128);
    data.runoutAngle = rand() % (360);
    data.runoutAmplitude = rand() % (65536);

    std::string str(reinterpret_cast<char *> (&data), sizeof(data));

    return str;
}

std::string getSensorData()
{
    Can::Reader::Instance().run();

    auto frame = Can::Reader::Instance().getCanFrame();

    if (!frame.has_value())
        return "";

    uint32_t full_id = frame->can_id & CAN_EFF_MASK;
    uint16_t node_id = full_id & 0xFF;
    uint16_t frame_id = (full_id >> 8) & 0xFFFF;

    std::string msg(reinterpret_cast<char *> (&node_id), sizeof(node_id));
    msg += std::string(reinterpret_cast<char *> (&frame_id), sizeof(frame_id));

    std::cout << "node_id: " << node_id << "frame: " << frame_id;

    switch(frame_id){
    case 20022: {
        Can::EscStatusInfo1 tempStatus1 = Can::EscStatusInfo1::unpack(reinterpret_cast<const char*>(frame->data));
        msg += std::string(reinterpret_cast<char *> (&tempStatus1), sizeof(tempStatus1));
        break;
    }
    case 20023: {
        Can::EscStatusInfo2 tempStatus2 = Can::EscStatusInfo2::unpack(reinterpret_cast<const char*>(frame->data));
        msg += std::string(reinterpret_cast<char *> (&tempStatus2), sizeof(tempStatus2));
        break;
    }
    case 20024: {
        Can::EscStatusInfo3 tempStatus3 = Can::EscStatusInfo3::unpack(reinterpret_cast<const char*>(frame->data));
        msg += std::string(reinterpret_cast<char *> (&tempStatus3), sizeof(tempStatus3));
        break;
    }
    default: break;
    }

    return msg;
}
