/**
 * @file mainwindow.h
 * @brief Header file for main window class
 * @author Peter Urgoš (xurgos00)
 * @author Adam Kľučiar (xkluci01)
 * @date 9.5.2021
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include "valueinspectdialog.h"
#include "mqtthandler.h"
#include "simulator.h"
#include <QDir>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Topic
{
public:
    /**
     * @brief Topic class represents a topic and stores messages posted to it
     * @param topic is the topic's topic
     */
    Topic(QString topic);

    /**
     * @brief Get topic name
     * @return topic name
     */
    QString getTopic();

    /**
     * @brief Add message to topic
     * @param message to add
     * @param maxCount of messages stored
     */
    void addMessage(std::string message, int maxCount);

    /**
     * @brief Get all messages
     * @param maxCount of mesages stored
     * @return list of messages
     */
    QList<std::string *> &getMessages(int maxCount);

    /**
     * @brief Find topic in the topics tree at the specified path
     * @param path is path to the topic in the tree
     * @return topic at specified path or nullptr if not found
     */
    Topic * findTopic(QStringList path);

    /**
     * @brief Add topic to tree, the location is determined by the topic attribute which is used as path in the tree
     * @param topic to add to topic's tree structure
     * @return the topic that was added to the tree
     */
    Topic * addTopic(Topic *topic);

    /**
     * @brief Export (save) captured data to a directory on disk
     * @param directory where to export
     */
    void exportToDisk(QDir directory);

private:
    /**
     * @brief topic of the topic
     */
    QString topic;

    /**
     * @brief List of messages
     */
    QList<std::string *> messages;

    /**
     * @brief List of children (subtopics)
     */
    QList<Topic *> children;

    /**
     * @brief Add topic to children
     * @param topic to add
     */
    void addChild(Topic *topic);

    /**
     * @brief Get all children
     * @return list of children
     */
    QList<Topic *> &getChildren();
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /**
     * @brief Function that is called when new message is received in the Paho client callback, updates UI with the new message
     * @param topic is the topic of the message
     * @param message is the message's conetnt (payload)
     */
    void newMessage(QString topic, std::string payload);

    /**
     * @brief Topics filter
     */
    QString topicsFilter = "";

    /**
     * @brief Forwards msg to all dashboard widgets with the same topic
     * @param msg message received from MQTT broker
     */
    void messageHandler(mqtt::const_message_ptr msg);

public slots:
    /**
     * @brief sends message to MQTT broker containing the opposite state of switch widget
     */
    void on_widgetSwitchButton_clicked();

    /**
     * @brief Sends message to MQTT broker contained in text field after signal from button
     */
    void on_widgetTextButton_clicked();

private slots:
    /**
     * @brief Subscibe to a topic
     */
    void on_subscribeButton_clicked();

    /**
     * @brief Reset topic subscibtion to no topic (receive all topics)
     */
    void on_subscribeResetButton_clicked();

    /**
     * @brief Inspect value in a modal window
     */
    void on_valueInspectButton_clicked();

    /**
     * @brief Open value inspect dialog when double clicked on an item
     * @param item that was double clicked
     */
    void on_valueHistoryList_itemDoubleClicked(QListWidgetItem *item);

    /**
     * @brief Update displayed value when item from history is selected
     */
    void on_valueHistoryList_itemSelectionChanged();

    /**
     * @brief Publish text message to a topic
     */
    void on_publishTextButton_clicked();

    /**
     * @brief Publish file to a topic
     */
    void on_publishFileButton_clicked();

    /**
     * @brief Connect to or disconnect from server (MQTT broker)
     */
    void on_connectToServerButton_clicked();

    /**
     * @brief Update list when tree view selection changed
     */
    void on_treeWidget_itemSelectionChanged();

    /**
     * @brief MainWindow::on_numberOfMessagesSetButton_clicked
     */
    void on_numberOfMessagesSetButton_clicked();

    /**
     * @brief Export captured data to disk
     */
    void on_exportButton_clicked();

    /**
     * @brief Run or stop simulator
     */
    void on_simulatorButton_clicked();

    /**
    * @brief Adds dashboard widget with user defined name topic and type
    */
    void on_widgetAddButton_clicked();

    /**
     * @brief Removes dashboard widget defined in comboBox
     */
    void on_widgetRemoveButton_clicked();

private:
    Ui::MainWindow *ui;

    /**
     * @brief MQTT client handler
     */
    MqttHandler *mqttHandler = nullptr;

    /**
     * @brief Tree of topics (backend model)
     */
    QList<Topic *> topicsTree;

    /**
     * @brief Number of messages stored in hisotry for each topic
     */
    int numberOfMessagesInHistory = 1;

    /**
     * @brief MQTT network simulator
     */
    Simulator *simulator = nullptr;

    /**
     * @brief Add root item to tree
     * @param title of the item
     * @return Root item that was created and added to the QTreeWidget
     */
    QTreeWidgetItem * treeViewAddRootItem(QString name);

    /**
     * @brief Add item to the parent
     * @param parent to add item to
     * @param title of the item
     * @return The item that was created and added to the parent
     */
    QTreeWidgetItem * treeViewAddItem(QTreeWidgetItem *parent, QString name);

    /**
     * @brief Get path to currently selected item in tree view
     * @return path to current item, empty path if not found
     */
    QStringList treeViewGetPathToCurrentItem();

    /**
     * @brief Find topic at the specified path in the topics tree structure
     * @param path is the path to the requested topic
     * @return topic at the path or nullptr if not found
     */
    Topic *treeViewFindTopic(QStringList path);

    /**
     * @brief Fills value history list with values related to the currently selected item in tree widget
     */
    void refreshValuesList();

    /**
     * @brief Creates dashboard widget for displaying switch state in interface
     * @param interface pointer to widget container
     * @param name name of dashboard widget
     * @param topic topic that is received by widget
     */
    void createSwitch(QWidget *interface, QString name, QString topic);

    /**
     * @brief Creates dashboard widget for displaying number value in interface
     * @param interface pointer to widget container
     * @param name name of dashboard widget
     * @param topic topic that is received by widget
     */
    void createDisplay(QWidget *interface, QString name, QString topic);

    /**
     * @brief Creates dashboard widget for displaying text in interface
     * @param interface pointer to widget container
     * @param name name of dashboard widget
     * @param topic topic that is received by widget
     */
    void createText(QWidget *interface, QString name, QString topic);

    /**
     * @brief Changes state of switch depending on received msg
     * @param msg message receiver from mqtt broker
     * @param interface pointer to widget container
     */
    void messageSwitchHandler(mqtt::const_message_ptr msg, QWidget *interface);

    /**
     * @brief Displays value from msg to LCDnumber widget
     * @param msg message receiver from mqtt broker
     * @param interface pointer to widget container
     */
    void messageDisplayHandler(mqtt::const_message_ptr msg, QWidget *interface);

    /**
     * @brief Appends msg payload to text area in widget determined by interface
     * @param msg message receiver from mqtt broker
     * @param interface pointer to widget container
     */
    void messageTextHandler(mqtt::const_message_ptr msg, QWidget *interface);

    /**
     * @brief Returns pointer to dashboard widget container depeding on index
     * @param index index of widget container
     * @return pointer to widget container
     */
    QWidget *getWidgetPtr(int index);

    /**
     * @brief Present a dialog
     * @param title of the dialog window
     * @param text of the dialgo window
     */
    static void presentDialog(QString title, QString text);
};

#endif // MAINWINDOW_H
