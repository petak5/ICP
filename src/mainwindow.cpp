/**
 * @file mainwindow.cpp
 * @brief Source file of main window class
 * @author Peter Urgoš (xurgos00)
 * @date 9.5.2021
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <fstream>
#include <limits>
#include <QFile>
#include <QMessageBox>
#include <QThread>

Topic::Topic(QString topic) : topic(topic) {}


/**
 * @brief Get topic name
 * @return topic name
 */
QString Topic::getTopic() { return topic; }


/**
 * @brief Add message to topic
 * @param message to add
 * @param maxCount of messages stored
 */
void Topic::addMessage(std::string message, int maxCount)
{
    messages.append(new std::string(message));

    if (messages.length() > maxCount)
        messages.removeFirst();
}


/**
 * @brief Get all messages
 * @param maxCount of mesages stored
 * @return list of messages
 */
QList<std::string *> &Topic::getMessages(int maxCount)
{
    while (messages.length() > maxCount)
        messages.removeFirst();

    return messages;
};


/**
 * @brief Add topic to children
 * @param topic to add
 */
void Topic::addChild(Topic *topic) { children.append(topic); }


/**
 * @brief Get all children
 * @return list of children
 */
QList<Topic *> &Topic::getChildren() { return children; }


/**
 * @brief Find topic in the topics tree at the specified path
 * @param path is path to the topic in the tree
 * @return topic at specified path or nullptr if not found
 */
Topic * Topic::findTopic(QStringList path)
{
    if (path.length() == 0) return nullptr;

    // Last element of path
    if (path.length() == 1)
    {
        if (path[0] == this->topic)
            return this;
    }

    if (path[0] != this->topic)
        return nullptr;

    path.removeFirst();
    for (int i = 0; i < children.length(); i++)
    {
        auto topic = children.at(i)->findTopic(path);

        if (topic != nullptr)
            return topic;
    }

    return nullptr;
}


/**
 * @brief Add topic to tree, the location is determined by the topic attribute which is used as path in the tree
 * @param topic to add to topic's tree structure
 * @return the topic that was added to the tree
 */
Topic * Topic::addTopic(Topic *topic)
{
    auto path = topic->topic.split("/");

    path.removeFirst();

    auto currentNode = this;
    for (int i = 0; i < path.length(); i++)
    {
        bool found = false;
        for (int j = 0; j < currentNode->children.length(); j++)
        {
            if (currentNode->children.at(j)->topic == path[i])
            {
                found = true;
                currentNode = currentNode->children.at(j);
                break;
            }
        }

        if (!found)
        {
            auto temp = new Topic(path[i]);
            currentNode->addChild(temp);
            currentNode = temp;
        }
    }

    return currentNode;
}


void Topic::exportToDisk(QDir directory)
{
    if (!directory.exists())
        directory.mkdir(directory.path());

    QDir newDir(directory.path().append("/").append(topic));
    if (!newDir.exists())
        newDir.mkdir(newDir.path());

    if (messages.length() > 0)
    {
        auto lastMessage = messages.last();

        auto payloadPath = newDir.path();

        // Figure out file type, only PNG and JPG is supported, everything else is just data in TXT
        QPixmap dummyPixmap;
        QByteArray data(lastMessage->data(), lastMessage->length());

        if (dummyPixmap.loadFromData(data, "PNG"))
            payloadPath.append("/payload.png");
        else if (dummyPixmap.loadFromData(data, "JPG"))
            payloadPath.append("/payload.jpg");
        else
            payloadPath.append("/payload.txt");

        // Write payload
        QFile payloadFile(payloadPath);
        payloadFile.open(QIODevice::WriteOnly);
        payloadFile.write(lastMessage->data(), lastMessage->length());
        payloadFile.close();
    }

    for (int i = 0; i < children.length(); i++)
    {
        children.at(i)->exportToDisk(newDir);
    }
}

//-------------//
// Main Window //
//-------------//

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), topicsTree()
{
    ui->setupUi(this);

    ui->numberOfMessagesTextField->setValidator(new QIntValidator(0, 100, this));
}

