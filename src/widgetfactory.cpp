#ifndef WIDGETFACTORY_CPP
#define WIDGETFACTORY_CPP

#include "mainwindow.h"
#include <QtWidgets>
#include <QTranslator>
#include "ui_mainwindow.h"

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

QWidget *MainWindow::getEmptyWidget(int num)
{
    switch(num)
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
