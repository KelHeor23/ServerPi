#include "ServerPi.h"

#include <iostream>


ServerPi::ServerPi(boost::asio::io_service &io_service, unsigned short port)
    : acceptor_(io_service, tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), port)), socket_(io_service)
{
    start_accept();
}

void ServerPi::run()
{
    io_service.run(); // Запуск io_service для обработки асинхронных операций
}

void ServerPi::exchange()
{
    /*auto buf = std::make_shared<boost::asio::streambuf>();
    auto self(shared_from_this()); // Удерживаем объект в памяти

    boost::asio::async_read_until(socket, *buf, '\n',
                                  [this, self, buf](const boost::system::error_code& ec, std::size_t) {
                                      if (!ec) {
                                          std::istream input_stream(buf.get());
                                          std::getline(input_stream, inputMSG);

                                          // Обработка принятого сообщения
                                          std::cout << "Received: " << inputMSG << std::endl;

                                          // Отправка сообщения
                                          boost::asio::async_write(socket,
                                                                   boost::asio::buffer(outputMSG + "\n"),
                                                                   [this, self](const boost::system::error_code& ec, std::size_t) {
                                                                       if (ec) {
                                                                           std::cerr << "Write error: " << ec.message() << std::endl;
                                                                       }
                                                                   });
                                      } else {
                                          std::cerr << "Read error: " << ec.message() << std::endl;
                                      }
                                  });*/
}

void ServerPi::start_accept()
{
    acceptor_.async_accept(socket_,
                           [this](const boost::system::error_code& error) {
                               if (!error) {
                                   std::cout << "Client connected!" << std::endl;
                                   start_sending(); // Начинаем отправку сообщений
                               } else {
                                   std::cerr << "Accept error: " << error.message() << std::endl;
                               }
                               start_accept(); // Ожидать следующее соединение
                           });
}

void ServerPi::start_sending()
{
    auto self(shared_from_this());
    output_message_ = "Hello from server\n"; // Сообщение для отправки

    send_message();

    // Запускаем таймер для отправки сообщений каждые 0.5 секунды
    timer_.expires_after(std::chrono::milliseconds(500));
    timer_.async_wait([this, self](const boost::system::error_code& ec) {
        if (!ec) {
            send_message();
            start_sending(); // Продолжаем отправку сообщений
        }
    });
}

void ServerPi::send_message()
{
    auto self(shared_from_this());
    boost::asio::async_write(socket_,
                             boost::asio::buffer(output_message_),
                             [this, self](const boost::system::error_code& ec, std::size_t /*length*/) {
                                 if (ec) {
                                     std::cerr << "Send error: " << ec.message() << std::endl;
                                     socket_.close(); // Закрываем сокет при ошибке
                                     return;
                                 }
                             });
}
