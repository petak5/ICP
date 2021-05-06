#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Populate the tree with some dummies
    auto root_1 = treeViewAddRootItem("Root item 1");
    treeViewAddItem(root_1, "Child 1");
    treeViewAddItem(root_1, "Child 2");

    auto root_2 = treeViewAddRootItem("Root item 2");
    auto child = treeViewAddItem(root_2, "Child 3");
    treeViewAddItem(child, "Item 4");
}

MainWindow::~MainWindow()
{
    delete ui;
}


// -------- //
// Tree tab //
// -------- //

/**
 * @brief Add root item to tree
 * @param title of the item
 * @return Root item that was created and added to the QTreeWidget
 */
QTreeWidgetItem * MainWindow::treeViewAddRootItem(QString title)
{
    auto rootItem = new QTreeWidgetItem(ui->treeWidget);
    rootItem->setText(0, title);
    //ui->treeWidget->addTopLevelItem(rootItem);

    return rootItem;
}


/**
 * @brief Add item to the parent
 * @param parent to add item to
 * @param title of the item
 * @return The item that was created and added to the parent
 */
QTreeWidgetItem * MainWindow::treeViewAddItem(QTreeWidgetItem *parent, QString title)
{
    auto childItem = new QTreeWidgetItem(parent);
    childItem->setText(0, title);
    //parent->addChild(childItem);

    return childItem;
}


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

