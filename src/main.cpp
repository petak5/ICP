/**
 * @file main.cpp
 * @brief Entry point of the application, MQTT Explorer
 * @author Peter Urgoš (xurgos00)
 * @date 9.5.2021
 */

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
