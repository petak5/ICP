#include "mainwindow.h"

#include <QApplication>
#include <string>
#include <mqtt/client.h>

int main(int argc, char *argv[])
{
    mqtt::client client("localhost:1883", "ICP_project");
    try {
        client.connect();

        auto msg = "Hello MQTT";
        client.publish("ICP_main_topic", msg, strlen(msg), 0, false);

        client.disconnect();
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        return 1;
    }


    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
