#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>
#include <iostream>
using namespace std;


LogInDialog::LogInDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogInDialog)
{
    ui->setupUi(this);
    ui->password->setEchoMode(QLineEdit::Password);
    //QObject::connect(this,SIGNAL(Login(QString)),&send,SLOT(setStat(QString)));
}

LogInDialog::~LogInDialog()
{
    delete ui;
}

/*void LogInDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    QString ftext = ui->lineEdit_3->text();

   // char * hostname = ftext.toLocal8Bit().data();


}*/

void LogInDialog::on_pushButton_2_clicked()
{
    this->hide();
}


void LogInDialog::on_pushButton_clicked()
{
    //log in to the server;
    QString ftext = ui->hostname->text();
    QByteArray hostname_array = ftext.toLatin1();
     char *hostname = hostname_array.data();
    //cout<<"hostname:"<<hostname<<endl;

    QString ftext_2 = ui->username->text();
    QByteArray username_array = ftext_2.toLatin1();
    //cout<<"hostname:"<<hostname<<endl;
     char *username = username_array.data();

    QString ftext_3 = ui->password->text();
    QByteArray password_array = ftext_3.toLatin1();
    char*  password = password_array.data();
    //cout<<"username:"<<username<<endl;

    QString ftext_4 = ui->port->text();
    QByteArray port_array = ftext_4.toLatin1();
     char * port = port_array.data();


    int int_port = atoi( port);
    //popup.show();
    emit SignalLogin(hostname,username,password,int_port);
    this->hide();
    ui->hostname->clear();
    ui->username->clear();
    ui->password->clear();
    ui->port->clear();

}
