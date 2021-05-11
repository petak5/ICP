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
#include <QtWidgets>
#include <QSizePolicy>

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


// Main Window


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), topicsTree()
{
    ui->setupUi(this);

    ui->numberOfMessagesTextField->setValidator(new QIntValidator(0, 100, this));
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

    // Select the first item
    ui->valueHistoryList->clearSelection();
    auto firstItem = ui->valueHistoryList->item(0);
    if (firstItem != nullptr)
        ui->valueHistoryList->setCurrentItem(firstItem);
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

//TO DO: subsribe to topic if not subscribed
void MainWindow::on_widgetAddButton_clicked()
{
    auto widgetName = ui->widgetNameText->text().trimmed();
    auto widgetType = ui->widgetAddBox->currentText().trimmed();
    auto widgetTopic = ui->widgetTopicText->text().trimmed();

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

    if(widgetTopic == "")
    {
        QMessageBox dialog;
        dialog.setWindowTitle("Widget topic is empty");
        auto text = QString("Widget topic can't be empty. Please select a topic");
        dialog.setText(text);

        dialog.exec();
        return;
    }

    ui->widgetNameText->clear();
    ui->widgetTopicText->clear();

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

    if(widgetType == "Switch")
    {
        createSwitch(getWidgetPtr(ui->widgetRemoveBox->count()), widgetName, widgetTopic);
    }
    else if(widgetType == "Display")
    {
        createDisplay(getWidgetPtr(ui->widgetRemoveBox->count()), widgetName, widgetTopic);
    }
    else if(widgetType == "Text")
    {
        createText(getWidgetPtr(ui->widgetRemoveBox->count()), widgetName, widgetTopic);
    }
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

    QWidget *interface;
    QString name;
    for(int i=1; i<= 12; i++)
    {
        interface = getWidgetPtr(i);
        name = (interface->findChild<QLabel *>("widgetNameLabel"))->text().trimmed();
        if(name == ui->widgetRemoveBox->currentText())
        {
            break;
        }
    }
    //doesnt remove layout, needs fix
    qDeleteAll(interface->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly));

    ui->widgetRemoveBox->removeItem(ui->widgetRemoveBox->currentIndex());
}

void MainWindow::on_widgetSwitchButton_clicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    QWidget *interface = button->parentWidget();
    QString topic = interface->objectName();

    QLabel *stateLabel = interface->findChild<QLabel *>("widgetSwitchStatusText");
    QString state = stateLabel->text().trimmed();
    QString newState;

    if(state == "OFF")
    {
        newState = "ON";
    }
    else
    {
        newState = "OFF";
    }

    mqttHandler->publishMessage(topic, newState.toStdString());
}

void MainWindow::on_widgetTextButton_clicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    QWidget *interface = button->parentWidget();
    QString topic = interface->objectName();

    QLineEdit *lineEdit = interface->findChild<QLineEdit *>("widgetTextAddText");
    QString text = lineEdit->text().trimmed();

    if(text == "")
    {
        QMessageBox dialog;
        dialog.setWindowTitle("Message empty");
        auto text = QString("Message can't be empty. Please write a message before sending.");
        dialog.setText(text);

        dialog.exec();
        return;
    }

    mqttHandler->publishMessage(topic, text.toStdString());
}

void MainWindow::messageHandler(mqtt::const_message_ptr msg)
{
    auto topic = QString().fromStdString(msg->get_topic());
    std::vector<QWidget *> interfaces;
    QWidget *widget;
    QLabel *label;

    for(int i=1; i <= 12 ; i++)
    {
        widget = getWidgetPtr(i)->findChild<QWidget *>(topic, Qt::FindDirectChildrenOnly);
        if(widget != nullptr)
        {
            interfaces.push_back(widget);
        }
    }

    if(interfaces.size() == 0)
    {
        return;
    }

    for(auto tmp : interfaces)
    {
        label = tmp->findChild<QLabel *>("widgetID", Qt::FindDirectChildrenOnly);

        if(label->text() == "switch")
        {
            messageSwitchHandler(msg, tmp);
        }
        else if(label->text() == "display")
        {
            messageDisplayHandler(msg, tmp);
        }
        else if(label->text() == "text")
        {
            messageTextHandler(msg, tmp);
        }
    }

}

QWidget *MainWindow::getWidgetPtr(int index)
{
    switch(index)
    {
        case 1:
            return ui->widget_1;
        case 2:
            return ui->widget_2;
        case 3:
            return ui->widget_3;
        case 4:
            return ui->widget_4;
        case 5:
            return ui->widget_5;
        case 6:
            return ui->widget_6;
        case 7:
            return ui->widget_7;
        case 8:
            return ui->widget_8;
        case 9:
            return ui->widget_9;
        case 10:
            return ui->widget_10;
        case 11:
            return ui->widget_11;
        case 12:
            return ui->widget_12;
    }

    return nullptr;
}

