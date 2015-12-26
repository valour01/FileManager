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
}

EditWindow::~EditWindow()
{
    delete ui;
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

void EditWindow::on_actionSave_triggered()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save File"), ".", tr("Text File(*.txt)"));

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
}

void EditWindow::on_actionKey_triggered()
{
    key.show();
}
