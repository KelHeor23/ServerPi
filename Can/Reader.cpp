#include "Reader.h"

#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <optional>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can/raw.h>
#include <thread>


namespace Can {

Reader::Reader() {
    try {
        initSocket();
        motorsPwm.assign(cntMotors, 9000);
    } catch (const std::string& msg) {
        throw msg;
        return;
    }
}

std::queue<can_frame> Reader::getMessages() const
{
    return messages;
}

void Reader::messagesClean()
{
    std::queue<can_frame> empty;
    messages.swap(empty);  // Очистка очереди
}

std::optional<can_frame> Reader::getCanFrame()
{
    std::lock_guard<std::mutex> lock(mutex_);

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

    if (true) { // включить полноценное тестовое управление
        std::thread send([this](){
            sendMotorsPwm();
        });
        send.detach();
    }
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

    struct can_filter filters[24] = {
          {0x184e3620 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3720 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3820 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3621 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3721 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3821 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3622 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3722 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3822 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3623 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3723 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3823 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3624 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3724 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3824 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3625 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3725 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3825 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3626 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3726 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3826 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3627 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3727 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
        , {0x184e3827 | CAN_EFF_FLAG, CAN_EFF_MASK | CAN_EFF_FLAG}
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

            std::lock_guard<std::mutex> lock(mutex_);
            messages.push(frame);
        }
    }
    catch (...) {
        throw std::string{"Чтение can было прервано"};
    }
}


void Reader::sendMsg(uint32_t can_id, const uint8_t* msg_data, uint8_t msg_len)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (msg_len > 8) {
        throw std::invalid_argument("CAN message length cannot exceed 8 bytes");
    }

    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.can_id = can_id | CAN_EFF_FLAG;   // Принимаемый CAN ID, например 0x004E2A01
    frame.can_dlc = msg_len; // Длина данных (максимум 8)

    memcpy(frame.data, msg_data, msg_len); // Копируем переданное сообщение

    int bytes_sent = write(canSocket, &frame, sizeof(frame));
    if (bytes_sent == -1) {
        throw std::runtime_error("Ошибка отправки CAN-сообщения");
    }
}

void Reader::sendMotorsPwm()
{
    while(true) {
        for (int i = 0; i < motorsPwm.size(); i++)
        {
            uint16_t pwm = motorsPwm[i];
            messageMotorSpeed[0] = i + 32;
            messageMotorSpeed[1] = pwm & 0xFF;         // Младший байт
            messageMotorSpeed[2] = (pwm >> 8) & 0xFF;  // Старший байт

            Can::Reader::Instance().sendMsg(0x004E2A01, messageMotorSpeed, 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void Reader::setMotorPwm(uint8_t num, uint16_t pwm)
{
    motorsPwm[num] = pwm * 10;
}

void Reader::subscribeESCData(){
    uint8_t data[] = {0x02, 0x36, 0x4E, 0xE8, 0x03, 0xC0};
    sendMsg(0x00E8A081, messageMotorSpeed, 6);
}



}
