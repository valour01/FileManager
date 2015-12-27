#include "pull_file.h"
#include "ui_pull_file.h"

pull_file::pull_file(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::pull_file)
{
    ui->setupUi(this);
}

pull_file::~pull_file()
{
    delete ui;
}

void pull_file::on_pushButton_clicked()
{
    //log in to the server;
    QString ftext = ui->remote_path->text();
    QByteArray hostname_array = ftext.toLatin1();
     char *remote_path = hostname_array.data();
    //cout<<"hostname:"<<hostname<<endl;

    QString ftext_2 = ui->local_path->text();
    QByteArray username_array = ftext_2.toLatin1();
    //cout<<"hostname:"<<hostname<<endl;
     char *local_path = username_array.data();

    //emit SignalPull(char * remote_path,char * local_path);
     emit SignalPull(remote_path,local_path);
    ui->remote_path->clear();
    ui->local_path->clear();
    this->hide();

}

void pull_file::on_pushButton_2_clicked()
{
    this->hide();
}
