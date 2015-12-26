#include "keydialog.h"
#include "ui_keydialog.h"

keyDialog::keyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::keyDialog)
{
    ui->setupUi(this);
}

keyDialog::~keyDialog()
{
    delete ui;
}
