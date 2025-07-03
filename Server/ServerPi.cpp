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
            Can::Reader::Instance().messagesClean();
            // Закрываем предыдущее подключение
            if (socket_ && socket_->is_open()) {
                boost::system::error_code ignore;
                socket_->close(ignore);
                timer_.cancel();
            }
            socket_ = new_socket;
            start_sending();
            start_receiving();
        }
        start_accept(); // Принимаем следующего клиента
    });
}

void ServerPi::start_sending()
{
    if (!socket_->is_open()) return;

    auto self(shared_from_this());
    timer_.expires_after(std::chrono::milliseconds(50));
    timer_.async_wait([this, self](boost::system::error_code ec) {
        if (ec || !socket_->is_open()) return;

        sendTestMessages();
        start_sending(); // Рекурсивный вызов только после завершения
    });
}

void ServerPi::start_receiving() {
    if (!socket_ || !socket_->is_open()) return;

    auto self(shared_from_this());
    socket_->async_read_some(boost::asio::buffer(buffer_),
                             [this, self](boost::system::error_code ec, size_t length) {
                                 if (!ec) {
                                     process_received_data(length);
                                     start_receiving();  // Цикл приема сообщений
                                 } else {
                                     // Обработка ошибок подключения
                                     if (ec != boost::asio::error::operation_aborted) {
                                         std::cerr << "Receive error: " << ec.message() << std::endl;
                                         if (socket_ && socket_->is_open()) {
                                             boost::system::error_code ignore;
                                             socket_->close(ignore);
                                         }
                                         timer_.cancel();
                                     }
                                 }
                             });
}

void ServerPi::process_received_data(size_t length) {
    // Проверка минимального размера
    if (length < sizeof(Msg::MotorControlMsg)) {
        return; // Пустой вектор
    }

    size_t offset = 0;
    while (offset <= length - sizeof(Msg::MotorControlMsg)) {
        Msg::MotorControlMsg msg;
        // Копируем данные из буфера
        std::memcpy(&msg, buffer_.data() + offset, sizeof(Msg::MotorControlMsg));
        offset += sizeof(Msg::MotorControlMsg);

        // Проверка команды
        if (msg.comand != Msg::Command::MOTOR_CONTROL) {
            // Пропускаем некорректное сообщение или прерываем обработку
            continue;
        }

        Can::Reader::Instance().setMotorPwm(msg.motorNum, msg.pwm);
    }
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

    msg = getSensorData(); // локальная копия
    boost::asio::async_write(*socket_,
                             boost::asio::buffer(msg),
                             [self](auto, auto){} // продлеваем время жизни
                             );
}
