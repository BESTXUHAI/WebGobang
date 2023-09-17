#include "login.h"
#include "ui_login.h"

Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    client_ = Connector::getInstance();
    if(client_->connectHost())
    {
        ui->resultLabel->setText("提示：成功连接服务器");
    }
    else
    {
        ui->resultLabel->setText("提示：无法连接服务器");
    }
    connect(client_, &Connector::login_Signal, this, &Login::login_slot);
    connect(client_, &Connector::register_Signal, this, &Login::register_slot);
}

Login::~Login()
{
    delete ui;
}

void Login::paintEvent(QPaintEvent *event)
{

    QPixmap backimg(QString(":/images/Welcome.jpg"));
    QPainter painter(this);
    painter.drawPixmap(0, 0, this->width(), this->height(), backimg);

}

void Login::on_loginButton_clicked()
{
    QString user = ui->userEdit->text();
    QString passwd = ui->passwdEdit->text();
    QString message = QString("LOGIN\r\n%1 %2\r\n").arg(user,passwd);
    client_->sendMessage(message);
}

void Login::on_registerButton_clicked()
{
    QString user = ui->userEdit->text();
    QString passwd = ui->passwdEdit->text();
    QString message = QString("REGISTER\r\n%1 %2\r\n").arg(user,passwd);
    client_->sendMessage(message);
}
//返回
void Login::on_resetButton_clicked()
{
    emit loginReturn_signal(false);
}

void Login::login_slot(bool isSuccess)
{
    if(isSuccess)
    {
        ui->resultLabel->setText("提示：登陆成功");
        emit loginReturn_signal(true);
    }
    else
    {
        ui->resultLabel->setText("提示：登陆失败，账号或密码错误");
    }

}

void Login::register_slot(bool isSuccess)
{
    if(isSuccess)
    {
        ui->resultLabel->setText("提示：注册成功");
    }
    else
    {
        ui->resultLabel->setText("提示：注册失败，账号已存在");
    }
}

