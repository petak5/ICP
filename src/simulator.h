#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "mqtthandler.h"

class Simulator
{
public:
    Simulator(QString address, QString port, QString clientId);
    void run();
    void stop();
    bool isRunning();
private:
    MqttHandler *mqttHandler = nullptr;
    bool running = false;
    void _run();
};

#endif // SIMULATOR_H
