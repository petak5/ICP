#include "valueinspectdialog.h"
#include "ui_valueinspectdialog.h"


ValueInspectDialog::ValueInspectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ValueInspectDialog)
{
    ui->setupUi(this);
}


ValueInspectDialog::~ValueInspectDialog()
{
    delete ui;
}


/**
 * @brief Set message to be shown in the dialog
 * @param message to show
 */
void ValueInspectDialog::setMessage(QString message)
{
    auto cppMessage = message.toStdString();
    QPixmap pixmap;
    QByteArray data(cppMessage.c_str(), cppMessage.length());

    if (pixmap.loadFromData(data))
    {
        ui->image_label->setPixmap(pixmap);
    }

    //ui->plainTextEdit->setPlainText(message);

    //QImage image(data);
    //image.save("/home/parallels/Desktop/facebook_copy.png");

    //QByteArray byteArray(message.toStdString().c_str(), message.length());
    //QPixmap picture(byteArray);

    //ui->image_label->setPixmap(picture);
}

