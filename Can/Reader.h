#ifndef READER_H
#define READER_H

#include <cstdint>
#include <linux/can.h>
#include <mutex>
#include <optional>
#include <string>
#include <unistd.h>
#include <queue>

namespace Can {
class Reader
{
public:
    static Reader& Instance(){
        try {
            static Reader instance;
            return instance;
        } catch (const std::string& msg) {
            throw msg;
        }
    }

    ~Reader();

    void run();
    std::queue<can_frame> getMessages() const;    
    std::optional<can_frame>  getCanFrame();
    void sendMsg(uint32_t can_id, const uint8_t* msg_data, uint8_t msg_len);
    void sendMotorsPwm();
    void subscribeESCData();
    void setMotorPwm(uint8_t num, uint16_t pwm);

private:
    Reader();
    int initSocket();
    void runCanHandler();

    Reader(const Reader&) = delete;
    Reader& operator=(const Reader&) = delete;

private:
    int canSocket = 0;
    struct can_frame frame;
    std::queue<can_frame> messages;
    mutable std::mutex mutex_;

    std::vector<uint16_t> motorsPwm;

    int cntMotors = 8;
    uint8_t messageMotorSpeed[4] = {0x20, 0x28, 0x23, 0xC0};
};
}

#endif // READER_H
