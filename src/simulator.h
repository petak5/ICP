/**
 * @file simulator.h
 * @brief Header file for simulator class
 * @author Peter Urgoš (xurgos00)
 * @date 9.5.2021
 */

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "mqtthandler.h"

class Simulator
{
public:
    /**
     * @brief Simulator class
     * @param address for the MQTT broker
     * @param port for the MQTT broker
     * @param clientId for the simulator MQTT client
     */
    Simulator(QString address, QString port, QString clientId);

    /**
     * @brief Run simulator
     */
    void run();

    /**
     * @brief Stop simulator
     */
    void stop();

    /**
     * @brief Get status of simulator
     * @return true when simulator is running, otherwise false
     */
    bool isRunning();

private:
    /**
     * @brief MQTT handler with a separate client from the main application
     */
    MqttHandler *mqttHandler = nullptr;

    /**
     * @brief Status of simulator (is running or not)
     */
    bool running = false;

    /**
     * @brief Simulator logic
     */
    void _run();
};

#endif // SIMULATOR_H
