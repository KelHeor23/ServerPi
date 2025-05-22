#include "Reader.h"

#include <iomanip>
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can/raw.h>
#include <thread>

#include "Common.h"

namespace Can {

Reader::Reader() {
    try {
        initSocket();
    } catch (const std::string& msg) {
        throw msg;
        return;
    }
}

std::queue<can_frame> Reader::getMessages() const
{
    return messages;
}

Reader::~Reader(){
    if (canSocket != -1) {
        close(canSocket);
    }
}

void Reader::run()
{
    std::thread read([this](){
        runCanHandler();
    });

    read.detach();
}

int Reader::initSocket()
{
    // Создание CAN-сокета
    canSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (canSocket == -1) {
        throw std::string{"Ошибка создания сокета!"};
        return -1;
    }

    // Получение индекса интерфейса can0
    struct ifreq ifr;
    strcpy(ifr.ifr_name, "can0");
    if (ioctl(canSocket, SIOCGIFINDEX, &ifr) == -1) {
        throw std::string{"Ошибка получения индекса интерфейса!"};
        close(canSocket);
        return -1;
    }

    // Настройка адреса сокета
    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(canSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        close(canSocket);
        throw std::string{"Ошибка привязки сокета к интерфейсу!"};
    }

    struct can_filter filters[3] = {
        {0x184e3620 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG},
        {0x184e3720 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG},
        {0x184e3820 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
    };

    // Установка фильтров
    if (setsockopt(
            canSocket,
            SOL_CAN_RAW,
            CAN_RAW_FILTER,
            &filters,
            sizeof(filters)
            ) == -1) {
        perror("setsockopt");
        close(canSocket);
        return 1;
    }

    int recvbuf_size = 1024 * 1024; // 1 МБ
    setsockopt(canSocket, SOL_SOCKET, SO_RCVBUF, &recvbuf_size, sizeof(recvbuf_size));

    return 0;
}

void Reader::runCanHandler()
{
    try {
        //sendMsg();
        while (true) {
            // Чтение CAN-сообщения
            ssize_t nbytes = read(canSocket, &frame, sizeof(frame));
            if (nbytes < 0) {
                if (errno == EINTR) {
                    // Системный вызов прерван, продолжаем чтение
                    continue;
                }
                std::cerr << "Ошибка чтения из сокета!" << std::endl;
                break;
            }

            if (nbytes < sizeof(frame)) {
                std::cerr << "Неполное CAN-сообщение!" << std::endl;
                continue;
            }

            CanFrameHeader temp = CanFrameHeader::unpack(frame.can_id);

            uint8_t node_id = ((frame.can_id & CAN_EFF_MASK)) & 0xFF;
            uint16_t extracted_id = ((frame.can_id & CAN_EFF_MASK) >> 8) & 0xFFFF;

            std::cout << "node_id: " << node_id << "frame: " << extracted_id;

            switch(extracted_id){
            case 20022: {
                EscStatusInfo1 tempStatus1 = EscStatusInfo1::unpack(reinterpret_cast<const char*>(frame.data));
                std::cout << " speed: " << (int)tempStatus1.speed << " recv_pwm: " << tempStatus1.recv_pwm / 10 << " comm_pwm: " << tempStatus1.comm_pwm / 10 << std::endl;
                break;
            }
            case 20023: {
                EscStatusInfo2 tempStatus2 = EscStatusInfo2::unpack(reinterpret_cast<const char*>(frame.data));
                std::cout << " voltage: " << tempStatus2.voltage / 10 << " bus_current: " << tempStatus2.bus_current / 10 << " current: " << tempStatus2.current / 10 << std::endl;
                break;
            }
            case 20024: {
                EscStatusInfo3 tempStatus3 = EscStatusInfo3::unpack(reinterpret_cast<const char*>(frame.data));
                std::cout << " cap: " << tempStatus3.cap_temp - 50 << " mcu_temp: " << (int)tempStatus3.mcu_temp - 50 << " motor_temp: " << (int)tempStatus3.motor_temp - 50 << " reserved: " << tempStatus3.reserved << std::endl;
                break;
            }
            default: break;
            }

            messages.push(frame);
        }
    }
    catch (...) {
        throw std::string{"Чтение can было прервано"};
    }
}
}
