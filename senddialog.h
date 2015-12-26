#ifndef SENDDIALOG_H
#define SENDDIALOG_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class sendDialog;
}

class sendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit sendDialog(QWidget *parent = 0);
    void showMsg();
    ~sendDialog();

private slots:
    void setStat(QString name);
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::sendDialog *ui;
    QString sendName;
};

#endif // SENDDIALOG_H
