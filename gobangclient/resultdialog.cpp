#include "resultdialog.h"
#include "ui_resultdialog.h"

resultDialog::resultDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::resultDialog)
{
    ui->setupUi(this);
    //去掉边框
    this->setWindowFlags(Qt::FramelessWindowHint);
}

resultDialog::~resultDialog()
{
    delete ui;
}

void resultDialog::showresult(QString path)
{
    QPixmap winPic(QString(":/images/win.png"));
    QPixmap chessPic(path);
    ui->winlabel->setScaledContents(true);
    ui->chesslabel->setScaledContents(true);
    ui->winlabel->setPixmap(winPic);
    ui->chesslabel->setPixmap(chessPic);

}
void resultDialog::showpeaceful()
{
    QPixmap peaceimg(QString(":/images/peace.png"));
    ui->winlabel->setScaledContents(true);
    ui->winlabel->setPixmap(peaceimg);

}
void resultDialog::on_pushButton_clicked()
{
    this->hide();
}
