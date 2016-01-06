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

#define LIBSSH_STATIC 1
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <iostream>
#include <errno.h>

using namespace std;

int show_remote_processes(ssh_session session);
sftp_session init_sftp(ssh_session session);
int sftp_list_dir(ssh_session session, sftp_session sftp, char * path);
int sftp_local_remote(ssh_session session,sftp_session sftp, string remote_filename,string filename);
int sftp_local_remote(ssh_session session, string remote_filename,string filename);
int sftp_list_contents(ssh_session session,char * path);
int mkdir_remote(ssh_session session);
int sftp_read_sync(ssh_session session, sftp_session sftp, string remote_file, string local_file);
int sftp_read_sync(ssh_session session,  string remote_file,string local_file);
int connect_smtp_server(const char* server_addr);
char* base64_encode(const char* src, char* des);
int communicate_server(int sockfd, const char* message);

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
    QObject::connect(&login_dialog,SIGNAL(SignalLogin(char*,char* ,char * , int )),this,SLOT(Login(char* ,char* ,char * , int )));
    QObject::connect(&send,SIGNAL(SignalSend(char *, char*,char*,char*,bool)),this,SLOT(Send(char*,char*,char*,char*,bool)));
    QObject::connect(&pull_doc,SIGNAL(SignalPull(char*,char*)),this,SLOT(get_file(char *,char*)));
    login=0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

string char_star_to_string(const char * target)
{
    string s(target);
    return s;
}

int connect_smtp_server(const char* server_addr)
{
        int sockfd = 0;
        struct hostent* ht;
        struct sockaddr_in si;

        ht = gethostbyname(server_addr);
        if (NULL == ht) {
                return -1;
        }

        si.sin_family = ht->h_addrtype;
        si.sin_port   = htons(25);
        si.sin_addr.s_addr = *(in_addr_t*)ht->h_addr_list[0];
        bzero(&si.sin_zero, 8);
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
                return -1;
        }
        if (connect(sockfd, (struct sockaddr*)&si, sizeof(si)) < 0) {
                return -1;
        }
        return sockfd;
}


char* base64_encode(const char* src, char* des)
{
        static const char cb64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        unsigned char in[3], out[4];
        int i, len, k = 0;

        while (*src) {
                for (len = i = 0; i < 3; i++) {
                        *src ? (in[i] = *src++, len++) : (in[i] = 0);
                }
                if (len) {
                        out[0] = cb64[ in[0] >> 2 ];
                        out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
                        out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
                        out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
                        for (i = 0; i < 4; i++) {
                                des[k++] = out[i];
                        }
                }
        }
        des[k] = '\0';
        return des;
}

int communicate_server(int sockfd, const char* message)
{
        int  errcode;
        char buf[SIZE512];
        snprintf(buf, SIZE512, message);
        errcode = send(sockfd, buf, strlen(buf), 0);

        printf("send : %s", buf);

        if (strlen(buf) != errcode) {
                return ERROR_SEND_MESSAGE;
        }
        errcode = recv(sockfd, buf, sizeof(buf), 0);
        if (errcode == SOCKET_ERROR || errcode == 0) {
                return ERROR_RECV_MESSAGE;
        }
        buf[errcode] = '\0';

        printf("recv : %s", buf);

        return 0;
}


