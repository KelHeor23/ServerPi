#include <iostream>

#include "Server/ServerPi.h"
#include "Can/Reader.h"

int main()
{
    try {
        Can::Reader::Instance().run();

        boost::asio::io_service io_service;
        auto server = std::make_shared<ServerPi>(io_service, 8002);
        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
