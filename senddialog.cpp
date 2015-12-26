#include "senddialog.h"
#include "ui_senddialog.h"

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
