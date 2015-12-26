#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "editwindow.h"
#include <QGridLayout>
#include <QLabel>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <QMessageBox>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "CFileNode.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    about.setWindowTitle("About CYPHER");
    edit.setWindowTitle("Edit Text");
    send.setWindowTitle("Send");
    ui->leftWidget->hide();
    ui->scrollArea->hide();
    ui->outPutListWidget->hide();
    ui->scrollArea_2->show();
    ui->outPutListWithoutSearch->show();
    allFiles = makeAllFileDir("/home");
    QObject::connect(this,SIGNAL(SignalsetFolderName(char*)),&Input,SLOT(setFolderName(char*)));
    QObject::connect(this,SIGNAL(SignalsetFileName(char*)),&Input,SLOT(setFileName(char*)));
    QObject::connect(this,SIGNAL(SignalsetOldName(QString)),&Input,SLOT(setOldName(QString)));
    QObject::connect(this,SIGNAL(SignalsetStat(QString)),&atrribute,SLOT(setStat(QString)));
    QObject::connect(this,SIGNAL(SignalsetStat(QString)),&send,SLOT(setStat(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}


vector<CFileNode*> MainWindow::makeCurrentFileDir(const char *dir)
{
    vector<CFileNode*> vReturn;
    vector<string> vCurrentPath;
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    if((dp = opendir(dir)) == NULL)
    {
        vReturn.clear();
        return vReturn;
    }

    chdir(dir);
    string strPath = dir;
    strPath = strPath + "/";
    vCurrentPath.push_back(strPath);

    while((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name,&statbuf);

        //hiden files & folder will not display
        if (!strncasecmp(entry->d_name, ".", 1)) {
            continue;
        }

        if(S_ISDIR(statbuf.st_mode))  //Is folder
        {


            if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0)
            {
                continue;
            }

            CFileNode* node = new CFileNode();
            node->setNodeName(entry->d_name);
            string strPath;
            for (int i = 0; i < vCurrentPath.size(); ++i)
            {
                strPath += vCurrentPath[i];
            }
            node->setNodePath(strPath);
            node->setNodeType(2);
            vReturn.push_back(node);
        }
        else    //Is file
        {
            CFileNode* node = new CFileNode();
            node->setNodeName(entry->d_name);
            string strPath;
            for (int i = 0; i < vCurrentPath.size(); ++i)
            {
                strPath += vCurrentPath[i];
            }
            node->setNodePath(strPath);
            node->setNodeType(1);
            vReturn.push_back(node);
        }
    }

    vCurrentPath.pop_back();
    chdir("..");
    closedir(dp);

    return vReturn;
}


vector<CFileNode*> MainWindow::makeAllFileDir(const char *dir)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    if((dp = opendir(dir)) == NULL)
    {
        result.clear();
        return result;
    }

    chdir(dir);
    string strPath = dir;
    strPath = strPath + "/";
    currentPath.push_back(strPath);

    while((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name,&statbuf);

        //hiden files & folder will not display
        if (!strncasecmp(entry->d_name, ".", 1)) {
            continue;
        }

        if(S_ISDIR(statbuf.st_mode))  //Is folder
        {
            if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0)
            {
                continue;
            }

            CFileNode* node = new CFileNode();
            node->setNodeName(entry->d_name);
            string strPath;
            for (int i = 0; i < currentPath.size(); ++i)
            {
                strPath += currentPath[i];
            }
            node->setNodePath(strPath);
            node->setNodeType(2);
            result.push_back(node);

            makeAllFileDir(entry->d_name);


        }
        else    //Is file
        {
            CFileNode* node = new CFileNode();
            node->setNodeName(entry->d_name);
            string strPath;
            for (int i = 0; i < currentPath.size(); ++i)
            {
                strPath += currentPath[i];
            }
            node->setNodePath(strPath);
            node->setNodeType(1);
            result.push_back(node);
        }
    }

    currentPath.pop_back();
    chdir("..");
    closedir(dp);

    return result;
}



