/**
 * @file mqtthandler.cpp
 * @brief Implementation of MQTT handler class
 * @author Peter Urgoš (xurgos00)
 * @author Adam Kľučiar (xkluci01)
 * @date 9.5.2021
 */

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
    // Subscribe to control topic
    client.subscribe("$SYS/#", 0);
    // And all other topic (non-control topics)
    client.subscribe("#", 0);
}


void callback::connection_lost(const std::string& cause)
{
    nretry_ = 0;
    reconnect();
}


void callback::message_arrived(mqtt::const_message_ptr msg)
{
    if (mainWindow == nullptr)
        return;

    // Widgets callback
    mainWindow->messageHandler(msg);

    if (!mainWindow->topicsFilter.isEmpty())
    {
        // Process message only if the topic is accepted
        auto expectedTopicPath = mainWindow->topicsFilter.split("/");
        auto messageTopicPath = QString::fromStdString(msg->get_topic()).split("/");

        // if the expected topic is more specific, the message is not part of the path and thus not accepted
        if (expectedTopicPath.length() > messageTopicPath.length())
            return;

        for (int i = 0; i < expectedTopicPath.length(); i++)
        {
            if (expectedTopicPath.at(i) != messageTopicPath.at(i))
                return;
        }
    }

    // Explorer's callback
    mainWindow->newMessage(QString().fromStdString(msg->get_topic()), msg->get_payload());
}


void callback::delivery_complete(mqtt::delivery_token_ptr token) {}


callback::callback(mqtt::async_client& cli, mqtt::connect_options& connOpts, MainWindow *mainWindow)
            : nretry_(0), client(cli), connectOptions(connOpts), mainWindow(mainWindow) {}

/////////////////////////////////////////////////////////////////////////////


MqttHandler::MqttHandler(QString address, QString port, QString clientId, MainWindow *mainWindow)
    : client(QString(address).append(":").append(port).toStdString(), clientId.toStdString()), cb(client, connOpts, mainWindow)
{
    this->address = address;
    this->port = port;

    client.set_callback(cb);

    connOpts.set_clean_session(true);

    try {
        client.connect(connOpts, nullptr, cb);
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        return;
    }
}


void MqttHandler::publishMessage(QString topic, std::string message)
{
    try {
        client.publish(topic.toStdString(), message.data(), message.length(), 0, false);
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
    }
}


QString MqttHandler::getAddress() { return this->address; }


QString MqttHandler::getPort() { return this->port; }