void MainWindow::createSwitch(QWidget* interface, QString name, QString topic)
{
    QLabel *nameLabel = new QLabel(name);
    nameLabel->setAlignment(Qt::AlignHCenter);
    nameLabel->setText(name);
    nameLabel->setObjectName("widgetNameLabel");

    QPixmap pixmap("../icons/light_switch.png");
    QLabel *icon = new QLabel("switch");
    icon->setPixmap(pixmap);
    icon->setAlignment(Qt::AlignHCenter);

    QLabel *status = new QLabel("OFF");
    status->setAlignment(Qt::AlignHCenter);
    status->setText("OFF");
    status->setObjectName("widgetSwitchStatusText");

    QPushButton * button = new QPushButton("Switch");
    button->show();
    button->setObjectName("widgetSwitchButton");
    connect(button, &QPushButton::clicked, this, &MainWindow::on_widgetSwitchButton_clicked);

    QLabel *id = new QLabel("switch");
    id->setVisible(false);
    id->setObjectName("widgetID");

    QVBoxLayout *layout = new QVBoxLayout(interface);
    layout->addWidget(nameLabel);
    layout->addWidget(icon);
    layout->addWidget(status);
    layout->addWidget(button);
    layout->setObjectName(topic);
    layout->addWidget(id);
}

void MainWindow::createDisplay(QWidget *interface, QString name, QString topic)
{
    QVBoxLayout *layout = new QVBoxLayout(interface);

    QLabel *nameLabel = new QLabel(name);
    nameLabel->setAlignment(Qt::AlignHCenter);
    nameLabel->setText(name);
    nameLabel->setObjectName("widgetNameLabel");

    QLCDNumber *display =  new QLCDNumber(10);
    display->setDecMode();
    display->setSmallDecimalPoint(true);
    display->show();
    display->setObjectName("widgetDisplay");
    display->display(42.42);

    QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    display->setSizePolicy(policy);

    QLabel *id = new QLabel("display");
    id->setVisible(false);
    id->setObjectName("widgetID");

    layout->addWidget(nameLabel);
    layout->addWidget(display);
    layout->setObjectName(topic);
    layout->addWidget(id);
}

void MainWindow::createText(QWidget *interface, QString name, QString topic)
{
    QLabel *nameLabel = new QLabel(name);
    nameLabel->setAlignment(Qt::AlignHCenter);
    nameLabel->setText(name);
    nameLabel->setObjectName("widgetNameLabel");

    QTextEdit *display = new QTextEdit();
    display->setReadOnly(true);
    display->setObjectName("widgetTextScrollArea");

    QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    display->setSizePolicy(policy);

    QPushButton * button = new QPushButton("Send");
    button->show();
    button->setObjectName("widgetTextButton");
    connect(button, &QPushButton::clicked, this, &MainWindow::on_widgetTextButton_clicked);

    QLineEdit *text = new QLineEdit();
    text->setObjectName("widgetTextAddText");

    QLabel *id = new QLabel("switch");
    id->setVisible(false);
    id->setObjectName("widgetID");

    QGridLayout *layout = new QGridLayout(interface);
    layout->addWidget(nameLabel, 1, 1, 1, 2, Qt::AlignHCenter);
    layout->addWidget(display, 2, 1, 1, 2, Qt::AlignHCenter);
    layout->addWidget(text, 3, 1, 1, 1, Qt::AlignLeft);
    layout->addWidget(button, 3, 2, 1, 1, Qt::AlignRight);
    layout->addWidget(id);
    layout->setObjectName(topic);
}

void MainWindow::messageSwitchHandler(mqtt::const_message_ptr msg, QWidget *interface)
{
    auto payload = msg->get_payload();
    QLabel *label = interface->findChild<QLabel *>("widgetSwitchStatusText");

    label->setText(QString().fromStdString(payload));
}

void MainWindow::messageDisplayHandler(mqtt::const_message_ptr msg, QWidget *interface)
{
    auto payload = msg->get_payload();
    QLCDNumber* display = interface->findChild<QLCDNumber *>("widgetDisplay");

    display->display(QString().fromStdString(payload));
}

void MainWindow::messageTextHandler(mqtt::const_message_ptr msg, QWidget *interface)
{
    auto payload = msg->get_payload();
    QTextEdit *text = interface->findChild<QTextEdit *>("widgetTextScrollArea");

    text->append(QString().fromStdString(payload));
}
