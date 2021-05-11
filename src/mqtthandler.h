/**
 * @file mqtthandler.h
 * @brief Header file for MQTT handler class
 * @author Peter Urgoš (xurgos00)
 * @date 9.5.2021
 */

#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#include <qstring.h>
#include <mqtt/async_client.h>

class MainWindow;


// Part of the code in this file was inspired by the official Paho library example:
// https://github.com/eclipse/paho.mqtt.cpp/blob/master/src/samples/async_subscribe.cpp

class callback : public virtual mqtt::callback, public virtual mqtt::iaction_listener
{
    /**
     * @brief Number of reconnect retries
     */
    int nretry_;

    /**
     * @brief Paho MQTT async client
     */
    mqtt::async_client& client;

    /**
     * @brief Paho MQTT client connection options
     */
    mqtt::connect_options& connectOptions;

    /**
     * @brief Reference to MainWindow instance on which callback functions are called
     */
    MainWindow *mainWindow;

    /**
     * @brief Callback for when reconnect is needed
     */
    void reconnect();

    /**
     * @brief Callback for when there is a failure
     * @param tok is a token
     */
    void on_failure(const mqtt::token& tok) override;

    /**
     * @brief Callback for when there is success
     * @param tok is a token
     */
    void on_success(const mqtt::token& tok) override;

    /**
     * @brief Callback for when the client has connected
     * @param cause of connection
     */
    void connected(const std::string& cause) override;

    /**
     * @brief Callback for when the client connection is lost
     * @param cause of connection loss
     */
    void connection_lost(const std::string& cause) override;

    /**
     * @brief Callback for when a new message arrives.
     * @param msg is the received message
     */
    void message_arrived(mqtt::const_message_ptr msg) override;

    /**
     * @brief Callback for when delivery got completed
     * @param token is a token
     */
    void delivery_complete(mqtt::delivery_token_ptr token) override;

public:
    /**
     * @brief Callback is a class for async handling of Paho MQTT client
     * @param cli is client instance
     * @param connOpts are client connection options
     * @param mainWindow is a pointer to MainWindow object on which a callback function is called when new message is received
     */
    callback(mqtt::async_client& cli, mqtt::connect_options& connOpts, MainWindow *mainWindow);
};

class MqttHandler
{
public:
    /**
     * @brief Handler for MQTT interaction
     * @param address of the MQTT broker
     * @param port of the MQTT broker
     * @param clientId for the MQTT client
     * @param mainWindow is pointer to Main Window in which a callback function is called when message is received
     */
    MqttHandler(QString address, QString port, QString clientId, MainWindow *mainWindow);

    /**
     * @brief Publish message to a topic
     * @param topic to publish to
     * @param message to publish
     */
    void publishMessage(QString topic, std::string message);

    /**
     * @brief Get address of server that the client is connected to
     * @return address (without port)
     */
    QString getAddress();

    /**
     * @brief Get port of server that the client is connected to
     * @return port
     */
    QString getPort();

private:
    /**
     * @brief Paho MQTT async client
     */
    mqtt::async_client client;

    /**
     * @brief Paho MQTT client connection options
     */
    mqtt::connect_options connOpts;

    /**
     * @brief Client's callback object
     */
    callback cb;

    /**
     * @brief Server address
     */
    QString address;

    /**
     * @brief Server port
     */
    QString port;
};

#endif // MQTTHANDLER_H
