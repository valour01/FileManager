#include "popup.h"
#include "ui_popup.h"

Popup::Popup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Popup)
{
    ui->setupUi(this);
}

Popup::~Popup()
{
    delete ui;
}

void Popup::on_pushButton_clicked()
{
    this->hide();
}

void Popup::setlabel( QString content)
{
    ui->label->setText(content);
}
