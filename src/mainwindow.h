/**
 * @file mainwindow.h
 * @brief Header file for main window class
 * @author Peter Urgoš (xurgos00)
 * @date 9.5.2021
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include "valueinspectdialog.h"
#include "mqtthandler.h"
#include <QDir>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Topic
{
public:
    Topic(QString topic);
    QString getTopic();
    void addMessage(std::string message, int maxCount);
    QList<std::string *> &getMessages(int maxCount);

    Topic * findTopic(QStringList path);
    Topic * addTopic(Topic *topic);

    void exportToDisk(QDir directory);

private:
    QString topic;
    QList<std::string *> messages;
    QList<Topic *> children;

    void addChild(Topic *topic);
    QList<Topic *> &getChildren();
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void newMessage(QString topic, std::string payload);

    QString topicsFilter = "";

    void messageHandler(mqtt::const_message_ptr msg);

public slots:
    void on_widgetSwitchButton_clicked();

    void on_widgetTextButton_clicked();

private slots:
    void on_subscribeButton_clicked();

    void on_subscribeResetButton_clicked();

    void on_valueInspectButton_clicked();

    void on_publishTextButton_clicked();

    void on_publishFileButton_clicked();

    void on_connectToServerButton_clicked();

    void on_valueHistoryList_itemSelectionChanged();

    void on_treeWidget_itemSelectionChanged();

    void on_numberOfMessagesSetButton_clicked();

    void on_exportButton_clicked();

    void on_widgetAddButton_clicked();

    void on_widgetRemoveButton_clicked();

private:
    Ui::MainWindow *ui;
    MqttHandler *mqttHandler = nullptr;
    QList<Topic *> topicsTree;
    int numberOfMessagesInHistory = 1;

    QTreeWidgetItem * treeViewAddRootItem(QString name);
    QTreeWidgetItem * treeViewAddItem(QTreeWidgetItem *parent, QString name);
    QStringList treeViewGetPathToCurrentItem();
    Topic *treeViewFindTopic(QStringList path);
    void refreshValuesList();

    void createSwitch(QWidget *interface, QString name, QString topic);
    void createDisplay(QWidget *interface, QString name, QString topic);
    void createText(QWidget *interface, QString name, QString topic);
    void messageSwitchHandler(mqtt::const_message_ptr msg, QWidget *interface);
    void messageDisplayHandler(mqtt::const_message_ptr msg, QWidget *interface);
    void messageTextHandler(mqtt::const_message_ptr msg, QWidget *interface);
    QWidget *getWidgetPtr(int index);
};

#endif // MAINWINDOW_H