void MainWindow::on_pushButton_2_clicked()//cancel the search
{
    ui->searchWidget->hide();
    ui->curLabel->show();
    ui->lineEdit_2->hide();
    ui->lineEdit_3->show();
    ui->scrollArea->hide();
    ui->outPutListWidget->hide();
    ui->scrollArea_2->show();
    ui->outPutListWithoutSearch->show();
    ui->GOTO->show();
}


/*void MainWindow::on_pushButton_clicked()
{
   string outPut ="";
   string searchWord = ui->lineEdit_2->text().toStdString();

   if(searchWord == "")
   {
       ui->outPutListWidget->clear();
       /*for(int i = 0; i < result.size(); ++i)
       {
            outPut = result[i]->getNodeName()+"\t\t"+result[i]->getNodePath();
            ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
       }
       outPut = "Please input a keyword :)";
       ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
   }
   else
   {
       ui->outPutListWidget->clear();

       for(int i = 0; i < result.size(); ++i)
       {
           if(result[i]->containKeyWord(searchWord))
           {
                outPut = result[i]->getNodeName()+"\t"+result[i]->getNodePath();
                ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
           }
       }
   }

   if(ui->outPutListWidget->count() == 0)
   {
       outPut = "No result :(";
       ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
   }
   //ui->lOutput->setText(QString(QString::fromLocal8Bit(outPut.c_str())));
}*/


void MainWindow::on_lineEdit_2_textChanged(const QString &arg1)//open folder textbox
{
    string outPut ="";
    string searchWord = ui->lineEdit_2->text().toStdString();

    if(searchWord == "")
    {
        ui->outPutListWidget->clear();
        outPut = "Please input a keyword :)";
        ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }
    else
    {
        ui->outPutListWidget->clear();

        vector<CFileNode*> vTemp;

        for(int i = 0; i < result.size(); ++i)
        {
            if(result[i]->containKeyWord(searchWord))
            {
                vTemp.push_back(result[i]);

            }
        }

        int count = 0;

        for(int i = 0; i < vTemp.size(); ++i){

            if(vTemp[i]->iLevel == 1)
            {
                outPut = vTemp[i]->getNodeName()+"\t"+vTemp[i]->getNodePath();
                ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
                count ++;
            }

        }

        if(count){
            outPut = "";
            ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
        }

        count = 0;

        for(int i = 0; i < vTemp.size(); ++i){

            if(vTemp[i]->iLevel == 2)
            {
                outPut = vTemp[i]->getNodeName()+"\t"+vTemp[i]->getNodePath();
                ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
                count++;
            }

        }

        if(count){
            outPut = "";
            ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
        }

        for(int i = 0; i < vTemp.size(); ++i){

            if(vTemp[i]->iLevel == 3)
            {
                outPut = vTemp[i]->getNodeName()+"\t"+vTemp[i]->getNodePath();
                ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
            }

        }
    }

    if(ui->outPutListWidget->count() == 0)
    {
        outPut = "No result :(";
        ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }
}

void MainWindow::on_GOTO_clicked()//open folder
{
    string outPut ="";
    string sPath = ui->lineEdit_3->text().toStdString();

    if(sPath == "")
    {
        ui->outPutListWithoutSearch->clear();
        outPut = "Please input a keyword :)";
        ui->outPutListWithoutSearch->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }
    else
    {
        ui->outPutListWithoutSearch->clear();

        /*const char* strToChar = sPath.c_str();

        char* cPath = (char*)malloc(sizeof(char)*sPath.length());

        memset(cPath, ' ',sPath.length());

        for(int i = 0; i < sPath.length(); ++i)
        {
            cPath[i] = strToChar[i];
        }*/

        //ui->lineEdit_2->setText(QString(QString::fromLocal8Bit(sPath.c_str())));

        vector<CFileNode*> currentFile = makeCurrentFileDir(sPath.c_str());

        if(currentFile.size() == 0)
        {
            ui->outPutListWithoutSearch->clear();
            outPut = "Incorrect path, Input again :(";
            ui->outPutListWithoutSearch->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
        }
        else
        {
            ui->outPutListWithoutSearch->clear();
            for(int i = 0; i < currentFile.size(); ++i)
            {

                outPut = currentFile[i]->getNodeName()+"\t"+currentFile[i]->getNodePath();
                ui->outPutListWithoutSearch->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));

            }
        }
    }

    if(ui->outPutListWidget->count() == 0)
    {
        outPut = "No result :(";
        ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }
}

