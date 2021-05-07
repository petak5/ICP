#include "mqtthandler.h"
#include <string>

MqttHandler::MqttHandler()
{
    client = new mqtt::client("", "");
}


bool MqttHandler::connect(QString address, QString port)
{
    client = new mqtt::client(QString(address).append(":").append(port).toStdString(), "ICP_project");
    try {
        client->connect();
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        return false;
    }

    return true;
}


bool MqttHandler::connect(QString address, QString port, QString username, QString password)
{
    // TODO
    return false;
}


void MqttHandler::disconnect()
{
    try {
        if (client == nullptr) return;

        client->disconnect();
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
    }
}


bool MqttHandler::isConnected()
{
    return client->is_connected();
}


void MqttHandler::publishMessage(QString topic, QString message)
{
    try {
        client->publish(topic.toStdString(), message.toStdString().c_str(), message.length(), 0, false);
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
    }
}


void MqttHandler::publishFile(QString topic, QString filePath)
{
    // TODO
    return;
}
