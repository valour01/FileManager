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
using namespace std;


int verify_knownhost(ssh_session session);
int show_remote_processes(ssh_session session);
sftp_session init_sftp(ssh_session session);
int sftp_list_dir(ssh_session session, sftp_session sftp, char * path);
int sftp_mk_file(ssh_session session, const char * filename);
int sftp_mk_file(ssh_session session, sftp_session sftp, const char * filename);
int sftp_list_contents(ssh_session session,char * path);
int mkdir_remote(ssh_session session);
int sftp_read_sync(ssh_session session, sftp_session sftp, const char * remote_file, const char * local_file);
int sftp_read_sync(ssh_session session,  const char * remote_file, const char * local_file);

int verify_knownhost(ssh_session session)
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
 // ...
  //sftp_free(sftp);
  return sftp;
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
  int rc;
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

int sftp_mk_file(ssh_session session, const char * filename)
{
  sftp_session sftp=init_sftp(session);
  //int sftp_rc = init_sftp(session,sftp);
  if(sftp ==NULL){
    return SSH_FATAL;
    }
  int rc;
  rc = sftp_mk_file(session,sftp,filename);
  if(rc !=SSH_OK){
    sftp_free(sftp);
    return SSH_ERROR;
    }
    sftp_free(sftp);
return SSH_OK;


}

int sftp_mk_file(ssh_session session, sftp_session sftp, const char * filename)
{
  int access_type = O_WRONLY | O_CREAT | O_TRUNC;
  sftp_file file;
  //const char *helloworld = "Hello, World!\n";
  //int length = strlen(helloworld);
  int rc;
  file = sftp_open(sftp, filename,
                   access_type, S_IRWXU);
  if (file == NULL)
  {
    fprintf(stderr, "Can't open file for writing: %s\n",
            ssh_get_error(session));
    return SSH_ERROR;
  }
  /* nwritten = sftp_write(file, helloworld, length);
  if (nwritten != length)
  {
    fprintf(stderr, "Can't write data to file: %s\n",
            ssh_get_error(session));
    sftp_close(file);
    return SSH_ERROR;
  }*/
  rc = sftp_close(file);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Can't close the written file: %s\n",
            ssh_get_error(session));
    return rc;
  }
  return SSH_OK;
}

#define MAX_XFER_BUF_SIZE 16384
int sftp_read_sync(ssh_session session, sftp_session sftp, const char * remote_file, const char * local_file)
{
  int access_type;
  sftp_file file;
  char buffer[MAX_XFER_BUF_SIZE];
  int nbytes, nwritten, rc;
  int fd;
  access_type = O_RDONLY;
  file = sftp_open(sftp, remote_file,
                   access_type, 0);
  if (file == NULL) {
      fprintf(stderr, "Can't open file for reading: %s\n",
              ssh_get_error(session));
      return SSH_ERROR;
  }
  fd = open(local_file, O_CREAT);
  if (fd < 0) {
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

int sftp_read_sync(ssh_session session,  const char * remote_file, const char * local_file)
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

void sendDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    QString ftext = ui->lineEdit->text();
    char * hostname = ftext.toLocal8Bit().data();
    QString ftext2 = ui->lineEdit_2->text();
    char * username = ftext2.toLocal8Bit().data();
    QString ftext3 = ui->lineEdit_3->text();
    char * password = ftext3.toLocal8Bit().data();
    QString ftext4 = ui->lineEdit_4->text();
    char * port = ftext4.toLocal8Bit().data();
    QString ftext5 = ui->lineEdit_5->text();
    char * filepath = ftext5.toLocal8Bit().data();
    QString ftext6 = ui->lineEdit_6->text();
    char * email = ftext6.toLocal8Bit().data();

    ssh_session my_ssh_session;
    int rc;
    int int_port = atoi( port);
    my_ssh_session = ssh_new();

    if (my_ssh_session == NULL)
      exit(-1);

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, hostname);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &int_port);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, username);

    rc = ssh_userauth_password(my_ssh_session, NULL, password);

    if (rc != SSH_AUTH_SUCCESS)
    {
      fprintf(stderr, "Error authenticating with password: %s\n",
              ssh_get_error(my_ssh_session));
      ssh_disconnect(my_ssh_session);
      ssh_free(my_ssh_session);
      exit(-1);
    }


    if(sftp_mk_file(my_ssh_session, filepath)==SSH_OK)
    {
          cout<<"read successfully"<<endl;
        }

    if(sftp_read_sync(my_ssh_session,"helloworld/helloworld.txt","/tmp/helloworld.txt")==SSH_OK){
          cout<<"read successfully"<<endl;
    }


}
