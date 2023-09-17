#ifndef RESULTDIALOG_H
#define RESULTDIALOG_H

#include <QDialog>

namespace Ui {
class resultDialog;
}

class resultDialog : public QDialog
{
    Q_OBJECT

public:
    explicit resultDialog(QWidget *parent = nullptr);
    ~resultDialog();
    void showresult(QString path);
    void showpeaceful();
private slots:
    void on_pushButton_clicked();

private:
    Ui::resultDialog *ui;
};

#endif // RESULTDIALOG_H
