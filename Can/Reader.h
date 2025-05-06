#ifndef READER_H
#define READER_H

#include <linux/can.h>
#include <unistd.h>

class Reader
{
public:
    Reader();

    ~Reader();

private:
    int initSocket();

private:
    int canSocket = 0;
    struct can_frame frame;
};

#endif // READER_H
