#ifndef KEYDIALOG_H
#define KEYDIALOG_H

#include <QDialog>

namespace Ui {
class keyDialog;
}

class keyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit keyDialog(QWidget *parent = 0);
    ~keyDialog();

private:
    Ui::keyDialog *ui;
};

#endif // KEYDIALOG_H
