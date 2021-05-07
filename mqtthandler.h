#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#include <qstring.h>
#include <mqtt/client.h>

class MqttHandler
{
public:
    MqttHandler();
    bool connect(QString address, QString port);
    bool connect(QString address, QString port, QString username, QString password);
    void disconnect();
    bool isConnected();
    void publishMessage(QString topic, QString message);
    void publishFile(QString topic, QString filePath);
private:
    mqtt::client *client;
};

#endif // MQTTHANDLER_H
