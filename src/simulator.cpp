#include "simulator.h"

#include <QtConcurrent/QtConcurrent>
#include <QThread>


Simulator::Simulator(QString address, QString port, QString clientId)
{
    mqttHandler = new MqttHandler(address, port, clientId, nullptr);
}


void Simulator::run()
{
    if (running)
        return;

    running = true;

    auto simulator = QtConcurrent::run( this, &Simulator::_run );
}


void Simulator::stop()
{
    running = false;
}


bool Simulator::isRunning() { return running; }


void Simulator::_run()
{
    for (int i = 0; i < 100; i++)
    {
        mqttHandler->publishMessage("Test topic", "Hello " + std::to_string(i));
        QThread::sleep(2);

        if (!running)
            return;
    }
}
