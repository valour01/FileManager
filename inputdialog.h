#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class inputDialog;
}

class inputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit inputDialog(QWidget *parent = 0);
    ~inputDialog();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);
    void setFolderName(char * fname);
    void setFileName(char * name);
    void setOldName(QString oname);
private:
    Ui::inputDialog *ui;
    char folderName[1000];
    char fileName[1000];
    QString oldName;
    int mode = -1;
};

#endif // INPUTDIALOG_H
