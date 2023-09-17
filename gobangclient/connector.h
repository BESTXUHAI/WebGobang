#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QPoint>
/*
    GAME响应状态
    LOGIN：请求登录
    REGISTER：请求注册
    MATE：匹配对局
    MESSAGE：对局信息
    GAMERESULT：对局结果

*/
enum REQUEST_STATES {LOGIN = 0, REGISTER, MATE, MESSAGE, GAMERESULT, UNKNOWN};
enum REQUEST_RESULT {SUCCESS = 0, FAIL};

//单例模式连接器，用于tcp通信
class Connector : public QObject
{
    Q_OBJECT
private:

    QString ipAddress_;
    quint16 port_;
    //是否成功连接
    bool isConnected_;
    //客户端套接字
    QTcpSocket *client_;
    //读数据缓存
    QString readbuf_;

private:
    explicit Connector(QObject *parent = nullptr, QString ipAddress = "192.168.150.130", quint16 port = 8000);
    //处理读取到的数据
    void readDatedeal();

public:
    ~Connector();
    static Connector *getInstance();
    bool connectHost();
    bool getconnState();
    void sendMessage(QString message);
    void closeConnect();

signals:
    //登录消息处理信号
    void login_Signal(bool isSuccess);
    //注册消息处理信号
    void register_Signal(bool isSuccess);
    //匹配消息处理信号
    void mate_Signal(int mode);
    //对手认输信号
    void rivalloss_Signal();
    //落子信息传递信号
    void drop_Signal(QPoint point);

private slots:
    void readyRead_Slot();//读取信息
    void disconnected_Slot();//连接被断开

};

#endif // CONNECTOR_H
