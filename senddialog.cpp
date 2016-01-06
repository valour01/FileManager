#include "senddialog.h"
#include "ui_senddialog.h"
#include <QMessageBox>
#define LIBSSH_STATIC 1
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "smtp.h"
#include <stdio.h>
using namespace std;


//int verify_knownhost(ssh_session session);
int show_remote_processes(ssh_session session);
sftp_session init_sftp(ssh_session session);
int sftp_list_dir(ssh_session session, sftp_session sftp, char * path);
int sftp_mk_file(ssh_session session, const char * filename);
int sftp_mk_file(ssh_session session, sftp_session sftp, const char * filename);
int sftp_list_contents(ssh_session session,char * path);
int mkdir_remote(ssh_session session);
int sftp_read_sync(ssh_session session, sftp_session sftp, const char * remote_file, const char * local_file);
int sftp_read_sync(ssh_session session,  const char * remote_file, const char * local_file);
int connect_smtp_server(const char* server_addr);
char* base64_encode(const char* src, char* des);
int communicate_server(int sockfd, const char* message);






int show_remote_processes(ssh_session session)
{
  ssh_channel channel;
  int rc;
  char buffer[256];
  int nbytes;
  channel = ssh_channel_new(session);
  if (channel == NULL)
    return SSH_ERROR;
  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK)
  {
    ssh_channel_free(channel);
    return rc;
  }
  rc = ssh_channel_request_exec(channel, "ps aux");
  if (rc != SSH_OK)
  {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return rc;
  }
  nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
  while (nbytes > 0)
  {
    if (write(1, buffer, nbytes) != (unsigned int) nbytes)
    {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return SSH_ERROR;
    }
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
  }

  if (nbytes < 0)
  {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return SSH_ERROR;
  }
  ssh_channel_send_eof(channel);
  ssh_channel_close(channel);
  ssh_channel_free(channel);
  return SSH_OK;
}


int sftp_list_dir(ssh_session session, sftp_session sftp, char * path)
{
  sftp_dir dir;
  sftp_attributes attributes;
  int rc;
  dir = sftp_opendir(sftp, path);
  if (!dir)
  {
    fprintf(stderr, "Directory not opened: %s\n",
            ssh_get_error(session));
    return SSH_ERROR;
  }
  printf("Name                       Size Perms    Owner\tGroup\n");
  while ((attributes = sftp_readdir(sftp, dir)) != NULL)
  {
    printf("%-20s %10llu %.8o %s(%d)\t%s(%d)\n",
     attributes->name,
     (long long unsigned int) attributes->size,
     attributes->permissions,
     attributes->owner,
     attributes->uid,
     attributes->group,
     attributes->gid);
     sftp_attributes_free(attributes);
  }
  if (!sftp_dir_eof(dir))
  {
    fprintf(stderr, "Can't list directory: %s\n",
            ssh_get_error(session));
    sftp_closedir(dir);
    return SSH_ERROR;
  }
  rc = sftp_closedir(dir);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Can't close directory: %s\n",
            ssh_get_error(session));
    return rc;
  }
}

int sftp_list_contents(ssh_session session,char * path){
    sftp_session sftp=init_sftp(session);
    if(sftp==NULL){
    return SSH_FATAL;
}
    int rc=sftp_list_dir(session,sftp, path);
    sftp_free(sftp);
  return rc;
}

int mkdir_remote(ssh_session session)
{
  sftp_session sftp=init_sftp(session);
  //int sftp_rc = init_sftp(session,sftp);
  if(sftp ==NULL){
    return SSH_FATAL;
}
  int rc,s;
  cout<<"before mkdir"<<endl;
  rc = sftp_mkdir(sftp, "helloworld", S_IRWXU);
  cout<<"after mkdir"<<endl;
  if (rc != SSH_OK)
  {
    if (sftp_get_error(sftp) != SSH_FX_FILE_ALREADY_EXISTS)
    {
      fprintf(stderr, "Can't create directory: %s\n",
              ssh_get_error(session));
        return rc;
    }
  }
sftp_free(sftp);
  return SSH_OK;
}





sendDialog::sendDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::sendDialog)
{
    ui->setupUi(this);

}

sendDialog::~sendDialog()
{
    delete ui;
}

void sendDialog::on_pushButton_2_clicked()
{

    QString ftext = ui->filepath->text();
    QByteArray filepath_array = ftext.toLatin1();
    char * remote_filepath = filepath_array.data();

    QString ftext2 = ui->email->text();
    QByteArray email_array = ftext2.toLatin1();
    char * email = email_array.data();

    QString ftext3 = ui->path->text();
    QByteArray path_array = ftext3.toLatin1();
    char * path = path_array.data();

    QString ftext4 = ui->name->text();
    QByteArray name_array = ftext4.toLatin1();
    char * name = name_array.data();
    bool encoded = ui->checkBox->isChecked();
    //cout<<"checkbox:"<<checked<<endl;
    emit SignalSend(remote_filepath,email,path,name,encoded);
    ui->filepath->clear();
    ui->email->clear();
    this->hide();

}

void sendDialog::showMsg()
{
    super:
    QStringList tmp = this->sendName.split("/");
    QString path = "";
    int num = tmp.count() - 1;
    for(int i = 1; i<num; i++)
    {
        path = path + "/" + tmp.at(i);
    }
    QString fname = tmp.at(num);
    ui->path->setText(path);
    ui->name->setText(fname);
    //ui->property->setText(path+fname);
}

void sendDialog::setStat(QString name)
{
    this->sendName = name;
}



void sendDialog::on_pushButton_clicked()
{
    this->hide();
}