void MainWindow::on_outPutListWidget_itemDoubleClicked(QListWidgetItem *item)//show files and folders
{

    string text = item->text().toStdString();
    if(text.find('\t') == text.npos)
    {
        return;
    }
    else
    {
        int iConsor = text.find('\t');
        string sName = text.substr(0,iConsor);
        string sPath = text.substr(iConsor+1, text.length());

        for(int i = 0; i < result.size(); ++i)
        {
            if(result[i]->getNodeName() == sName && result[i]->getNodePath() == sPath)
            {
                result[i]->openCurrent();
            }
        }

        //string command = "nautilus " + sPath + sName;

        //ui->lineEdit_2->setText(QString(QString::fromLocal8Bit(text.c_str())));

        //system(command.c_str());
    }

    //result[iNum]-> openCurrent();
}



void MainWindow::on_action_F_triggered()//NEW FOLDER
{
       QString dir_name = ui->lineEdit_3->text();
       char * cdir_name = dir_name.toLocal8Bit().data();
       emit SignalsetFolderName(cdir_name);
       Input.show();
}

void MainWindow::on_action_D_triggered()//NEW FILE
{
       QString file_name = ui->lineEdit_3->text();
       char * cfile_name = file_name.toLocal8Bit().data();
       emit SignalsetFileName(cfile_name);
       Input.show();
}

void MainWindow::on_action_C_3_triggered()//COPY
{
    QListWidgetItem *item = ui->outPutListWithoutSearch->currentItem();
    if(item == NULL)
    {
         QMessageBox::information(this,"Error","Please choose the item you want to copy. ",QMessageBox::Yes,QMessageBox::Yes);
         return;
    }
    QString source = item->text();
    QStringList tmp = source.split("\t");
    QString source1 = "";
    source1 = tmp.at(1)+tmp.at(0);
   const char * csource = source1.toLocal8Bit().data();
   strcpy(this->sString,csource);
   this->mode = 0;
    QMessageBox::information(this,"Message",this->sString,QMessageBox::Yes,QMessageBox::Yes);
}

void MainWindow::Copy(char * spathname, char * tpathname)
{
       int sfd,tfd;
       struct stat s;
       char c;
       sfd=open(spathname,O_RDONLY);
       tfd=open(tpathname,O_RDWR|O_CREAT);
       while(read(sfd,&c,1)>0)
            write(tfd,&c,1);
       fstat(sfd,&s);
       chown(tpathname,s.st_uid,s.st_gid);
       chmod(tpathname,s.st_mode);
       ::close(sfd);
       ::close(tfd);
}

