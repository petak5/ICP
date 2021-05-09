#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <fstream>

Topic::Topic(QString topic) : topic(topic) {}


/**
 * @brief Get topic name
 * @return topic name
 */
QString Topic::getTopic() { return topic; }


/**
 * @brief Add message to topic
 * @param message to add
 */
void Topic::addMessage(std::string message) { messages.append(new std::string(message)); }


/**
 * @brief Get all messages
 * @return list of messages
 */
QList<std::string *> &Topic::getMessages() { return messages; };


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


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), topicsTree()
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
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

    topicObject->addMessage(payload);

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

    auto messages = topic->getMessages();
    QStringList values;
    for (int i = 0; i < messages.length(); i++)
    {
        values.append(QString::fromStdString(messages.at(i)->data()));
    }

    ui->valueHistoryList->addItems(values);

    // Select the first item
    ui->valueHistoryList->clearSelection();
    auto firstItem = ui->valueHistoryList->item(0);
    if (firstItem != nullptr)
        ui->valueHistoryList->setCurrentItem(firstItem);
}


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

    if (topic.isEmpty()) return;

    mqttHandler->subscribe(topic);
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
    auto message = ui->valueTextField->text();
    if (message.isEmpty()) return;

    auto dialog = new ValueInspectDialog();
    dialog->setMessage(message);
    dialog->exec();
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

    mqttHandler->publishMessage(topic, message);
}


/**
 * @brief Publish file to a topic
 */
void MainWindow::on_publishFileButton_clicked()
{
    auto topic = ui->publishTopicTextField->text();
    auto filePath = ui->publishFilePathTextField->text().trimmed();

    if (topic.isEmpty() || filePath.isEmpty()) return;

    std::ifstream ifs(filePath.toStdString());
    std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

    mqttHandler->publishMessage(topic, content.c_str());
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
        auto username = ui->UsernameTextField->text();
        auto password = ui->passwordTextField->text();

        if (address.isEmpty() || port.isEmpty()) return;

        mqttHandler = new MqttHandler(address, port, this);

        if (mqttHandler != nullptr)
        {
            ui->connectToServerButton->setText("Disconnect");

            ui->subscribeButton->setEnabled(true);
            ui->subscribeResetButton->setEnabled(true);
            ui->publishTextButton->setEnabled(true);
            ui->publishFileButton->setEnabled(true);
        }
    }
    else
    {
        delete mqttHandler;
        mqttHandler = nullptr;

        ui->connectToServerButton->setText("Connect");

        ui->subscribeButton->setEnabled(false);
        ui->subscribeResetButton->setEnabled(false);
        ui->publishTextButton->setEnabled(false);
        ui->publishFileButton->setEnabled(false);
    }
}

