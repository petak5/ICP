#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#include <qstring.h>
#include <mqtt/async_client.h>

class MainWindow;


// Part of the code in this file was inspired by the official Paho library example:
// https://github.com/eclipse/paho.mqtt.cpp/blob/master/src/samples/async_subscribe.cpp

class callback : public virtual mqtt::callback, public virtual mqtt::iaction_listener
{
    int nretry_;
    mqtt::async_client& client;
    mqtt::connect_options& connectOptions;
    MainWindow *mainWindow;

    void reconnect();

    void on_failure(const mqtt::token& tok) override;

    void on_success(const mqtt::token& tok) override;

    void connected(const std::string& cause) override;

    void connection_lost(const std::string& cause) override;

    void message_arrived(mqtt::const_message_ptr msg) override;

    void delivery_complete(mqtt::delivery_token_ptr token) override;

public:
    callback(mqtt::async_client& cli, mqtt::connect_options& connOpts, MainWindow *mainWindow);
};

class MqttHandler
{
public:
    MqttHandler(QString address, QString port, MainWindow *mainWindow);
    void publishMessage(QString topic, QString message);
    void publishFile(QString topic, QString filePath);
    void subscribe(QString topic);
private:
    mqtt::async_client client;
    bool isSubscribed = false;
    mqtt::connect_options connOpts;
    callback cb;
};

#endif // MQTTHANDLER_H