void MainWindow::d_copy(char *spathname,char *tpathname)
{
   struct stat s,temp_s;
   struct dirent *s_p;
   DIR *dirs,*dirt;
   stat(spathname,&s);
   mkdir(tpathname,s.st_mode);
   chown(tpathname,s.st_uid,s.st_gid);
   dirt=opendir(tpathname);
   dirs=opendir(spathname);
   strcpy(this->temp_paths,spathname);
   strcpy(this->temp_patht,tpathname);
   while((s_p=readdir(dirs))!=NULL)
   {
      if(strcmp(s_p->d_name,".")!=0&&strcmp(s_p->d_name,"..")!=0)
      {
          strcat(temp_paths,"/");
          strcat(temp_paths,s_p->d_name);
          strcat(temp_patht,"/");
          strcat(temp_patht,s_p->d_name);
          lstat(temp_paths,&s);
          temp_s.st_mode=s.st_mode;
          if(S_ISLNK(temp_s.st_mode))
          {
          }
          else if(S_ISREG(temp_s.st_mode))
          {
              Copy(temp_paths,temp_patht);
              strcpy(temp_paths,spathname);
              strcpy(temp_patht,tpathname);
          }
          else if(S_ISDIR(temp_s.st_mode))
          {
              d_copy(temp_paths,temp_patht);
              strcpy(temp_paths,spathname);
              strcpy(temp_patht,tpathname);
          }
      }
   }
}

void MainWindow::on_action_P_2_triggered()//PASTE
{
    if(mode == -1)
    {
        return;
    }
    QString target = ui->lineEdit_3->text();   
    char * ctarget = target.toLocal8Bit().data();
    DIR *dir_s,*dir_t;
    struct stat buf;
    if(lstat(this->sString,&buf)<0)
    {
        QMessageBox::information(this,"Error","An error of the source occurs . ",QMessageBox::Yes,QMessageBox::Yes);
        return;
    }
    if (S_ISREG(buf.st_mode))
    {
        dir_t = opendir(ctarget);
        if(dir_t == NULL)
        {
            mkdir(ctarget,0777);
        }
        QString tmp = QString::fromLocal8Bit(this->sString);
        QStringList alltmp = tmp.split("/");
        QString fileName = alltmp.at(alltmp.count( )-1);
        QString target1 = target + "/"+fileName;
        char * ctarget1 = target1.toLocal8Bit().data();
        Copy(this->sString, ctarget1);
        if(this->mode == 1)
        {
            mydelete(this->sString);
        }
        mode = -1;
        ui->lineEdit_3->setText("");
        ui->lineEdit_3->setText(target);
        return;
    }
    if(S_ISDIR(buf.st_mode))
    {
        struct dirent *sp;
        char spath[1000],tpath[1000],temp_spath[1000],temp_tpath[1000];
        struct stat sbuf,temp_sbuf;
        char sdirect[1000],tdirect[1000];
        strcpy(sdirect,this->sString);
        dir_s=opendir(sdirect);
        if(stat(sdirect,&sbuf)!=0)
        {
              QMessageBox::information(this,"Error","Get status error . ",QMessageBox::Yes,QMessageBox::Yes);
              return;
        }
        QString src_name = QString::fromLocal8Bit(this->sString);
        QStringList lsrc_name = src_name.split("/");
        QString src_fname = lsrc_name.at(lsrc_name.count()-1);
        char * src_cfname = src_fname.toLocal8Bit().data();
        strcpy(tdirect,ctarget);
        strcat(tdirect,"/");
        strcat(tdirect,src_cfname);
        dir_t=opendir(tdirect);
        if(dir_t==NULL)
        {
           mkdir(tdirect,sbuf.st_mode);
           chown(tdirect,sbuf.st_uid,sbuf.st_gid);
           dir_t=opendir(tdirect);
        }
        else
        {
           chmod(tdirect,sbuf.st_mode);
           chown(tdirect,sbuf.st_uid,sbuf.st_gid);
        }
        strcpy(spath,sdirect);
        strcpy(tpath,tdirect);
        strcpy(temp_spath,sdirect);
        strcpy(temp_tpath,tdirect);
        while((sp=readdir(dir_s))!=NULL)
        {
           if(strcmp(sp->d_name,".")!=0&&strcmp(sp->d_name,"..")!=0)
           {
               strcat(temp_spath,"/");
               strcat(temp_spath,sp->d_name);
               strcat(temp_tpath,"/");
               strcat(temp_tpath,sp->d_name);
               lstat(temp_spath,&sbuf);
               temp_sbuf.st_mode=sbuf.st_mode;
               if(S_ISLNK(temp_sbuf.st_mode))
               {
               }
               else if((S_IFMT&temp_sbuf.st_mode)==S_IFREG)
               {
                   Copy(temp_spath,temp_tpath);
                   strcpy(temp_tpath,tpath);
                   strcpy(temp_spath,spath);
               }
               else if((S_IFMT&temp_sbuf.st_mode)==S_IFDIR)
               {
                   d_copy(temp_spath,temp_tpath);
                   strcpy(temp_tpath,tpath);
                   strcpy(temp_spath,spath);
               }
           }
        }
        closedir(dir_t);
        closedir(dir_s);
        if(this->mode == 1)
        {
            mydelete(this->sString);
        }
        mode = -1;
        strcpy(temp_paths,"");
        strcpy(temp_patht,"");
        ui->lineEdit_3->setText("");
        ui->lineEdit_3->setText(target);
        return;
    }
}

