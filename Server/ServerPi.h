#ifndef SERVERPI_H
#define SERVERPI_H

#include <boost/asio.hpp>

#include "../Protocols/EngineSensors.h"
#include "../Protocols/VoltageRegulators.h"

using boost::asio::ip::tcp;

namespace Msg {

enum Command{
    MOTOR_CONTROL = 0x01
};

#pragma pack(push, 1) // Отключаем выравнивание
struct MotorControlMsg {
    Command     comand;
    uint8_t     motorNum;
    uint16_t    pwm;
};
#pragma pack(pop) // Восстанавливаем предыдущее значение выравнивания
};

class ServerPi : public std::enable_shared_from_this<ServerPi>
{
public:
    ServerPi(boost::asio::io_service& io_service, unsigned short port);

    void run();
    void start_accept();
    void start_sending();
    void start_receiving();
    void process_received_data(size_t length);
    void send_message();

    void sendTestMessages();

private:
    uint8_t messageMotorSpeed[4] = {0x20, 0x28, 0x23, 0xC0};

    int port_g = 8000;
    boost::asio::io_service io_service;

    tcp::acceptor acceptor_;
    std::shared_ptr<tcp::socket> socket_;
    boost::asio::steady_timer timer_{acceptor_.get_executor()};
    std::string output_message_;
    std::array<char, 1024> buffer_;

    EngineSensors engineSensors;
    VoltageRegulators voltageRegulators;
};


#endif // SERVERPI_H
