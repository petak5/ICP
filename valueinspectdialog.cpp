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

void ValueInspectDialog::setMessage(QString message)
{
    ui->plainTextEdit->setPlainText(message);
}