void MainWindow::on_action_D_2_triggered()//DELETE
{
    QListWidgetItem *item = ui->outPutListWithoutSearch->currentItem();
    QString cpath = ui->lineEdit_3->text();
    if(item == NULL)
    {
         QMessageBox::information(this,"Error","Please choose the item you want to copy. ",QMessageBox::Yes,QMessageBox::Yes);
         return;
    }
    QString del_name = item->text();
    QStringList tmp = del_name.split("\t");
    QString del_name1 = "";
    del_name1 = tmp.at(1)+tmp.at(0);
    const char * delName = del_name1.toLocal8Bit().data();
    mydelete(delName);
    ui->lineEdit_3->setText("");
    ui->lineEdit_3->setText(cpath);
}

void MainWindow::mydelete(const char * del_name)
{
    struct stat buf;
    if(lstat(del_name,&buf) != 0)
    {
        QMessageBox::information(this,"Error","Get status error . ",QMessageBox::Yes,QMessageBox::Yes);
        return;
    }
    if (S_ISREG(buf.st_mode))
    {
        if(::remove(del_name) < 0)
        {
            QMessageBox::information(this,"Error","Can not remove the item . ",QMessageBox::Yes,QMessageBox::Yes);
        }
        return;
    }
    if(S_ISDIR(buf.st_mode))
    {
       char mytemp[1000];
        struct dirent *sp;
        DIR * dir;
        dir = opendir(del_name);
        while((sp=readdir(dir))!=NULL)
        {
            strcpy(mytemp,del_name);
            if(strcmp(sp->d_name,".")!=0&&strcmp(sp->d_name,"..")!=0)
            {
                strcat(mytemp,"/");
                strcat(mytemp,sp->d_name);
                mydelete(mytemp);
            }
        }
        if(::remove(del_name) < 0)
        {
            QMessageBox::information(this,"Error","Can not remove the item . ",QMessageBox::Yes,QMessageBox::Yes);
        }
        return;
    }
}

void MainWindow::on_action_C_2_triggered()//CUT
{
    QListWidgetItem *item = ui->outPutListWithoutSearch->currentItem();
    if(item == NULL)
    {
         QMessageBox::information(this,"Error","Please choose the item you want to cut. ",QMessageBox::Yes,QMessageBox::Yes);
         return;
    }
    QString source = item->text();
    QStringList tmp = source.split("\t");
    QString source1 = "";
    source1 = tmp.at(1)+tmp.at(0);
   const char * csource = source1.toLocal8Bit().data();
   strcpy(this->sString,csource);
   this->mode = 1;
    QMessageBox::information(this,"Message",this->sString,QMessageBox::Yes,QMessageBox::Yes);
}

