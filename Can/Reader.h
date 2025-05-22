#ifndef READER_H
#define READER_H

#include <linux/can.h>
#include <optional>
#include <string>
#include <unistd.h>
#include <queue>

namespace Can {
class Reader
{
public:
    static Reader& Instance(){
        try {
            static Reader instance;
            return instance;
        } catch (const std::string& msg) {
            throw msg;
        }
    }

    ~Reader();

    void run();
    std::queue<can_frame> getMessages() const;    
    std::optional<can_frame>  getCanFrame();

private:
    Reader();
    int initSocket();
    void runCanHandler();

    Reader(const Reader&) = delete;
    Reader& operator=(const Reader&) = delete;

private:
    int canSocket = 0;
    struct can_frame frame;
    std::queue<can_frame> messages;
};
}

#endif // READER_H
