#ifndef WIDGETFACTORY_CPP
#define WIDGETFACTORY_CPP

#include "mainwindow.h"
#include <QtWidgets>
#include <QTranslator>
#include "ui_mainwindow.h"
#include <QSizePolicy>

void MainWindow::createSwitch(QWidget* interface, QString name, QString topic)
{
    QLabel *nameLabel = new QLabel(name, interface, Qt::Widget);
    nameLabel->setAlignment(Qt::AlignHCenter);
    nameLabel->setText(name);

    QPixmap pixmap("../icons/light_switch.png");
    QLabel *icon = new QLabel("switch", interface, Qt::Widget);
    icon->setPixmap(pixmap);
    icon->setAlignment(Qt::AlignHCenter);

    QLabel *status = new QLabel("OFF", interface, Qt::Widget);
    status->setAlignment(Qt::AlignHCenter);
    status->setText("OFF");
    status->setObjectName("widgetSwitchStatusText");

    QPushButton * button = new QPushButton("Switch", interface);
    button->show();
    button->setObjectName("widgetSwitchButton");
    connect(button, &QPushButton::clicked, this, &MainWindow::on_widgetSwitchButton_clicked);


    QVBoxLayout *layout = new QVBoxLayout(interface);
    layout->addWidget(nameLabel);
    layout->addWidget(icon);
    layout->addWidget(status);
    layout->addWidget(button);
}

void MainWindow::createDisplay(QWidget *interface, QString name, QString topic)
{
    QLabel *nameLabel = new QLabel(name, interface, Qt::Widget);
    nameLabel->setAlignment(Qt::AlignHCenter);
    nameLabel->setText(name);

    QLCDNumber *display =  new QLCDNumber(4, interface);
    display->setDecMode();
    display->setSmallDecimalPoint(true);
    display->show();
    display->setObjectName("widgetTemperatureDisplay");
    display->display(42.42);

    QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    display->setSizePolicy(policy);

    QVBoxLayout *layout = new QVBoxLayout(interface);
    layout->addWidget(nameLabel);
    layout->addWidget(display);
}

void MainWindow::createText(QWidget *interface, QString name, QString topic)
{
    QLabel *nameLabel = new QLabel(name, interface, Qt::Widget);
    nameLabel->setAlignment(Qt::AlignHCenter);
    nameLabel->setText(name);

    QScrollArea *scroll = new QScrollArea(interface);
    scroll->autoFillBackground();
    scroll->setObjectName("widgetTextScrollArea");

    QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    scroll->setSizePolicy(policy);

    QPushButton * button = new QPushButton("Send", interface);
    button->show();
    button->setObjectName("widgetTextButton");
    connect(button, &QPushButton::clicked, this, &MainWindow::on_widgetTextButton_clicked);

    QLineEdit *text = new QLineEdit(interface);
    text->setObjectName("widgetTextAddText");

    QGridLayout *layout = new QGridLayout(interface);
    layout->addWidget(nameLabel, 1, 1, 1, 2, Qt::AlignHCenter);
    layout->addWidget(scroll, 2, 1, 1, 2, Qt::AlignHCenter);
    layout->addWidget(text, 3, 1, 1, 1, Qt::AlignLeft);
    layout->addWidget(button, 3, 2, 1, 1, Qt::AlignRight);
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
