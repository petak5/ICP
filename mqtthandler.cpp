#include "mqtthandler.h"
#include <string>
#include "mainwindow.h"

// Part of the code in this file was inspired by the official Paho library example:
// https://github.com/eclipse/paho.mqtt.cpp/blob/master/src/samples/async_subscribe.cpp

const int	N_RETRY_ATTEMPTS = 5;

/////////////////////////////////////////////////////////////////////////////

void callback::reconnect()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    try {
        client.connect(connectOptions, nullptr, *this);
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        exit(1);
    }
}


void callback::on_failure(const mqtt::token& tok)
{
    if (++nretry_ > N_RETRY_ATTEMPTS)
        exit(1);
    reconnect();
}


void callback::on_success(const mqtt::token& tok) {}


void callback::connected(const std::string& cause)
{
    client.subscribe("#", 0);
}


void callback::connection_lost(const std::string& cause)
{
    nretry_ = 0;
    reconnect();
}


void callback::message_arrived(mqtt::const_message_ptr msg)
{
    mainWindow->newMessage(QString().fromStdString(msg->get_topic()), QString().fromStdString(msg->to_string()));
}


void callback::delivery_complete(mqtt::delivery_token_ptr token) {}


callback::callback(mqtt::async_client& cli, mqtt::connect_options& connOpts, MainWindow *mainWindow)
            : nretry_(0), client(cli), connectOptions(connOpts), mainWindow(mainWindow) {}

/////////////////////////////////////////////////////////////////////////////












MqttHandler::MqttHandler(QString address, QString port, MainWindow *mainWindow)
    : client(address.append(":").append(port).toStdString(), "ICP_project"), cb(client, connOpts, mainWindow)
{
    client.set_callback(cb);

    try {
        client.connect(connOpts, nullptr, cb);
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        return;
    }
}


void MqttHandler::publishMessage(QString topic, QString message)
{
    try {
        client.publish(topic.toStdString(), message.toStdString().c_str(), message.length(), 0, false);
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


void MqttHandler::subscribe(QString topic)
{
}