int send_mail(const char* s_server, const char* username, const char* password, content_t* ct, char* receiver)
{
    int  sockfd, errcode;
    char buf[SIZE512], *domain;
    assert(s_server && username && password && receiver);

    /* connect and receive confirm message */
    sockfd = connect_smtp_server(s_server);
    if (sockfd == -1) {
        errcode = ERROR_CONNECT_SMTP;
        return errcode;
    }
    errcode = recv(sockfd, buf, sizeof(buf), 0);
    if (errcode == SOCKET_ERROR || errcode == 0) {
        return ERROR_RECV_MESSAGE;
    }
    buf[errcode] = '\0';
    puts(buf);

    /* send hello message */
    errcode = communicate_server(sockfd, "HELO SMTP\r\n");
    if (errcode != 0) {
        return errcode;
    }

    /* send login message */
    errcode = communicate_server(sockfd, "AUTH LOGIN\r\n");
    if (errcode != 0) {
        return errcode;
    }

    /* send username */
    strcat(base64_encode(username, buf), "\r\n");
    errcode = communicate_server(sockfd, buf);
    if (errcode != 0) {
        return errcode;
    }

    /* send password */
    strcat(base64_encode(password, buf), "\r\n");
    errcode = communicate_server(sockfd, buf);
    if (errcode != 0) {
        return errcode;
    }

    /* send mail from message */
    domain =const_cast<char*>(strchr(s_server, '.'));
    if (domain == NULL) {
        return ERROR_ILLEGAL_ADDRESS;
    }
    snprintf(buf, SIZE512, "MAIL FROM: <%s@%s>\r\n", username, domain + 1);
    errcode = communicate_server(sockfd, buf);
    if (errcode != 0) {
        return errcode;
    }

    /* get the receiver */
    while (*receiver) {
        snprintf(buf, SIZE512, "RCPT TO: <%s>\r\n", receiver);
        errcode = communicate_server(sockfd, buf);
        if (errcode != 0) {
            return errcode;
        }
        receiver += strlen(receiver) + 1;
    }

    /* ready to send data */
    errcode = communicate_server(sockfd, "DATA\r\n");
    if (errcode != 0) {
        return errcode;
    }

    /* start to send data */
    int   len = (SIZE64 * 4 * + strlen(ct->text)) * sizeof(char);
    char* ptmpbuf = (char*) malloc(len);
    if (ptmpbuf == NULL) {
        return ERROR_ALLOC_MEMORY;
    }
    snprintf(ptmpbuf, len, "From: \"%s\"<%s>\r\nTo: \"%s\"<%s>\r\nSubject: %s\r\n\r\n%s\r\n.\r\n",
            ct->from_name, ct->from_addr, ct->to_name, ct->to_addr, ct->subject, ct->text);
    errcode = communicate_server(sockfd, ptmpbuf);
    if (errcode != 0) {
        return errcode;
    }

    /* quit */
    errcode = communicate_server(sockfd, "QUIT\r\n");
    if (errcode != 0) {
        return errcode;
    }

    close(sockfd);

    return 0;
}

sftp_session init_sftp(ssh_session session)
{
  sftp_session sftp;
  int rc;
  sftp = sftp_new(session);
  if (sftp == NULL)
  {
    fprintf(stderr, "Error allocating SFTP session: %s\n",
            ssh_get_error(session));
    return NULL;
  }
  rc = sftp_init(sftp);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Error initializing SFTP session: %s.\n",
            sftp_get_error(sftp));
    sftp_free(sftp);
    return NULL;
  }
  //sftp_free(sftp);
  return sftp;
}

int sftp_local_remote(ssh_session session,string remote_filename,string filename)
{
  sftp_session sftp=init_sftp(session);
  //int sftp_rc = init_sftp(session,sftp);
  if(sftp ==NULL){
    return SSH_FATAL;
    }
  int rc;
  rc = sftp_local_remote(session,sftp,remote_filename,filename);
  if(rc !=SSH_OK){
    sftp_free(sftp);
    return SSH_ERROR;
    }
    sftp_free(sftp);
return SSH_OK;


}

#define MAX_XFER_BUF_SIZE 16384
int sftp_local_remote(ssh_session session,sftp_session sftp, string remote_filename,string filename)
{
  int access_type = O_WRONLY | O_CREAT;
  sftp_file file;
  int nbytes, nwritten, rc;
  int fd;
  char buffer[MAX_XFER_BUF_SIZE];
  cout<<"filename"<<filename<<endl;
  fd = open(filename.c_str(),O_RDONLY |O_CREAT);
  if (fd < 0) {
      fprintf(stderr, "Can't open file for reading: %s\n",
              strerror(errno));
      return SSH_ERROR;
  }

  file = sftp_open(sftp, remote_filename.c_str(),
                   access_type, S_IRWXU);
  if (file == NULL)
  {
    fprintf(stderr, "Can't open file for writing: %s\n",
            ssh_get_error(session));
    return SSH_ERROR;
  }


  for (;;) {
      nbytes = read(fd, buffer, sizeof(buffer));
      if (nbytes == 0) {
          break; // EOF
      } else if (nbytes < 0) {
          fprintf(stderr, "Error while reading file: %s\n",
                  ssh_get_error(session));
          close(fd);
          return SSH_ERROR;
      }
      nwritten = sftp_write(file, buffer, nbytes);
      cout<<"nwritten:"<<nwritten<<endl;
      cout<<"nbytes:"<<nbytes<<endl;
      if (nwritten != nbytes) {
          fprintf(stderr, "Error writing: %s\n",
                  strerror(errno));
          sftp_close(file);
          return SSH_ERROR;
      }
  }

  rc = sftp_close(file);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Can't close the written file: %s\n",
            ssh_get_error(session));
    return rc;
  }
  return SSH_OK;
}