void MainWindow::on_action_R_2_triggered()//RENAME
{
    QListWidgetItem *item = ui->outPutListWithoutSearch->currentItem();
    if(item == NULL)
    {
         QMessageBox::information(this,"Error","Please choose the item you want to rename. ",QMessageBox::Yes,QMessageBox::Yes);
         return;
    }
    QString old_name = item->text();
    QStringList tmp = old_name.split("\t");
    QString old_name1 = tmp.at(1)+tmp.at(0);
    emit SignalsetOldName(old_name1);
    Input.show();
}

void MainWindow::on_action_P_triggered()//ATTRIBUTES
{
    QListWidgetItem *item = ui->outPutListWithoutSearch->currentItem();
    if(item == NULL)
    {
         QMessageBox::information(this,"Error","Please choose the item. ",QMessageBox::Yes,QMessageBox::Yes);
         return;
    }
    QString fname = item->text();
    QStringList tmp = fname.split("\t");
    QString fname1 = tmp.at(1)+tmp.at(0);
    emit SignalsetStat(fname1);
    atrribute.showMsg();
    atrribute.show();
}

void MainWindow::on_refresh_triggered()//REFRESH
{
    QString qs = ui->lineEdit_3->text();
    ui->lineEdit_3->setText("");
    ui->lineEdit_3->setText(qs);
}

void MainWindow::on_action_S_triggered()//SEARCH
{
    ui->searchWidget->show();
    ui->curLabel->hide();
    ui->lineEdit_2->show();
    ui->lineEdit_3->hide();
    ui->outPutListWidget->show();
    ui->scrollArea->show();
    ui->scrollArea_2->hide();
    ui->outPutListWithoutSearch->hide();
    ui->GOTO->hide();
}

void MainWindow::on_lineEdit_3_textChanged(const QString &arg1)//search edit textbox
{
    string outPut ="";
    string sPath = ui->lineEdit_3->text().toStdString();

    if(sPath == "")
    {
        ui->outPutListWithoutSearch->clear();
        outPut = "Please input a keyword :)";
        ui->outPutListWithoutSearch->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }
    else
    {
        ui->outPutListWithoutSearch->clear();

        vector<CFileNode*> currentFile = makeCurrentFileDir(sPath.c_str());

        if(currentFile.size() == 0)
        {
            ui->outPutListWithoutSearch->clear();
            outPut = "Empty :(";
            ui->outPutListWithoutSearch->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
        }
        else
        {
            int count = 0;
            ui->outPutListWithoutSearch->clear();
            for(int i = 0; i < currentFile.size(); ++i)
            {
                if(currentFile[i]->getNodeType() == 1)
                {
                    outPut = currentFile[i]->getNodeName()+"\t"+currentFile[i]->getNodePath();
                    ui->outPutListWithoutSearch->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
                    count ++;
                }

            }

            if(count)
            {
                outPut = "";
                ui->outPutListWithoutSearch->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
            }

            for(int i = 0; i < currentFile.size(); ++i)
            {
                if(currentFile[i]->getNodeType() == 2)
                {
                    outPut = currentFile[i]->getNodeName()+"\t"+currentFile[i]->getNodePath();
                    ui->outPutListWithoutSearch->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
                }
            }
        }
    }
}

void MainWindow::on_outPutListWithoutSearch_itemDoubleClicked(QListWidgetItem *item)//show search results
{
    string text = item->text().toStdString();
    if(text.find('\t') == text.npos)
    {
        return;
    }
    else
    {
        int iConsor = text.find('\t');
        string sName = text.substr(0,iConsor);
        string sPath = text.substr(iConsor+1, text.length());

        for(int i = 0; i < result.size(); ++i)
        {
            if(result[i]->getNodeName() == sName && result[i]->getNodePath() == sPath)
            {
                if(result[i]->
                        getNodeType() == 2)
                {
                  string text = sPath + sName;
                  ui->lineEdit_3->setText(QString(QString::fromLocal8Bit(text.c_str())));
                }
                else
                {
                    result[i]->openCurrent();
                }

            }
        }

        //string command = "nautilus " + sPath + sName;

        //ui->lineEdit_2->setText(QString(QString::fromLocal8Bit(text.c_str())));

        //system(command.c_str());
    }
}

