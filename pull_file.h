#ifndef PULL_FILE_H
#define PULL_FILE_H

#include <QDialog>

namespace Ui {
class pull_file;
}

class pull_file : public QDialog
{
    Q_OBJECT

public:
    explicit pull_file(QWidget *parent = 0);
    ~pull_file();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

signals:
    void SignalPull(char * remote_path, char*local_path);


private:
    Ui::pull_file *ui;
};

#endif // PULL_FILE_H
