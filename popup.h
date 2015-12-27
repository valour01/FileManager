#ifndef POPUP_H
#define POPUP_H

#include <QDialog>

namespace Ui {
class Popup;
}

class Popup : public QDialog
{
    Q_OBJECT

public:
    explicit Popup(QWidget *parent = 0);
    void setlabel(QString content);
    ~Popup();

private slots:
    void on_pushButton_clicked();

private:
    Ui::Popup *ui;
};

#endif // POPUP_H
