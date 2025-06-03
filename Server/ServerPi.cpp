#include "ServerPi.h"

#include <iostream>

#include "../Protocols/EngineSensors.h"


ServerPi::ServerPi(boost::asio::io_service &io_service, unsigned short port)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), socket_(io_service)
{
    start_accept();
}

void ServerPi::run()
{
    io_service.run(); // Запуск io_service для обработки асинхронных операций
}

void ServerPi::start_accept()
{
    acceptor_.async_accept(socket_,
                           [this](const boost::system::error_code& error) {
                               if (!error) {
                                   std::cout << "Client connected!" << std::endl;
                                   start_sending(); // Начинаем отправку сообщений
                                   // После завершения работы с клиентом, снова начинаем ожидать подключений
                                   start_accept();
                               } else {
                                   std::cerr << "Accept error: " << error.message() << std::endl;
                                   // Если ошибка, пробуем переподключиться через некоторое время
                                   boost::asio::steady_timer timer(io_service);
                                   timer.expires_after(std::chrono::seconds(1)); // Ждем 1 секунду
                                   timer.async_wait([this](const boost::system::error_code& ec) {
                                       if (!ec) {
                                           start_accept(); // Повторяем попытку подключения
                                       }
                                   });
                               }
                           });
}

void ServerPi::start_sending()
{
    auto self(shared_from_this());
    sendTestMessages();

    //output_message_ = getSensorData();
    //send_message();

    // Запускаем таймер для отправки сообщений каждые 0.5 секунды
    timer_.expires_after(std::chrono::milliseconds(500));
    timer_.async_wait([this, self](const boost::system::error_code& ec) {
        if (!ec) {
            start_sending(); // Продолжаем отправку сообщений
        }
    });

    start_accept();
}

void ServerPi::send_message()
{
    auto self(shared_from_this());
    boost::asio::async_write(socket_,
                             boost::asio::buffer(output_message_),
                             [this, self](const boost::system::error_code& ec, std::size_t) {
                                 if (ec) {
                                     std::cerr << "Send error: " << ec.message() << std::endl;
                                     socket_.close(); // Закрываем сокет при ошибке
                                 }
                             });
}

void ServerPi::sendTestMessages()
{
    /*for (int i = 0; i < 8; i++) {
        output_message_ = engineSensors.generateMsg(i);
        std::cout << output_message_ << std::endl;
        send_message();
        usleep(1000);
    }

    for (int i = 0; i < 8; i++) {
        output_message_ = voltageRegulators.generateMsg(i);
        send_message();
        usleep(1000);
    }*/

    uint8_t message[4] = {0x20, 0x28, 0x23, 0xC0};
    Can::Reader::Instance().sendMsg(0x004E2A01, message, 4);

    output_message_ = getSensorData();
    send_message();
}
