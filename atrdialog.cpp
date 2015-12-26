#include "atrdialog.h"
#include "ui_atrdialog.h"
#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <QMessageBox>

atrdialog::atrdialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::atrdialog)
{
    ui->setupUi(this);
}

atrdialog::~atrdialog()
{
    delete ui;
}

void atrdialog::mysize(char *path, int *size)
{
    struct stat buf;
    int si = 0;
    if(stat(path,&buf) != 0)
    {
        QMessageBox::information(this,"Error","Get status error . ",QMessageBox::Yes,QMessageBox::Yes);
        return;
    }
    if(S_ISDIR(buf.st_mode))
    {
        int i ;
        char tmppath[1000];
        struct dirent * sp;
        DIR * dir;
        dir = opendir(path);
        while((sp=readdir(dir))!=NULL)
        {
            strcpy(tmppath,path);
            if(strcmp(sp->d_name,".")!=0 && strcmp(sp->d_name,"..") != 0)
            {
                strcat(tmppath,"/");
                strcat(tmppath,sp->d_name);
                mysize(tmppath,&i);
                si = si+i;
            }
        }
        closedir(dir);
    }
    else
    {
        si = buf.st_size;
    }
    *size = si;
}

void atrdialog::showMsg()
{
    super:
    QStringList tmp = this->atrName.split("/");
    QString path="";
    int num = tmp.count()-1;
    for(int i = 1; i < num; i++)
    {
        path = path+"/"+tmp.at(i);
    }
    QString fname = tmp.at(num);
    ui->label_name1->setText(fname);
    ui->label_location1->setText(path);
    struct stat buf;
    char * aname = this->atrName.toLocal8Bit().data();
    if(stat(aname,&buf)!=0)
    {
        QMessageBox::information(this,"Error","Stat error.",QMessageBox::Yes,QMessageBox::Yes);
    }
    else
    {
        if(S_ISREG(buf.st_mode))
        {
            ui->label_type1->setText("Regular File");
        }
        else if(S_ISDIR(buf.st_mode))
        {
            ui->label_type1->setText("Directory");
        }
        else if(S_ISCHR(buf.st_mode))
        {
            ui->label_type1->setText("Character Device");
        }
        else if(S_ISBLK(buf.st_mode))
        {
             ui->label_type1->setText("Block Device");
        }
        else if(S_ISFIFO(buf.st_mode))
        {
            ui->label_type1->setText("FIFO");
        }
        else if(S_ISLNK(buf.st_mode))
        {
            ui->label_type1->setText("Symbolic Link");
        }
        else if(S_ISSOCK(buf.st_mode))
        {
             ui->label_type1->setText("Socket");
        }
        int size;
        mysize(aname,&size);
        ui->label_size1->setText(QString::number(size) + "字节");
        tm * local;
        time_t stime = buf.st_ctim.tv_sec;
        local = localtime(&stime);
        char buftime[128]={0};
        strftime(buftime, 64, "%Y-%m-%d %H:%M:%S", local);
        QString sStime = QString::fromLocal8Bit(buftime);
        ui->label_stime1->setText(sStime);
        time_t mtime = buf.st_mtim.tv_sec;
        local = localtime(&mtime);
        buftime[128]={0};
        strftime(buftime, 64, "%Y-%m-%d %H:%M:%S", local);
        QString sMtime = QString::fromLocal8Bit(buftime);
        ui->label_chtime1->setText(sMtime);
        time_t atime = buf.st_atim.tv_sec;
        local = localtime(&atime);
        buftime[128]={0};
        strftime(buftime, 64, "%Y-%m-%d %H:%M:%S", local);
        QString sAtime = QString::fromLocal8Bit(buftime);
        ui->label_vtime1->setText(sAtime);
        if(buf.st_mode & S_IRUSR)
        {
            ui->readable->setChecked(true);
        }
        if(buf.st_mode & S_IWUSR)
        {
            ui->writeable->setChecked(true);
        }
        if(buf.st_mode & S_IXGRP)
        {
            ui->runnable->setChecked(true);
        }
    }
}

void atrdialog::setStat(QString name)
{
    this->atrName = name;
}

void atrdialog::on_buttonBox_clicked(QAbstractButton *button)
{
    this->close();
}
