#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QFrame>
#include <QPainter>
#include <QTcpSocket>
#include "connector.h"

namespace Ui {
class Login;
}

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();
protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void on_loginButton_clicked();

    void on_registerButton_clicked();

    void on_resetButton_clicked();
    //处理连接器发送过来的信号
    void login_slot(bool isSuccess);;
    void register_slot(bool isSuccess);;
private:
    Ui::Login *ui;
    Connector *client_;

signals:
    void loginReturn_signal(bool isSuccess);
};

#endif // LOGIN_H
