#include "Reader.h"

#include <iomanip>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can/raw.h>

Reader::Reader() {
    try {
        initSocket();
    } catch (const std::string& msg) {
        throw msg;
        return;
    }
}

Reader::~Reader(){
    close(canSocket);
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

    return 0;
}

void Reader::readCanMsg()
{
    try {
        while (true) {
            // Чтение CAN-сообщения
            ssize_t nbytes = read(canSocket, &frame, sizeof(frame));
            if (nbytes < 0) {
                std::cerr << "Ошибка чтения из сокета!" << std::endl;
                break;
            }

            if (nbytes < sizeof(frame)) {
                std::cerr << "Неполное CAN-сообщение!" << std::endl;
                continue;
            }

            // Вывод информации о сообщении
            std::cout << "ID: " << std::hex << std::setw(3) << std::setfill('0')
                      << (frame.can_id & 0x1FFFFFFF) << "  Длина: " << std::dec
                      << static_cast<int>(frame.can_dlc) << "  Данные: ";

            // Вывод данных сообщения
            for (int i = 0; i < frame.can_dlc; i++) {
                std::cout << std::hex << std::setw(2) << std::setfill('0')
                          << static_cast<int>(frame.data[i]) << " ";
            }
            std::cout << std::endl;
        }
    }
    catch (...) {
        throw std::string{"Чтение can было прервано"};
    }
}
