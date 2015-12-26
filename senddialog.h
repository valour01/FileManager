#ifndef SENDDIALOG_H
#define SENDDIALOG_H

#include <QDialog>

namespace Ui {
class sendDialog;
}

class sendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit sendDialog(QWidget *parent = 0);
    ~sendDialog();

private:
    Ui::sendDialog *ui;
};

#endif // SENDDIALOG_H
