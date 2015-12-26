#include "inputdialog.h"
#include "ui_inputdialog.h"
#include <QMessageBox>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

inputDialog::inputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::inputDialog)
{
    ui->setupUi(this);
}

inputDialog::~inputDialog()
{
    delete ui;
}

void inputDialog::setFolderName(char *fname)
{
    strcpy(folderName,fname);
    mode = 0;
}

void inputDialog::setFileName(char *name)
{
    strcpy(fileName,name);
    mode = 1;
}
void inputDialog::setOldName(QString oname)
{
    this->oldName = oname;
    mode = 2;
}

void inputDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if(ui->buttonBox->button(QDialogButtonBox::Ok)  == (QPushButton *)button)
    {
        QString ftext = ui->lineEdit->text();
        char * name = ftext.toLocal8Bit().data();
        if(mode == 0)
        {
            strcat(folderName,"/");
            strcat(folderName,name);
           if(mkdir(folderName,0777) < 0)
           {
               QMessageBox::information(this,"Error","Can not creat the folder. ",QMessageBox::Yes,QMessageBox::Yes);
           }
           else
           {
               QMessageBox::information(this,"Message","The folder has been created.",QMessageBox::Yes,QMessageBox::Yes);
           }
        }
        else if(mode == 1)
        {
            strcat(fileName,"/");
            strcat(fileName,name);
            if(creat(fileName,10705) < 0)
             {
                    QMessageBox::information(this,"Error","Can not creat the file. ",QMessageBox::Yes,QMessageBox::Yes);
             }
            else
            {
                QMessageBox::information(this,"Message","The file has been created.",QMessageBox::Yes,QMessageBox::Yes);
            }
        }
        else if(mode == 2)
        {
            QString nName = "";
            QStringList tmpname = this->oldName.split("/");
            int c = tmpname.count();
            for(int i = 1; i < c-1; i++)
            {
                nName = nName +"/"+ tmpname.at(i);
            }
            nName = nName + "/" + ui->lineEdit->text();
            char * cOldName = this->oldName.toLocal8Bit().data();
            char * cNewName = nName.toLocal8Bit().data();
            if(rename(cOldName,cNewName) < 0)
            {
               //QMessageBox::information(this,"Error",this->oldName+" can not be renamed to "+nName,QMessageBox::Yes,QMessageBox::Yes);
               QMessageBox::information(this,"Error",strerror(errno),QMessageBox::Yes,QMessageBox::Yes);
            }
            else
            {
                QMessageBox::information(this,"Message","The file or filoder has been renamed.",QMessageBox::Yes,QMessageBox::Yes);
            }
        }
        this->close();
    }
    else
    {
       this->close();
    }
}
