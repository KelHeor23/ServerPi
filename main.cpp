#include "ServerPi.h"
#include <iostream>

int main()
{
    try {
        boost::asio::io_service io_service;
        auto server = std::make_shared<ServerPi>(io_service, 8001); // Замените на нужный вам порт
        io_service.run(); // Запуск обработки событий
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
