#ifndef SERVERPI_H
#define SERVERPI_H

#include <boost/asio.hpp>

#include "../Protocols/EngineSensors.h"
#include "../Protocols/VoltageRegulators.h"

using boost::asio::ip::tcp;

class ServerPi : public std::enable_shared_from_this<ServerPi>
{
public:
    ServerPi(boost::asio::io_service& io_service, unsigned short port);

    void run();
    void start_accept();
    void start_sending();
    void send_message();

    void sendTestMessages();

private:
    int port_g = 8000;
    boost::asio::io_service io_service;

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    boost::asio::steady_timer timer_{acceptor_.get_executor()};
    std::string output_message_;

    EngineSensors engineSensors;
    VoltageRegulators voltageRegulators;
};

#endif // SERVERPI_H
