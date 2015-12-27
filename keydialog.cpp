#include "keydialog.h"
#include "ui_keydialog.h"

keyDialog::keyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::keyDialog)
{
    ui->setupUi(this);
    ui->key->setEchoMode(QLineEdit::Password);
    ui->confirm_key->setEchoMode(QLineEdit::Password);
}

keyDialog::~keyDialog()
{
    delete ui;
}

void keyDialog::on_pushButton_clicked()
{

    QString ftext = ui->key->text();
    QByteArray hostname_array = ftext.toLatin1();
     char *key = hostname_array.data();
    //cout<<"hostname:"<<hostname<<endl;

    QString ftext_2 = ui->confirm_key->text();
    QByteArray username_array = ftext_2.toLatin1();
    //cout<<"hostname:"<<hostname<<endl;
     char *confirm_key = username_array.data();
     if(strcmp(key,confirm_key)!=0)
     {
         popup.setlabel("password not same");
         popup.show();
     }
     else{
         emit Signalencode(key);
         this->hide();
         ui->key->clear();
         ui->confirm_key->clear();
     }

}
