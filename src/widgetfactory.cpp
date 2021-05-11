#ifndef WIDGETFACTORY_CPP
#define WIDGETFACTORY_CPP

#include "mainwindow.h"
#include <QtWidgets>
#include <QTranslator>
#include "ui_mainwindow.h"
#include <QSizePolicy>

void MainWindow::createSwitch(QWidget* interface, QString name, QString topic)
{
    QLabel *nameLabel = new QLabel(name);
    nameLabel->setAlignment(Qt::AlignHCenter);
    nameLabel->setText(name);

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

#endif // WIDGETFACTORY_CPP
