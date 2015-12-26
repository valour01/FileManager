#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <aboutdlg.h>
#include <inputdialog.h>
#include <atrdialog.h>
#include <editwindow.h>
#include <senddialog.h>
#include <QWidget>
#include <vector>
#include "CFileNode.h"
#include <QListWidgetItem>

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    AboutDlg about;
    inputDialog Input;
    atrdialog atrribute;
    EditWindow edit;
    sendDialog send;

    //temp valuable
    vector<CFileNode*> result;//file dir
    vector<string> currentPath;//store current path

    //store filenode
    vector<CFileNode*> allFiles;

    vector<CFileNode*> makeCurrentFileDir(const char *dir);
    vector<CFileNode*> makeAllFileDir(const char *dir);
    void InitSearchWidget();
    ~MainWindow();


private slots:
    void on_action_A_triggered();

    void on_action_S_triggered();

    void on_pushButton_2_clicked();

    void on_showSideBar_triggered();

    void on_outPutListWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_lineEdit_2_textChanged(const QString &arg1);


    void on_GOTO_clicked();

    void on_outPutListWithoutSearch_itemDoubleClicked(QListWidgetItem *item);

    void on_lineEdit_3_textChanged(const QString &arg1);

    void on_action_F_triggered();

    void on_action_D_triggered();

    void on_action_C_3_triggered();

    void on_action_P_2_triggered();

    void on_action_D_2_triggered();

    void on_action_C_2_triggered();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_action_R_2_triggered();

    void on_action_P_triggered();

    void on_refresh_triggered();

    void on_actionEdit_triggered();

    void on_Send_clicked();

signals:
    void SignalsetFolderName(char * fname);
    void SignalsetFileName(char * name);
    void SignalsetOldName(QString oname);
    void SignalsetStat(QString name);

private:
    Ui::MainWindow *ui;
    char sString[1000];
    char temp_paths[1000],temp_patht[1000];
    int mode = -1;
    void Copy(char * a, char * b);
    void d_copy(char * a, char * b);
    void mydelete(const char * del_name);
    };

#endif // MAINWINDOW_H