int sftp_read_sync(ssh_session session, sftp_session sftp, string remote_file, string local_file)
{
  int access_type;
  sftp_file file;
  char buffer[MAX_XFER_BUF_SIZE];
  int nbytes, nwritten, rc;
  int fd;
  access_type = O_RDONLY;
  file = sftp_open(sftp, remote_file.c_str(),
                   access_type, 0);
  if (file == NULL) {
      fprintf(stderr, "Can't open file for reading: %s\n",
              ssh_get_error(session));
      return SSH_ERROR;
  }
  fd = open(local_file.c_str(), O_CREAT | O_RDWR | O_TRUNC,0777);
  cout<<"local:"<<local_file<<endl;
  if (fd < 0) {
      cout<<"aaaaaaaaaaaaaaaaaaa"<<endl;
      fprintf(stderr, "Can't open file for writing: %s\n",
              strerror(errno));
      return SSH_ERROR;
  }
  for (;;) {
      nbytes = sftp_read(file, buffer, sizeof(buffer));
      if (nbytes == 0) {
          break; // EOF
      } else if (nbytes < 0) {
          fprintf(stderr, "Error while reading file: %s\n",
                  ssh_get_error(session));
          sftp_close(file);
          return SSH_ERROR;
      }
      nwritten = write(fd, buffer, nbytes);
      cout<<"nwritten:"<<nwritten<<endl;
      cout<<"nbytes:"<<nbytes<<endl;
      if (nwritten != nbytes) {
          fprintf(stderr, "Error writing: %s\n",
                  strerror(errno));
          sftp_close(file);
          return SSH_ERROR;
      }
  }
  rc = sftp_close(file);
  if (rc != SSH_OK) {
      fprintf(stderr, "Can't close the read file: %s\n",
              ssh_get_error(session));
      return rc;
  }
  return SSH_OK;
}

