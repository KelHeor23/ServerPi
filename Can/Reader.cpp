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
    close(canSocket);
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

    return 0;
}

void Reader::runCanHandler()
{    
    try {
        sendMsg();
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

            messages.push(frame);
        }
    }
    catch (...) {
        throw std::string{"Чтение can было прервано"};
    }
}

void Reader::sendMsg()
{
    CanRequestMsg request;
    memset(&request, 0, sizeof(request));

    request.opcode = 0x00;                   // Пример кода операции
    request.status_msg_id = htons(0x4E36);    // Пример ID сообщения
    request.upload_period_ms = htons(1000);    // Пример периода в мс

    // Создание CAN-фрейма
    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));

    // Настройка фрейма (пример 11-битного идентификатора)
    frame.can_id = 0x00E8A081;                     // CAN ID сообщения
    frame.can_dlc = 8;                        // Всегда 8 байт для CAN

    // Копирование данных во фрейм
    memcpy(frame.data, &request, sizeof(request));

    // Отправка данных
    int bytes_sent = write(canSocket, &frame, sizeof(frame));
    if (bytes_sent == -1) {
        throw std::runtime_error("Ошибка отправки CAN-сообщения");
    }
}
}
