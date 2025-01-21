#ifndef SERVERPI_H
#define SERVERPI_H

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int port_g = 8000;

class ServerPi
{
public:
    ServerPi();

    void exchange();

private:
    tcp::acceptor acceptor;
    boost::asio::io_service io_service;
    tcp::socket socket{io_service};

    std::string     inputMSG;
    std::string     outputMSG;
};

#endif // SERVERPI_H
