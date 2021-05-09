#ifndef VALUEINSPECTDIALOG_H
#define VALUEINSPECTDIALOG_H

#include <QDialog>

namespace Ui {
class ValueInspectDialog;
}

class ValueInspectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ValueInspectDialog(QWidget *parent = nullptr);
    ~ValueInspectDialog();
    void setMessage(std::string message);

private:
    Ui::ValueInspectDialog *ui;
};

#endif // VALUEINSPECTDIALOG_H
