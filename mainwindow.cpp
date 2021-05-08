#include "mainwindow.h"
#include "ui_mainwindow.h"

Topic::Topic(QString topic) : topic(topic) {}


QString Topic::getTopic() { return topic; }


void Topic::addMessage(QString message) { messages.append(new QString(message)); }


QList<QString *> &Topic::getMessages() { return messages; };


void Topic::addChild(Topic *topic) { children.append(topic); }


QList<Topic *> &Topic::getChildren() { return children; }


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


void MainWindow::newMessage(QString topic, QString message)
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

    topicObject->addMessage(message);

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


void MainWindow::refreshValuesList()
{
    ui->valueHistoryList->clear();

    // Get path from tree
    auto currentItem = ui->treeWidget->currentItem();
    if (currentItem == nullptr)
        return;
    auto itemPath = QStringList(currentItem->text(0));

    while (currentItem->parent() != NULL)
    {
        itemPath.prepend(currentItem->parent()->text(0));
        currentItem = currentItem->parent();
    }

    Topic *rootTopic = nullptr;
    for (int i = 0; i < topicsTree.length(); i++)
    {
        if (topicsTree.at(i)->getTopic() == itemPath[0])
        {
            rootTopic = topicsTree.at(i);
            break;
        }
    }

    if (rootTopic == nullptr)
        return;


    auto topic = rootTopic->findTopic(itemPath);
    if (topic == nullptr)
        return;

    auto messages = topic->getMessages();
    QStringList values;
    for (int i = 0; i < messages.length(); i++)
    {
        values.append(messages.at(i)->toStdString().c_str());
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
    auto filePath = ui->publishFilePathTextField->text();

    if (topic.isEmpty() || filePath.isEmpty()) return;

    // auto content = readFile(filePath);
    // model.publishMessage(topic, message);
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

        if (mqttHandler != nullptr) ui->connectToServerButton->setText("Disconnect");
    }
    else
    {
        delete mqttHandler;
        mqttHandler = nullptr;
        ui->connectToServerButton->setText("Connect");
    }
}

