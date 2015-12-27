#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#define LIBSSH_STATIC 1
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "smtp.h"
#include <stdio.h>

namespace Ui {
class LogInDialog;
}

class LogInDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LogInDialog(QWidget *parent = 0);
    //void setlabel(char * content);
    ~LogInDialog();

private slots:
    //void on_buttonBox_clicked(QAbstractButton *button);

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

private:
    Ui::LogInDialog *ui;

signals:
    void SignalLogin(char * hostname,char * username,char * password, int port);
};

#endif // LOGINDIALOG_H