MainWindow::~MainWindow()
{
    if (simulator != nullptr)
        simulator->stop();
    delete ui;
}


/**
 * @brief Function that is called when new message is received in the Paho client callback, updates UI with the new message
 * @param topic is the topic of the message
 * @param message is the message's conetnt (payload)
 */
void MainWindow::newMessage(QString topic, std::string payload)
{
    auto topicPath = topic.split(QString("/"));
    if (topicPath.length() <= 0)
        return;

    int topicsRowIndex = -1;
    for (int i = 0; i < topicsTree.length(); i++)
    {
        if (topicsTree.at(i)->getTopic() == topicPath[0])
        {
            topicsRowIndex = i;
            break;
        }
    }

    // Topics branch not found in the tree
    if (topicsRowIndex == -1)
    {
        topicsTree.append(new Topic(topicPath[0]));
        topicsRowIndex = topicsTree.length() - 1;
    }

    auto row = topicsTree.at(topicsRowIndex);
    //topicPath.removeFirst();
    auto topicObject = row->findTopic(topicPath);

    if (topicObject == nullptr)
    {
        // Add topic to backend model
        topicObject = row->addTopic(new Topic(topic));
    }

    topicObject->addMessage(payload, numberOfMessagesInHistory);



    // Append topics/subtopics to UI tree
    if (topicPath.length() > 0)
    {
        auto foundItems = ui->treeWidget->findItems(topicPath[0], Qt::MatchExactly);

        QTreeWidgetItem *currentItem = nullptr;
        if (foundItems.empty())
            currentItem = treeViewAddRootItem(topicPath[0]);
        else
            currentItem = foundItems.first();

        topicPath.removeFirst();
        for (int i = 0; i < topicPath.length(); i ++)
        {
            bool found = false;
            for (int j = 0; j < currentItem->childCount(); j++)
            {
                if (currentItem->child(j)->text(0) == topicPath[i])
                {
                    found = true;
                    currentItem = currentItem->child(j);
                    break;
                }
            }

            if (!found)
            {
                currentItem = treeViewAddItem(currentItem, topicPath[i]);
            }
        }

        refreshValuesList();
    }
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
 * @brief Get path to currently selected item in tree view
 * @return path to current item, empty path if not found
 */
QStringList MainWindow::treeViewGetPathToCurrentItem()
{
    // Get path from tree
    auto currentItem = ui->treeWidget->currentItem();
    if (currentItem == nullptr)
        return QStringList();

    auto itemPath = QStringList(currentItem->text(0));

    while (currentItem->parent() != NULL)
    {
        itemPath.prepend(currentItem->parent()->text(0));
        currentItem = currentItem->parent();
    }

    return itemPath;
}


/**
 * @brief Find topic at the specified path in the topics tree structure
 * @param path is the path to the requested topic
 * @return topic at the path or nullptr if not found
 */
Topic *MainWindow::treeViewFindTopic(QStringList path)
{
    Topic *rootTopic = nullptr;
    for (int i = 0; i < topicsTree.length(); i++)
    {
        if (topicsTree.at(i)->getTopic() == path[0])
        {
            rootTopic = topicsTree.at(i);
            break;
        }
    }

    if (rootTopic == nullptr)
        return nullptr;


    auto topic = rootTopic->findTopic(path);
    return topic;
}


/**
 * @brief Fills value history list with values related to the currently selected item in tree widget
 */
void MainWindow::refreshValuesList()
{
    ui->valueHistoryList->clear();
    ui->valueTextField->clear();

    auto itemPath = treeViewGetPathToCurrentItem();
    if (itemPath.empty())
        return;

    auto topic = treeViewFindTopic(itemPath);
    if (topic == nullptr)
        return;

    auto messages = topic->getMessages(numberOfMessagesInHistory);
    QStringList values;
    for (int i = 0; i < messages.length(); i++)
    {
        values.append(QString::fromStdString(messages.at(i)->data()));
    }

    ui->valueHistoryList->addItems(values);

    QThread::msleep(20);

    /*
     * QUrl url = QUrl::fromUserInput(usertext);
    if (checkUrl(url))
    */
}


/**
 * @brief Update list when tree view selection changed
 */
void MainWindow::on_treeWidget_itemSelectionChanged()
{
    refreshValuesList();
}


/**
 * @brief Subscibe to a topic
 */
void MainWindow::on_subscribeButton_clicked()
{
    auto topic = ui->subscribeTopicTextField->text();

    if (topic.isEmpty())
        return;

    topicsFilter = topic;
}


/**
 * @brief Reset topic subscibtion to no topic (receive all topics)
 */
void MainWindow::on_subscribeResetButton_clicked()
{
    topicsFilter = "";
}


/**
 * @brief Inspect value in a modal window
 */
void MainWindow::on_valueInspectButton_clicked()
{
    auto selectedIndex = ui->valueHistoryList->currentIndex();
    if (!selectedIndex.isValid())
        return;

    auto itemPath = treeViewGetPathToCurrentItem();
    if (itemPath.empty())
        return;

    auto topic = treeViewFindTopic(itemPath);
    if (topic == nullptr)
        return;

    auto messages = topic->getMessages(numberOfMessagesInHistory);

    // Some internal error probably
    if (selectedIndex.row() >= messages.length())
        return;

    auto message = messages.at(selectedIndex.row());

    auto dialog = new ValueInspectDialog();
    dialog->setMessage(*message);
    dialog->exec();
}


void MainWindow::on_valueHistoryList_itemDoubleClicked(QListWidgetItem *item)
{
    on_valueInspectButton_clicked();
}


/**
 * @brief Update displayed value when item from history is selected
 */
void MainWindow::on_valueHistoryList_itemSelectionChanged()
{
    auto item = ui->valueHistoryList->currentItem();
    ui->valueTextField->setText(item->text());
}


/**
 * @brief Publish text message to a topic
 */
void MainWindow::on_publishTextButton_clicked()
{
    auto topic = ui->publishTopicTextField->text();
    auto message = ui->publishTextMessageTextField->text();

    if (topic.isEmpty() || message.isEmpty()) return;

    mqttHandler->publishMessage(topic, message.toStdString());
}


/**
 * @brief Publish file to a topic
 */
void MainWindow::on_publishFileButton_clicked()
{
    auto topic = ui->publishTopicTextField->text();
    auto filePath = ui->publishFilePathTextField->text().trimmed();

    if (topic.isEmpty() || filePath.isEmpty()) return;

    QFile file(filePath);
    QFileInfo fileInfo(filePath);
    if (!fileInfo.isFile())
    {
        QMessageBox dialog;
        dialog.setWindowTitle("Invalid file name");
        auto text = QString("File '").append(filePath).append("' either does not exist or is not a file. Please check if you've provided the correct path.");
        dialog.setText(text);

        dialog.exec();
        return;
    }

    std::ifstream ifs(filePath.toStdString());
    std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

    if (content.empty())
        return;

    mqttHandler->publishMessage(topic, content);
}

// ------------ //
// Settings tab //
// ------------ //

/**
 * @brief Connect to or disconnect from server (MQTT broker)
 */
void MainWindow::on_connectToServerButton_clicked()
{
    if (mqttHandler == nullptr)
    {
        auto address = ui->AddressTextField->text();
        auto port = ui->PortTextField->text();

        if (address.isEmpty() || port.isEmpty()) return;

        mqttHandler = new MqttHandler(address, port, "xurgos00_ICP_explorer", this);

        if (mqttHandler != nullptr)
        {
            ui->connectToServerButton->setText("Disconnect");

            ui->publishTextButton->setEnabled(true);
            ui->publishFileButton->setEnabled(true);
            ui->simulatorButton->setEnabled(true);
        }
    }
    else
    {
        delete mqttHandler;
        mqttHandler = nullptr;

        ui->connectToServerButton->setText("Connect");

        if (simulator != nullptr)
            simulator->stop();
        ui->simulatorButton->setChecked(false);
        ui->simulatorButton->setText("Run");
        ui->publishTextButton->setEnabled(false);
        ui->publishFileButton->setEnabled(false);
        ui->simulatorButton->setEnabled(false);
    }
}


/**
 * @brief MainWindow::on_numberOfMessagesSetButton_clicked
 */
void MainWindow::on_numberOfMessagesSetButton_clicked()
{
    auto numberString = ui->numberOfMessagesTextField->text();
    numberOfMessagesInHistory = numberString.toInt();

    // Set to infinity when set to 0 (or lower)
    if (numberOfMessagesInHistory <= 0)
        numberOfMessagesInHistory = INT_MAX;

    refreshValuesList();
}


/**
 * @brief Export captured data to disk
 */
void MainWindow::on_exportButton_clicked()
{
    auto directoryPath = ui->exportPathTextField->text().trimmed();

    if (directoryPath.isEmpty())
        return;

    auto directory = QDir(directoryPath);
    if (!directory.isEmpty())
    {
        QMessageBox dialog;
        dialog.setWindowTitle("Directory is not empty");
        auto text = QString("Directory '").append(directory.path()).append("' is not empty. Please choose another path.");
        dialog.setText(text);

        dialog.exec();
        return;
    }

    for (int i = 0; i < topicsTree.length(); i++)
    {
        topicsTree.at(i)->exportToDisk(directory);
    }
}


/**
 * @brief Run or stop simulator
 */
void MainWindow::on_simulatorButton_clicked()
{
    if (mqttHandler == nullptr)
        return;

    if (simulator == nullptr)
        simulator = new Simulator(mqttHandler->getAddress(), mqttHandler->getPort(), "xurgos00_ICP_simulator");

    auto doSimulate = ui->simulatorButton->isChecked();

    if (doSimulate)
    {
        simulator->run();
        ui->simulatorButton->setText("Stop");
    }
    else
    {
        simulator->stop();
        ui->simulatorButton->setText("Run");
    }
}


//-----------//
// Dashboard //
//-----------//

void MainWindow::on_widgetAddButton_clicked()
{
    auto widgetName = ui->widgetNameText->text().trimmed();
    auto widgetType = ui->widgetAddBox->currentText().trimmed();

    if(ui->widgetRemoveBox->count() == 12)
    {
        QMessageBox dialog;
        dialog.setWindowTitle("Maximum number of widgets");
        auto text = QString("Can't create more widgets, maximum number of widgets has been reached. Please remove some widgets before creating new nes.");
        dialog.setText(text);

        dialog.exec();
        return;
    }

    if(widgetName == "")
    {
        QMessageBox dialog;
        dialog.setWindowTitle("Widget name is empty");
        auto text = QString("Widget name can't be empty. Please select a name");
        dialog.setText(text);

        dialog.exec();
        return;
    }

    ui->widgetNameText->clear();

    if(ui->widgetRemoveBox->findText(widgetName, Qt::MatchCaseSensitive|Qt::MatchExactly) != -1)
    {
        QMessageBox dialog;
        dialog.setWindowTitle("Widget name duplicate");
        auto text = QString("Widget name already exists. Please select a different name.");
        dialog.setText(text);

        dialog.exec();
        return;
    }

    ui->widgetRemoveBox->addItem(widgetName);

    //TO DO: create widget
}


void MainWindow::on_widgetRemoveButton_clicked()
{
    if(ui->widgetRemoveBox->count() == 0 || ui->widgetRemoveBox->currentText().trimmed() == "")
    {
        QMessageBox dialog;
        dialog.setWindowTitle("Widget count is zero");
        auto text = QString("No widget selected to delete or no widget exists.");
        dialog.setText(text);

        dialog.exec();
        return;
    }

    ui->widgetRemoveBox->removeItem(ui->widgetRemoveBox->currentIndex());

    //TO DO: remove widget
}

