#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QWidget>
#include <QTime>
#include <QDebug>
#include <QStack>
#include "resultdialog.h"
#include "chessboard.h"
#include "aiplayer.h"
#include "connector.h"
namespace Ui {
class GameWindow;
}

class GameWindow : public QWidget
{
    Q_OBJECT

public:
    //游戏模式
    enum Gamemode{PLAYERVSPLAYER, PLAYERVSAI, AIVSAI};
    //游戏状态
    enum GameState{GAMERUNNING, GAMEOVER};
    //当前执棋的角色，当前玩家，电脑，对手
    enum CurrentPlayer{PLAYER, AI, OTHER};
    explicit GameWindow(QWidget *parent = nullptr);
    ~GameWindow();
    void running();
    void init();
    void process_allEvents(int times);
    bool reasonable(int x,int y);
    bool judge();
    void showresult(bool peaceful);
    void setgamemode(int mode, bool playerfirst);
    void playchess();
    void cheessTypeChange();
    //判断当前形式
    void statusJudge(int mode);
private slots:
    void on_startButton_clicked();
    void on_restartButton_clicked();
    void on_returnButton_clicked();
    void on_repentanceButton_clicked();
    //处理配对信息
    void mate_Slot(int mode);
    //对方下棋
    void drop_Slot(QPoint point);
    //对方认输
    void rivalloss_Slot();
protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void showMainWindow();

private:
    Ui::GameWindow *ui;
    ChessBoard *chessBoard;
    ChessType ** chessInfo;
    Gamemode gameMode;
    GameState gameState;
    CurrentPlayer currentplayer;
    CurrentPlayer firstplayer;
    resultDialog *redialog;
    ChessType currentchesstype;
    AIplayer *aiplayer;
    int totalchessnum;
    //记录下棋路径
    QStack<QPoint> pathStack;
    //连接器
    Connector *connector;
};

#endif // GAMEWINDOW_H
