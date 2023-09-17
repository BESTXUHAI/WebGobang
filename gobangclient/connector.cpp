#include "connector.h"

Connector::Connector(QObject *parent, QString ipAddress, quint16 port)
    : QObject(parent),
      ipAddress_(ipAddress),
      port_(port),
      isConnected_(false),
      client_(new QTcpSocket(this))
{
    connect(client_, &QTcpSocket::disconnected, this, &Connector::disconnected_Slot);
    connect(client_, &QTcpSocket::readyRead, this, &Connector::readyRead_Slot);
    readbuf_ = "";
}

Connector::~Connector()
{

}

bool Connector::connectHost()
{
    if(isConnected_)
        return true;
    client_->connectToHost(ipAddress_, port_);
    //等待建立连接1s
    isConnected_ = client_->waitForConnected(1000);
    if(isConnected_)
    {
        qDebug()<<"建立连接";
    }
    readbuf_ = "";
    return isConnected_;
}

//懒汉模式
Connector *Connector::getInstance()
{
    static Connector connector;
    return &connector;
}

void Connector::disconnected_Slot()
{
    qDebug()<<"连接已关闭";
    isConnected_ = false;
}


void Connector::readyRead_Slot()
{
    QByteArray data = client_->readAll();

    QString message = QString::fromLocal8Bit(data.data());
    readbuf_ = readbuf_+message;
    qDebug()<<message;
    readDatedeal();
}

void Connector::readDatedeal()
{
    int len = readbuf_.length();
    //未读取到结束标志符
    if(readbuf_[len-1] != '\n')
    {
        return;
    }
    QStringList readlist =  readbuf_.split("\r\n");
    if(readlist.size() < 2)
        return ;

    for(int i=0; i<readlist.size(); i++)
           qDebug()<<"list:"<<readlist[i];

    if(readlist[0] == "LOGIN")
    {
        if(readlist[1] == "SUCCESS")
            emit login_Signal(true);
        else if(readlist[1] == "FAIL")
            emit login_Signal(false);
        else
            qDebug()<<"unknown:"<<readlist[1];
    }
    else if(readlist[0] == "REGISTER")
    {
        if(readlist[1] == "SUCCESS")
            emit register_Signal(true);
        else if(readlist[1] == "FAIL")
            emit register_Signal(false);
        else
            qDebug()<<"unknown:"<<readlist[1];
    }
    else if(readlist[0] == "MATE")
    {
        //匹配成功
        if(readlist[1] == "FAIL")
        {
            emit mate_Signal(0);
        }
        else if(readlist[1] == "FIRST")
        {
            emit mate_Signal(1);
        }
        else if(readlist[1] == "SECOND")
        {
            emit mate_Signal(2);
        }
    }
    else if(readlist[0] == "MESSAGE")
    {
        QStringList pointlist = readlist[1].split(" ");
        QPoint point(pointlist[0].toInt(), pointlist[1].toInt());
        emit drop_Signal(point);
    }
    else if(readlist[0] == "GAMERESULT")
    {
        emit rivalloss_Signal();
    }
    else
        qDebug()<<"unknown:"<<readlist[0];

    //数据完整，清空缓存
    readbuf_ = "";
    //剩余数据
    for(int i=2; i<readlist.size(); i++)
        if(readlist[i] != "")
            readbuf_ = readbuf_ + readlist[i];
}


bool Connector::getconnState()
{
    return isConnected_;
}


void Connector::sendMessage(QString message)
{
    QByteArray data = message.toLocal8Bit();
    if(client_->isOpen() && client_->isValid())
    {
        client_->write(data);
    }
}

void Connector::closeConnect()
{
    if(isConnected_)
    {
        client_->close();
        isConnected_ = false;
    }
}

