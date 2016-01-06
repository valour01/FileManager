#include "editwindow.h"
#include "ui_editwindow.h"
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QGridLayout>
#include "qfiledialog.h"
#include "qtextstream.h"
#include "qmessagebox.h"
#include <iostream>

using namespace std;

EditWindow::EditWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EditWindow)
{
    ui->setupUi(this);

    setWindowTitle(tr("Text Edit"));
    key.setWindowTitle("Set Key");
    textEdit = new QTextEdit(this);
    setCentralWidget(textEdit);
    QObject::connect(&key,SIGNAL(Signalencode(char*)),this,SLOT(setkey(char *)));
}

EditWindow::~EditWindow()
{
    delete ui;
}

string char_star_to_string_2(const char * target)
{
    string s(target);
    return s;
}

void EditWindow::setkey(char * key){
    cout<<"key"<<key<<endl;
    my_key = char_star_to_string_2(key);
    cout<<"mykey"<<my_key<<endl;
    set_key =1;
}


void EditWindow::on_actionOpen_triggered()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open File"), ".", tr("Text Files(*.txt)"));

    if(!path.isEmpty())
    {
        QFile file(path);
        if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            QMessageBox::warning(this, tr("Read File"),tr("can't open file"));
            return;
        }
        QTextStream in(&file);
        textEdit->setText(in.readAll());
        file.close();
    }
    else
    {
        QMessageBox::warning(this, tr("Path"), tr("you didn't select anything"));
    }
}





void encode(char * filepath,char * key){
    int sum=0;
    cout<<"encode"<<key<<endl;
    char * key_path=(char *)((char_star_to_string_2(filepath)+".key").c_str());
    FILE * k_fp = fopen(key_path,"w");
    int k;
    printf("key:%s\n",key);
    fprintf(k_fp,"key:%s\n",key);
    for (int i=0;i<strlen(key);i++){
        cout<<key[i]<<endl;
        sum+=int(key[i]);
    }
    fclose(k_fp);
    int last_move = sum%10;
    if( last_move == 0){
        last_move+=10;
    }


    FILE *fp=fopen(filepath, "r");

    char * encode_file = (char *)((char_star_to_string_2(filepath)+".ecd").c_str());

    FILE * w_fp = fopen(encode_file,"w");
    if( fp == NULL|w_fp==NULL ){
        cout<<"aaaaaaaaaaa"<<endl;
        return;
    }

    cout<<"debug"<<endl;
    int c;
    while ((c = fgetc(fp)) != EOF) {
      //  cout<<"start"<<endl;
        int temp = c+last_move;
        if(temp>=128){
           temp= temp-128;
        }
        printf("%c", temp);
        fputc(temp,w_fp);
        //cout<<c<<endl;
    }
    fclose(fp);
    fclose(w_fp);
    //cout<<"last_move"<<last_move<<endl;
}

void EditWindow::on_actionSave_triggered()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save File"), ".", tr("Text File(*.txt)"));
    cout<<path.toLatin1().data()<<endl;
    cout<<"save"<<my_key<<endl;


    if(!path.isEmpty())
    {
        QFile file(path);
        if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
        {
            QMessageBox::warning(this, tr("Save File"), tr("can't save file"));
            return;
        }
        QTextStream out(&file);
        out<<textEdit->toPlainText();
        file.close();
    }
    else
    {
        QMessageBox::warning(this, tr("Path"), tr("you didn't select anything"));
    }
    encode(path.toLatin1().data(),(char *)my_key.c_str());
}

void EditWindow::on_actionKey_triggered()
{
    key.show();
}
