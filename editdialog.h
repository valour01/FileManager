#ifndef EDITDIALOG_H
#define EDITDIALOG_H

#include <QDockWidget>

namespace Ui {
class editdialog;
}

class editdialog : public QDockWidget
{
    Q_OBJECT

public:
    explicit editdialog(QWidget *parent = 0);
    ~editdialog();

private:
    Ui::editdialog *ui;
};

#endif // EDITDIALOG_H
