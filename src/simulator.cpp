#include "simulator.h"

#include <QtConcurrent/QtConcurrent>
#include <QThread>


/**
 * @brief Simulator::Simulator
 * @param address for the MQTT broker
 * @param port for the MQTT broker
 * @param clientId for the simulator MQTT client
 */
Simulator::Simulator(QString address, QString port, QString clientId)
{
    mqttHandler = new MqttHandler(address, port, clientId, nullptr);
}


/**
 * @brief Run simulator
 */
void Simulator::run()
{
    if (running)
        return;

    running = true;

    auto simulator = QtConcurrent::run( this, &Simulator::_run );
}


/**
 * @brief Stop simulator
 */
void Simulator::stop()
{
    running = false;
}


/**
 * @brief Get status of simulator
 * @return true when simulator is running, otherwise false
 */
bool Simulator::isRunning() { return running; }


/**
 * @brief Simulator logic
 */
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

