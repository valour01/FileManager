#ifndef ATRDIALOG_H
#define ATRDIALOG_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class atrdialog;
}

class atrdialog : public QDialog
{
    Q_OBJECT

public:
    explicit atrdialog(QWidget *parent = 0);
    ~atrdialog();
    void showMsg();
private slots:
    void setStat(QString name);
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::atrdialog *ui;
    QString atrName;
    void mysize(char * path, int * size);
};

#endif // ATRDIALOG_H
