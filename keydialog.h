#ifndef KEYDIALOG_H
#define KEYDIALOG_H

#include <QDialog>
#include <popup.h>
namespace Ui {
class keyDialog;
}

class keyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit keyDialog(QWidget *parent = 0);
    Popup popup;
    ~keyDialog();

private slots:
    void on_pushButton_clicked();

signals:
    void Signalencode(char* key);

private:
    Ui::keyDialog *ui;
};

#endif // KEYDIALOG_H
