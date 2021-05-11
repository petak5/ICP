# File: ICP.pro
# Brief: MQTT Explorer
# Author: Peter Urgoš (xurgos00)
# Date 9.5.2021

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
LIBS += -lpaho-mqtt3c -lpaho-mqtt3a -lpaho-mqttpp3

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    mqtthandler.cpp \
    valueinspectdialog.cpp

HEADERS += \
    mainwindow.h \
    mqtthandler.h \
    valueinspectdialog.h

FORMS += \
    mainwindow.ui \
    valueinspectdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES +=
