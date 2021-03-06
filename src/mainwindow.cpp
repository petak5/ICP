/**
 * @file mainwindow.cpp
 * @brief Source file of main window class
 * @author Peter Urgoš (xurgos00)
 * @author Adam Kľučiar (xkluci01)
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
#include <QThread>
#include <QUrl>

Topic::Topic(QString topic) : topic(topic) {}


QString Topic::getTopic() { return topic; }


void Topic::addMessage(std::string message, int maxCount)
{
    messages.append(new std::string(message));

    if (messages.length() > maxCount)
        messages.removeFirst();
}


QList<std::string *> &Topic::getMessages(int maxCount)
{
    while (messages.length() > maxCount)
        messages.removeFirst();

    return messages;
};


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

    // Set validator for number of messages stored text field
    ui->numberOfMessagesTextField->setValidator(new QIntValidator(0, 100, this));
}

MainWindow::~MainWindow()
{
    if (simulator != nullptr)
        simulator->stop();
    delete ui;
}


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

QTreeWidgetItem * MainWindow::treeViewAddRootItem(QString title)
{
    auto rootItem = new QTreeWidgetItem(ui->treeWidget);
    rootItem->setText(0, title);
    //ui->treeWidget->addTopLevelItem(rootItem);

    return rootItem;
}


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
}


void MainWindow::on_treeWidget_itemSelectionChanged()
{
    refreshValuesList();
}


void MainWindow::on_subscribeButton_clicked()
{
    auto topic = ui->subscribeTopicTextField->text();

    if (topic.isEmpty())
    {
        presentDialog("No topic provided", "Please provide a topic to subscribe. To subscribe to all topics press Reset button below Subscibe button.");
        return;
    }

    topicsFilter = topic;
}


void MainWindow::on_subscribeResetButton_clicked()
{
    topicsFilter = "";
}


void MainWindow::on_valueInspectButton_clicked()
{
    auto selectedIndex = ui->valueHistoryList->currentIndex();
    if (!selectedIndex.isValid())
    {
        presentDialog("No value selected", "Please select a value from value history list.");
        return;
    }

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


void MainWindow::on_valueHistoryList_itemSelectionChanged()
{
    auto item = ui->valueHistoryList->currentItem();
    ui->valueTextField->setText(item->text());
}


void MainWindow::on_publishTextButton_clicked()
{
    auto topic = ui->publishTopicTextField->text();
    auto message = ui->publishTextMessageTextField->text();

    if (topic.isEmpty())
    {
        presentDialog("No topic provided", "Please provide a topic for sending the message.");
        return;
    }
    if (message.isEmpty())
    {
        presentDialog("No message provided", "Please provide a message to send.");
        return;
    }

    mqttHandler->publishMessage(topic, message.toStdString());
}


void MainWindow::on_publishFileButton_clicked()
{
    auto topic = ui->publishTopicTextField->text();
    auto filePath = ui->publishFilePathTextField->text().trimmed();

    if (topic.isEmpty())
    {
        presentDialog("No topic provided", "Please provide a topic for sending the message.");
        return;
    }
    if (filePath.isEmpty())
    {
        presentDialog("No file path provided", "Please provide a path to file.");
        return;
    }

    QFile file(filePath);
    QFileInfo fileInfo(filePath);
    if (!fileInfo.isFile())
    {
        auto text = QString("File '").append(filePath).append("' either does not exist or is not a file. Please check if you've provided the correct path.");
        presentDialog("Invalid file name", text);
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

void MainWindow::on_connectToServerButton_clicked()
{
    if (mqttHandler == nullptr)
    {
        auto address = ui->AddressTextField->text();
        auto port = ui->PortTextField->text();

        if (address.isEmpty())
        {
            presentDialog("No address provided", "Please enter an address of server.");
            return;
        }
        if (port.isEmpty())
        {
            presentDialog("No port provided", "Please enter a port of server.");
            return;
        }

        QUrl url = QUrl::fromUserInput(QString(address).append(":").append(port));
        if (!url.isValid())
        {
            auto text = QString("'").append(url.url()).append("' is not a valid address.");
            presentDialog("Invalid address", text);
            return;
        }

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


void MainWindow::on_numberOfMessagesSetButton_clicked()
{
    auto numberString = ui->numberOfMessagesTextField->text();
    if (numberString.isEmpty())
    {
        presentDialog("No input provided", "Please enter a number for how many messages should be stored.");
        return;
    }

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
    {
        presentDialog("No path provided", "Please enter path where to save captured data.");
        return;
    }

    auto directory = QDir(directoryPath);
    if (!directory.isEmpty())
    {
        auto text = QString("Directory '").append(directory.path()).append("' is not empty. Please choose different path.");
        presentDialog("Directory is not empty", text);
        return;
    }

    for (int i = 0; i < topicsTree.length(); i++)
    {
        topicsTree.at(i)->exportToDisk(directory);
    }
}


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

    QWidget* interface;
    for(int i=1; i <= 12; i++)
    {
        interface = getWidgetPtr(i);
        if(interface->findChild<QWidget *>(QString(), Qt::FindDirectChildrenOnly) == nullptr)
        {
            break;
        }
    }

    if(widgetType == "Switch")
    {
        createSwitch(interface, widgetName, widgetTopic);
    }
    else if(widgetType == "Display")
    {
        createDisplay(interface, widgetName, widgetTopic);
    }
    else if(widgetType == "Text")
    {
        createText(interface, widgetName, widgetTopic);
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
    QLabel *label;
    for(int i=1; i<= 12; i++)
    {
        interface = getWidgetPtr(i);
        label =interface->findChild<QLabel *>("widgetNameLabel");

        if(label == nullptr)
        {
            continue;
        }

        name = label->text().trimmed();
        if(name == ui->widgetRemoveBox->currentText())
        {
            break;
        }
    }

    QLayout *layout = interface->findChild<QLayout *>(QString(), Qt::FindDirectChildrenOnly);
    qDeleteAll(layout->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly));
    delete layout;
    qDeleteAll(interface->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly));


    ui->widgetRemoveBox->removeItem(ui->widgetRemoveBox->currentIndex());
}


void MainWindow::on_widgetSwitchButton_clicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    QWidget *interface = button->parentWidget();
    QString topic = (interface->findChild<QLayout *>(QString(), Qt::FindDirectChildrenOnly))->objectName();

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

    stateLabel->setText(newState);
    mqttHandler->publishMessage(topic, newState.toStdString());
}


void MainWindow::on_widgetTextButton_clicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    QWidget *interface = button->parentWidget();
    QString topic = (interface->findChild<QLayout *>(QString(), Qt::FindDirectChildrenOnly))->objectName();

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

    lineEdit->clear();
    mqttHandler->publishMessage(topic, text.toStdString());
}


void MainWindow::messageHandler(mqtt::const_message_ptr msg)
{
    auto topic = QString().fromStdString(msg->get_topic());
    std::vector<QWidget *> interfaces;
    QLayout *widget;
    QLabel *label;

    for(int i=1; i <= 12 ; i++)
    {
        widget = getWidgetPtr(i)->findChild<QLayout *>(topic, Qt::FindDirectChildrenOnly);
        if(widget != nullptr)
        {
            interfaces.push_back(getWidgetPtr(i));
        }
    }

    if(interfaces.size() == 0)
    {
        return;
    }

    for(auto tmp : interfaces)
    {
        label = tmp->findChild<QLabel *>("widgetID");

        if(label == nullptr)
        {
            continue;
        }

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

    QPixmap pixmap("../src/icons/light_switch.png");
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

    QLabel *id = new QLabel("text");
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
    QString newState = "OFF";
    if(payload == "on" || payload == "On" || payload == "ON" || payload == "1")
    {
        newState = "ON";
    }

    if(label != nullptr)
    {
        label->setText(newState);
    }
}


void MainWindow::messageDisplayHandler(mqtt::const_message_ptr msg, QWidget *interface)
{
    auto payload = msg->get_payload();
    QLCDNumber* display = interface->findChild<QLCDNumber *>("widgetDisplay");

    if(display != nullptr)
    {
            display->display(QString().fromStdString(payload));
    }
}


void MainWindow::messageTextHandler(mqtt::const_message_ptr msg, QWidget *interface)
{
    auto payload = msg->get_payload();
    QTextEdit *text = interface->findChild<QTextEdit *>("widgetTextScrollArea");

    if(text != nullptr)
    {
        text->append(QString().fromStdString(payload));
    }
}


void MainWindow::presentDialog(QString title, QString text)
{
    QMessageBox dialog;
    dialog.setWindowTitle(title);
    dialog.setText(text);

    dialog.exec();
    return;
}

