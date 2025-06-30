#include "ServerPi.h"

#include <iostream>

#include "../Protocols/EngineSensors.h"


ServerPi::ServerPi(boost::asio::io_service& io_service, unsigned short port)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    , timer_(io_service) // сокет теперь инициализируется при принятии подключения
{
    start_accept();
}

void ServerPi::run()
{
    io_service.run(); // Запуск io_service для обработки асинхронных операций
}

void ServerPi::start_accept()
{
    auto new_socket = std::make_shared<tcp::socket>(acceptor_.get_executor());

    acceptor_.async_accept(*new_socket, [this, new_socket](auto ec) {
        if (!ec) {
            // Закрываем предыдущее подключение
            if (socket_ && socket_->is_open()) {
                boost::system::error_code ignore;
                socket_->close(ignore);
                timer_.cancel();
            }
            socket_ = new_socket;
            start_sending();
        }
        start_accept(); // Принимаем следующего клиента
    });
}

void ServerPi::start_sending()
{
    if (!socket_->is_open()) return;

    auto self(shared_from_this());
    timer_.expires_after(std::chrono::milliseconds(500));
    timer_.async_wait([this, self](boost::system::error_code ec) {
        if (ec || !socket_->is_open()) return;

        sendTestMessages();
        start_sending(); // Рекурсивный вызов только после завершения
    });
}

void ServerPi::send_message()
{
    if (!socket_->is_open()) return;

    auto self(shared_from_this());
    boost::asio::async_write(*socket_,
                             boost::asio::buffer(output_message_),
                             [this, self](boost::system::error_code ec, size_t) {
                                 if (ec) {
                                     socket_->close();
                                     timer_.cancel();
                                 }
                             });
}

void ServerPi::sendTestMessages()
{
    std::string msg = voltageRegulators.generateMsg(0);


    auto self(shared_from_this());
    boost::asio::async_write(*socket_,
                             boost::asio::buffer(msg),
                             [self](auto, auto){} // продлеваем время жизни
                             );

    setMotorSpeed();
    msg = getSensorData(); // локальная копия

    boost::asio::async_write(*socket_,
                             boost::asio::buffer(msg),
                             [self](auto, auto){} // продлеваем время жизни
                             );
}

void ServerPi::setMotorSpeed()
{
    static int i = 0;

    static uint16_t pwm = 9000;

    messageMotorSpeed[1] = pwm & 0xFF;         // Младший байт
    messageMotorSpeed[2] = (pwm >> 8) & 0xFF;  // Старший байт

    Can::Reader::Instance().sendMsg(0x004E2A01, messageMotorSpeed, 4);

    if (pwm < 19500 && i == 10) {
        i = 0;
        pwm += 500;
    }
    i++;
}
