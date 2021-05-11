/**
 * @file valueinspecdialog.h
 * @brief Header file for value inspect dialog class
 * @author Peter Urgoš (xurgos00)
 * @date 9.5.2021
 */

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

    /**
     * @brief Set message to be shown in the dialog, try to parse it as an image too
     * @param message to show
     */
    void setMessage(std::string message);

private:
    Ui::ValueInspectDialog *ui;
};

#endif // VALUEINSPECTDIALOG_H
