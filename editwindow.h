#ifndef EDITWINDOW_H
#define EDITWINDOW_H

#include <QMainWindow>
#include <qtextedit.h>
#include <keydialog.h>
#include <string>

using namespace std;
namespace Ui {
class EditWindow;
}

class EditWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditWindow(QWidget *parent = 0);
    keyDialog key;

    ~EditWindow();

private slots:
    void on_actionOpen_triggered();

    void on_actionSave_triggered();

    void on_actionKey_triggered();

    void setkey(char *);

private:
    Ui::EditWindow *ui;
    void openFile();
    void saveFile();
    int set_key;
    string my_key;
    QAction * openAction;
    QAction * saveAction;
    QTextEdit * textEdit;
};

#endif // EDITWINDOW_H
