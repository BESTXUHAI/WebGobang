#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "gamewindow.h"
#include "login.h"
#include "connector.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void chessshow();
private slots:

    void on_Playvs_Button_clicked();

    void on_AIvsAI_Button_clicked();

    void on_AIvs_Button_clicked();

    void returnMainWindow();

    void loginReturn_slot(bool isSuccess);

private:
    Ui::MainWindow *ui;
    GameWindow *gameWindow;
    Login *loginWindow;
    bool islogin;
};
#endif // MAINWINDOW_H
