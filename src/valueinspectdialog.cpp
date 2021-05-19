/**
 * @file valueinspecdialog.cpp
 * @brief Implementation of value inspect dialog class
 * @author Peter Urgoš (xurgos00)
 * @date 9.5.2021
 */

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


void ValueInspectDialog::setMessage(std::string message)
{
    ui->plainTextEdit->setPlainText(QString::fromStdString(message));

    QPixmap pixmap;
    QByteArray data(message.data(), message.length());

    if (pixmap.loadFromData(data))
    {
        ui->image_label->setPixmap(pixmap);

        // Focus second tab when data is identified as image
        ui->tabWidget->setCurrentIndex(1);
    }
}

