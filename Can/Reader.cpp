#include "Reader.h"

#include <iomanip>
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <optional>
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

std::optional<can_frame> Reader::getCanFrame()
{
    if (messages.empty()) {
        return std::nullopt;
    }

    can_frame frame = messages.front();
    messages.pop();
    return frame;
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

            messages.push(frame);
        }
    }
    catch (...) {
        throw std::string{"Чтение can было прервано"};
    }
}
}
