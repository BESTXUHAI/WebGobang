#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    gameWindow = new GameWindow();
    loginWindow = new Login();

    Qt::WindowFlags flags = loginWindow->windowFlags();
    flags &= ~Qt::WindowCloseButtonHint;        //禁止关闭按钮
    loginWindow->setWindowFlags(flags);

    flags = gameWindow->windowFlags();
    flags &= ~Qt::WindowCloseButtonHint;        //禁止关闭按钮
    gameWindow->setWindowFlags(flags);


    islogin = false;
    //通过发送信号返回主窗口
    connect(gameWindow, &GameWindow::showMainWindow, this, &MainWindow::returnMainWindow);
    //登录窗口返回
    connect(loginWindow, &Login::loginReturn_signal, this, &MainWindow::loginReturn_slot);

}

MainWindow::~MainWindow()
{
    delete ui;
    delete gameWindow;
    delete loginWindow;
}

void MainWindow::chessshow()
{

}
//双人对战
void MainWindow::on_Playvs_Button_clicked()
{
    //连接服务器
    Connector *connector = Connector::getInstance();
    connector->connectHost();

    if(islogin)
    {
        gameWindow->setgamemode(0, true);
        gameWindow->show();
    }
    else
    {
        loginWindow->show();
    }

    this->hide();
}

void MainWindow::on_AIvsAI_Button_clicked()
{
    gameWindow->setgamemode(2, true);
    gameWindow->show();
    this->hide();
}

void MainWindow::on_AIvs_Button_clicked()
{
    //人机对战判断是否选择先手
    gameWindow->setgamemode(1, ui->checkBox->isChecked());
    gameWindow->show();
    this->hide();
}

void MainWindow::returnMainWindow()
{
    gameWindow->hide();
    this->show();
}

void MainWindow::loginReturn_slot(bool isSuccess)
{
    islogin = isSuccess;
    loginWindow->hide();
    //成功登录
    if(islogin)
    {
        gameWindow->setgamemode(0, true);
        gameWindow->show();
    }
    else
    {
        this->show();
    }
}
