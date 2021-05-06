#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


// -------- //
// Tree tab //
// -------- //

/**
 * @brief Subscibe to a topic
 */
void MainWindow::on_subscribeButton_clicked()
{
    auto topic = ui->subscribeTopicTextField->text();

    if (topic.isEmpty()) return;

    // model.setTopic(topic);
}


/**
 * @brief Reset topic subscibtion to no topic (display all topics)
 */
void MainWindow::on_subscribeResetButton_clicked()
{
    // model.resetTopic();
}


/**
 * @brief Inspect value in a modal window
 */
void MainWindow::on_valueInspectButton_clicked()
{
    // OpenModal(with value of the message);
}


/**
 * @brief Publish text message to a topic
 */
void MainWindow::on_publishTextButton_clicked()
{
    auto topic = ui->publishTopicTextField->text();
    auto message = ui->publishTextMessageTextField->text();

    if (topic.isEmpty() || message.isEmpty()) return;

    // model.publishMessage(topic, message);
}


/**
 * @brief Publish file to a topic
 */
void MainWindow::on_publishFileButton_clicked()
{
    auto topic = ui->publishTopicTextField->text();
    auto filePath = ui->publishFilePathTextField->text();

    if (topic.isEmpty() || filePath.isEmpty()) return;

    // auto content = readFile(filePath);
    // model.publishMessage(topic, message);
}

// ------------ //
// Settings tab //
// ------------ //

/**
 * @brief Connect to server (MQTT broker)
 */
void MainWindow::on_connectToServerButton_clicked()
{
    auto address = ui->AddressTextField->text();
    auto port = ui->PortTextField->text();
    auto username = ui->UsernameTextField->text();
    auto password = ui->passwordTextField->text();

    if (address.isEmpty() || port.isEmpty()) return;

    // model.connectToServer(address, port, username, password);
}

