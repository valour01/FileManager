#include "editdialog.h"
#include "ui_editdialog.h"
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

editdialog::editdialog(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::editdialog)
{
    ui->setupUi(this);
}

editdialog::~editdialog()
{
    delete ui;
}