void MainWindow::on_action_A_triggered()
{
    about.show();
}

void MainWindow::on_actionEdit_triggered()
{
    edit.show();
}

void MainWindow::on_Send_clicked()
{
    QListWidgetItem *item = ui->outPutListWithoutSearch->currentItem();
    if(item == NULL)
    {
         QMessageBox::information(this,"Error","Please choose the item. ",QMessageBox::Yes,QMessageBox::Yes);
         return;
    }
    QString fname = item->text();
    QStringList tmp = fname.split("\t");
    QString fname1 = tmp.at(1)+tmp.at(0);
    emit SignalsetStat(fname1);
    send.showMsg();
    send.show();
}

void MainWindow::on_showSideBar_triggered()//show side bar
{
    if(!ui->showSideBar->isChecked())
    {
        ui->leftWidget->close();
    }else
    {
        ui->leftWidget->show();
    }
}

void MainWindow::on_pushButton_3_clicked()//side bar:home
{
    string outPut ="";
    string sPath = "/home";

    ui->outPutListWithoutSearch->clear();

    //ui->lineEdit_2->setText(QString(QString::fromLocal8Bit(sPath.c_str())));

    vector<CFileNode*> currentFile = makeCurrentFileDir(sPath.c_str());

    for(int i = 0; i < currentFile.size(); ++i)
    {

        outPut = currentFile[i]->getNodeName()+"\t"+currentFile[i]->getNodePath();
        ui->outPutListWithoutSearch->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }

    ui->lineEdit_3->setText(QString(QString::fromLocal8Bit(sPath.c_str())));

    if(ui->outPutListWidget->count() == 0)
    {
        outPut = "No result :(";
        ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }
}

void MainWindow::on_pushButton_4_clicked()//side bar:desktop
{
    string outPut ="";
    string sHome = "/home";
    string sPath = "";

    ui->outPutListWithoutSearch->clear();

    //ui->lineEdit_2->setText(QString(QString::fromLocal8Bit(sPath.c_str())));

    vector<CFileNode*> homeFile = makeCurrentFileDir(sHome.c_str());

    if(homeFile.size() > 0)
    {
        sPath = homeFile[0]->getNodePath() + homeFile[0]->getNodeName() + "/Desktop";
    }

    vector<CFileNode*> currentFile = makeCurrentFileDir(sPath.c_str());

    if(currentFile.size() == 0)
    {
        sPath = homeFile[0]->getNodePath() + homeFile[0]->getNodeName() + "/桌面";
    }

    currentFile.clear();

    currentFile = makeCurrentFileDir(sPath.c_str());


    for(int i = 0; i < currentFile.size(); ++i)
    {

        outPut = currentFile[i]->getNodeName()+"\t"+currentFile[i]->getNodePath();
        ui->outPutListWithoutSearch->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }

    ui->lineEdit_3->setText(QString(QString::fromLocal8Bit(sPath.c_str())));

    if(ui->outPutListWidget->count() == 0)
    {
        outPut = "No result :(";
        ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }
}

void MainWindow::on_pushButton_5_clicked()//side bar:downloads
{
    string outPut ="";
    string sHome = "/home";
    string sPath = "";

    ui->outPutListWithoutSearch->clear();

    //ui->lineEdit_2->setText(QString(QString::fromLocal8Bit(sPath.c_str())));

    vector<CFileNode*> homeFile = makeCurrentFileDir(sHome.c_str());

    if(homeFile.size() > 0)
    {
        sPath = homeFile[0]->getNodePath() + homeFile[0]->getNodeName() + "/Downloads";
    }

    vector<CFileNode*> currentFile = makeCurrentFileDir(sPath.c_str());

    if(currentFile.size() == 0)
    {
        sPath = homeFile[0]->getNodePath() + homeFile[0]->getNodeName() + "/下载";
    }

    currentFile.clear();

    currentFile = makeCurrentFileDir(sPath.c_str());

    for(int i = 0; i < currentFile.size(); ++i)
    {

        outPut = currentFile[i]->getNodeName()+"\t"+currentFile[i]->getNodePath();
        ui->outPutListWithoutSearch->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }

    ui->lineEdit_3->setText(QString(QString::fromLocal8Bit(sPath.c_str())));

    if(ui->outPutListWidget->count() == 0)
    {
        outPut = "No result :(";
        ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }
}