int sftp_read_sync(ssh_session session,  string remote_file, string local_file)
{
  sftp_session sftp=init_sftp(session);
  if(sftp ==NULL){
    return SSH_FATAL;
    }
  int rc;
  rc = sftp_read_sync(session,sftp, remote_file, local_file);
  if(rc!=SSH_OK){
    sftp_free(sftp);
    return SSH_ERROR;
}
  sftp_free(sftp);
  return SSH_OK;

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

    SignalsetOldName(old_name1);
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
    if(login!=1){
        popup.setlabel("please login the remote server first");
        popup.show();
        return;
    }
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

void MainWindow::on_actionLog_in_triggered()
{
    login_dialog.show();
}

int MainWindow::verify_knownhost(ssh_session session)
{
  int state, hlen;
  unsigned char *hash = NULL;
  char *hexa;
  char buf[10];
  state = ssh_is_server_known(session);
  hlen = ssh_get_pubkey_hash(session, &hash);
  if (hlen < 0)
    return -1;
  switch (state)
  {
    case SSH_SERVER_KNOWN_OK:
      break; /* ok */
    case SSH_SERVER_KNOWN_CHANGED:
      fprintf(stderr, "Host key for server changed: it is now:\n");
      ssh_print_hexa("Public key hash", hash, hlen);
      fprintf(stderr, "For security reasons, connection will be stopped\n");
      free(hash);
      return -1;
    case SSH_SERVER_FOUND_OTHER:
      fprintf(stderr, "The host key for this server was not found but an other"
        "type of key exists.\n");
      fprintf(stderr, "An attacker might change the default server key to"
        "confuse your client into thinking the key does not exist\n");
      free(hash);
      return -1;
    case SSH_SERVER_FILE_NOT_FOUND:
      fprintf(stderr, "Could not find known host file.\n");
      fprintf(stderr, "If you accept the host key here, the file will be"
       "automatically created.\n");
      /* fallback to SSH_SERVER_NOT_KNOWN behavior */
    case SSH_SERVER_NOT_KNOWN:
      hexa = ssh_get_hexa(hash, hlen);
      fprintf(stderr,"The server is unknown. Do you trust the host key?\n");
      fprintf(stderr, "Public key hash: %s\n", hexa);
      free(hexa);
      if (fgets(buf, sizeof(buf), stdin) == NULL)
      {
        free(hash);
        return -1;
      }
      if (strncasecmp(buf, "yes", 3) != 0)
      {
        free(hash);
        return -1;
      }
      if (ssh_write_knownhost(session) < 0)
      {
        fprintf(stderr, "Error %s\n", strerror(errno));
        free(hash);
        return -1;
      }
      break;
    case SSH_SERVER_ERROR:
      fprintf(stderr, "Error %s", ssh_get_error(session));
      free(hash);
      return -1;
  }
  free(hash);
  return 0;
}

void MainWindow::Login(char * hostname,char* username, char * password, int port)
{
    //ssh_session my_ssh_session;
    int rc;
    cout<<"hostname:"<<hostname<<endl;
    cout<<"username:"<<username<<endl;
    cout<<"password:"<<password<<endl;
    cout<<"port:"<<port<<endl;
    //int my_port = 25001;
    //char * my_username="jmh";
    //char * my_password ="wujieyijiu";
    this->my_ssh_session = ssh_new();

    if (this->my_ssh_session == NULL)
    {
        popup.setlabel("Login failed");
        popup.show();
        return;
        //emit SignalLogin(my_ssh_session);
    }

    ssh_options_set(this->my_ssh_session, SSH_OPTIONS_HOST, hostname);
    ssh_options_set(this->my_ssh_session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(this->my_ssh_session, SSH_OPTIONS_USER, username);

    rc = ssh_connect(my_ssh_session);

    if (rc != SSH_OK)
    {
      fprintf(stderr, "Error connecting to localhost: %s\n",
              ssh_get_error(my_ssh_session));
      popup.setlabel("Login failed");
      popup.show();
      return;
    }

    if (verify_knownhost(my_ssh_session) < 0)
    {
      ssh_disconnect(my_ssh_session);
      ssh_free(my_ssh_session);
      popup.setlabel("verify host failed");
      popup.show();
      return;
    }

    rc = ssh_userauth_password(this->my_ssh_session, NULL, password);

    if (rc != SSH_AUTH_SUCCESS)
    {
      //cout<<"1111"<<endl;
      fprintf(stderr, "Error authenticating with password: %s\n",
              ssh_get_error(this->my_ssh_session));
      //cout<<"1111"<<endl;
      ssh_disconnect(this->my_ssh_session);
      ssh_free(this->my_ssh_session);
      popup.setlabel("Login failed");
      popup.show();
      return;
    }

    popup.setlabel("Login success");
    login=1;
    popup.show();
    return;

}


void MainWindow::Send(char * remote_filepath,char* email,char* path, char* name,bool encoded){

    content_t ct;
    strcpy(ct.from_name, "FileManager");
    strcpy(ct.from_addr, "hui200918@163.com");
    strcpy(ct.to_name, "Muhui Jiang");
    strcpy(ct.to_addr, email);
    strcpy(ct.subject, "Decode Number");
    string receive_email=char_star_to_string(email)+"\0";


    if(encoded==0){
        string filepath = char_star_to_string(path)+char_star_to_string(name);
        cout<<"filepath:"<<filepath<<endl;
        string remote =char_star_to_string(remote_filepath)+"/"+char_star_to_string(name);
        cout<<"remote:"<<remote<<endl;
        if((sftp_local_remote(my_ssh_session,remote,filepath)==SSH_OK)){
            popup.setlabel("send success");
            popup.show();
            return;
        }
        else{
            popup.setlabel("send fail");
            popup.show();
            return;
        }

    }else{
        char * key_path= (char *)((char_star_to_string(path)+char_star_to_string(name)+".key").c_str());

        cout<<key_path<<endl;
        FILE * fp_key = fopen(key_path,"r");
        if(fp_key ==NULL){
            popup.setlabel("send fail");
            popup.show();
            return;
        }
        char  key[100];
        fscanf(fp_key,"%s",key);
        ct.text = key;
        send_mail("smtp.163.com", "hui200918", "wujieyijiu", &ct, (char*)receive_email.c_str());
        string filepath = char_star_to_string(path)+char_star_to_string(name)+".ecd";
        cout<<"filepath:"<<filepath<<endl;
        string remote =char_star_to_string(remote_filepath)+"/"+char_star_to_string(name)+".ecd";
        cout<<"remote:"<<remote<<endl;
        string remote_decode =char_star_to_string(remote_filepath)+"/decode";
        cout<< remote_decode<<endl;
    if((sftp_local_remote(my_ssh_session,remote,filepath)==SSH_OK)&&(sftp_local_remote(my_ssh_session,remote_decode,"/usr/local/src/decode")==SSH_OK)){
        popup.setlabel("send success");
        popup.show();
        return;
    }
    else{
        popup.setlabel("send fail");
        popup.show();
        return;
    }
    }




}

void MainWindow::get_file(char * remote_path,char * local_path){

    cout<<"remote:"<<remote_path<<endl;
    cout<<"local:"<<local_path<<endl;
    int rc;
    rc=sftp_read_sync(my_ssh_session,char_star_to_string(remote_path),char_star_to_string(local_path));
    if(rc==SSH_OK){
    popup.setlabel("pull success");
    popup.show();
    return;}
    else{
        popup.setlabel("pull failed");
        popup.show();
        return;
    }
}

void MainWindow::on_actionTransfer_triggered()
{
    if(login!=1){
        popup.setlabel("Please login first");
        popup.show();
        return;
    }
    else{
        pull_doc.show();
    }
}

void MainWindow::on_actionLogout_triggered()
{
    login=0;
    ssh_free(my_ssh_session);
    popup.setlabel("logout success");
    popup.show();
    return;

}
