#include "ServerPi.h"

#include <iostream>

ServerPi::ServerPi() : acceptor(io_service, tcp::endpoint(tcp::v4(), port_g)) {
    acceptor.accept(socket);
}

void ServerPi::exchange()
{
    // Принятие сообщения
    boost::asio::streambuf buf;
    boost::asio::read_until(socket, buf, '\n');
    std::istream input_stream(&buf);
    std::getline(input_stream, inputMSG);

    // Обработка принятого сообщения
    std::cout << "Received: " << inputMSG << std::endl;

    // Отправка сообщения
    boost::asio::write(socket, boost::asio::buffer(outputMSG + "\n"));
}
