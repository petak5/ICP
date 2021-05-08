#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include "valueinspectdialog.h"
#include "mqtthandler.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void newMessage(QString topic, QString message);

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

    QTreeWidgetItem * treeViewAddRootItem(QString name);
    QTreeWidgetItem * treeViewAddItem(QTreeWidgetItem *parent, QString name);
};
#endif // MAINWINDOW_H
