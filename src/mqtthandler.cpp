/**
 * @file mqtthandler.cpp
 * @brief Implementation of MQTT handler class
 * @author Peter Urgoš (xurgos00)
 * @date 9.5.2021
 */

#include "mqtthandler.h"
#include <string>
#include "mainwindow.h"

// Part of the code in this file was inspired by the official Paho library example:
// https://github.com/eclipse/paho.mqtt.cpp/blob/master/src/samples/async_subscribe.cpp

const int	N_RETRY_ATTEMPTS = 5;

/////////////////////////////////////////////////////////////////////////////

/**
 * @brief Callback for when reconnect is needed
 */
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


/**
 * @brief Callback for when there is a failure
 * @param tok is a token
 */
void callback::on_failure(const mqtt::token& tok)
{
    if (++nretry_ > N_RETRY_ATTEMPTS)
        exit(1);
    reconnect();
}


/**
 * @brief Callback for when there is success
 * @param tok is a token
 */
void callback::on_success(const mqtt::token& tok) {}


/**
 * @brief Callback for when the client has connected
 * @param cause of connection
 */
void callback::connected(const std::string& cause)
{    
    client.subscribe("$SYS/#", 0);
    client.subscribe("#", 0);
}


/**
 * @brief Callback for when the client connection is lost
 * @param cause of connection loss
 */
void callback::connection_lost(const std::string& cause)
{
    nretry_ = 0;
    reconnect();
}


/**
 * @brief Callback for when a new message arrives.
 * @param msg is the received message
 */
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


/**
 * @brief Callback for when delivery got completed
 * @param token is a token
 */
void callback::delivery_complete(mqtt::delivery_token_ptr token) {}


/**
 * @brief Callback is a class for async handling of Paho MQTT client
 * @param cli is client instance
 * @param connOpts are client connection options
 * @param mainWindow is a pointer to MainWindow object on which a callback function is called when new message is received
 */
callback::callback(mqtt::async_client& cli, mqtt::connect_options& connOpts, MainWindow *mainWindow)
            : nretry_(0), client(cli), connectOptions(connOpts), mainWindow(mainWindow) {}

/////////////////////////////////////////////////////////////////////////////


/**
 * @brief Handler for MQTT interaction
 * @param address of the MQTT broker
 * @param port of the MQTT broker
 * @param clientId for the MQTT client
 * @param mainWindow is pointer to Main Window in which a callback function is called when message is received
 */
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


/**
 * @brief Publish message to a topic
 * @param topic to publish to
 * @param message to publish
 */
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