void MainWindow::on_pushButton_6_clicked()//side bar:documents
{
    string outPut ="";
    string sHome = "/home";
    string sPath = "";

    ui->outPutListWithoutSearch->clear();

    //ui->lineEdit_2->setText(QString(QString::fromLocal8Bit(sPath.c_str())));

    vector<CFileNode*> homeFile = makeCurrentFileDir(sHome.c_str());

    if(homeFile.size() > 0)
    {
        sPath = homeFile[0]->getNodePath() + homeFile[0]->getNodeName() + "/Documents";
    }

    vector<CFileNode*> currentFile = makeCurrentFileDir(sPath.c_str());

    if(currentFile.size() == 0)
    {
        sPath = homeFile[0]->getNodePath() + homeFile[0]->getNodeName() + "/文档";
    }

    currentFile.clear();

    currentFile = makeCurrentFileDir(sPath.c_str());

    for(int i = 0; i < currentFile.size(); ++i)
    {

        outPut = currentFile[i]->getNodeName()+"\t"+currentFile[i]->getNodePath();
        ui->outPutListWithoutSearch->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }

    ui->lineEdit_3->setText(QString(QString::fromLocal8Bit(sPath.c_str())));

    if(ui->outPutListWidget->count() == 0)
    {
        outPut = "No result :(";
        ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }
}

void MainWindow::on_pushButton_7_clicked()//side bar:trash
{
    string outPut ="";
    string sHome = "/home";
    string sPath = "";

    ui->outPutListWithoutSearch->clear();

    //ui->lineEdit_2->setText(QString(QString::fromLocal8Bit(sPath.c_str())));

    vector<CFileNode*> homeFile = makeCurrentFileDir(sHome.c_str());

    if(homeFile.size() > 0)
    {
        sPath = homeFile[0]->getNodePath() + homeFile[0]->getNodeName() + "/.local/share/Trash/files";
    }

    vector<CFileNode*> currentFile = makeCurrentFileDir(sPath.c_str());

    for(int i = 0; i < currentFile.size(); ++i)
    {

        outPut = currentFile[i]->getNodeName()+"\t"+currentFile[i]->getNodePath();
        ui->outPutListWithoutSearch->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }

    ui->lineEdit_3->setText(QString(QString::fromLocal8Bit(sPath.c_str())));

    if(ui->outPutListWidget->count() == 0)
    {
        outPut = "No result :(";
        ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }
}

void MainWindow::on_pushButton_8_clicked()//side bar:computer
{
    string outPut ="";
    string sPath = "/";

    ui->outPutListWithoutSearch->clear();

    //ui->lineEdit_2->setText(QString(QString::fromLocal8Bit(sPath.c_str())));

    vector<CFileNode*> currentFile = makeCurrentFileDir(sPath.c_str());

    for(int i = 0; i < currentFile.size(); ++i)
    {

        outPut = currentFile[i]->getNodeName()+"\t"+currentFile[i]->getNodePath();
        ui->outPutListWithoutSearch->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }

    //ui->lineEdit_3->setText(QString(QString::fromLocal8Bit(sPath.c_str())));

    if(ui->outPutListWidget->count() == 0)
    {
        outPut = "No result :(";
        ui->outPutListWidget->addItem((QString(QString::fromLocal8Bit(outPut.c_str()))));
    }
}
