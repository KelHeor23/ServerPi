#ifndef READER_H
#define READER_H

#include <linux/can.h>
#include <unistd.h>

namespace Can {
class Reader
{
public:
    Reader();
    ~Reader();

    void run();
private:
    int initSocket();
    void readCanMsg();

private:
    int canSocket = 0;
    struct can_frame frame;
};
}

#endif // READER_H
