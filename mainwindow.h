#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include "valueinspectdialog.h"
#include "mqtthandler.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Topic
{
public:
    Topic(QString topic);
    QString getTopic();
    void addMessage(std::string message);
    QList<std::string *> &getMessages();

    Topic * findTopic(QStringList path);
    Topic * addTopic(Topic *topic);

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

private slots:
    void on_subscribeButton_clicked();

    void on_subscribeResetButton_clicked();

    void on_valueInspectButton_clicked();

    void on_publishTextButton_clicked();

    void on_publishFileButton_clicked();

    void on_connectToServerButton_clicked();

    void on_valueHistoryList_itemSelectionChanged();

    void on_treeWidget_itemSelectionChanged();

private:
    Ui::MainWindow *ui;
    MqttHandler *mqttHandler = nullptr;
    QList<Topic *> topicsTree;

    QTreeWidgetItem * treeViewAddRootItem(QString name);
    QTreeWidgetItem * treeViewAddItem(QTreeWidgetItem *parent, QString name);
    QStringList treeViewGetPathToCurrentItem();
    Topic *treeViewFindTopic(QStringList path);
    void refreshValuesList();
};

#endif // MAINWINDOW_H
