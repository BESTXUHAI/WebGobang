#include "gamewindow.h"
#include "ui_gamewindow.h"

GameWindow::GameWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameWindow)
{
    ui->setupUi(this);
    //创建棋盘
    chessBoard = new ChessBoard();
    redialog = new resultDialog();
    chessInfo = chessBoard->getchessinfo();
    ui->chessboardlayout->addWidget(chessBoard);
    //创建ai
    aiplayer = new AIplayer(chessInfo);
    init();
    connector = Connector::getInstance();
    //匹配信息传递
    connect(connector, &Connector::mate_Signal, this, &GameWindow::mate_Slot);
    //落子信息传递
    connect(connector, &Connector::drop_Signal, this, &GameWindow::drop_Slot);
    //对方认输
    connect(connector, &Connector::rivalloss_Signal, this, &GameWindow::rivalloss_Slot);

}

GameWindow::~GameWindow()
{
    delete ui;
    delete chessBoard;
    delete redialog;
    delete aiplayer;
}


void GameWindow::init()
{

}


//重写绘画事件设置背景
void GameWindow::paintEvent(QPaintEvent *event)
{

    QPixmap backimg(QString(":/images/backGround.png"));
    QPainter painter(this);
    painter.drawPixmap(0, 0, this->width(), this->height(), backimg);

}




void GameWindow::setgamemode(int mode, bool playerfirst)
{
    QPixmap playerimg(QString(":/images/human.png"));
    QPixmap AIimg(QString(":/images/AIimg.jpg"));
    QPixmap pvsp(QString(":/images/PlayervsPlayer.png"));
    QPixmap pvsa(QString(":/images/AIvsPlayer.png"));
    QPixmap avsa(QString(":/images/AIvsAI.png"));

    //图片自适应大小
    ui->Player1imglabel->setScaledContents(true);
    ui->Player2imglabel->setScaledContents(true);
    ui->gamemodelabel->setScaledContents(true);
    if(mode == 0)
    {
        ui->Player1imglabel->setPixmap(playerimg);
        ui->Player2imglabel->setPixmap(playerimg);
        ui->p1namelabel->setText(QString("人类玩家"));
        ui->p2namelabel->setText(QString("人类玩家"));
        ui->gamemodelabel->setPixmap(pvsp);
        gameMode = PLAYERVSPLAYER;
        //隐藏重开和悔棋按钮
        ui->startButton->show();
        ui->returnButton->show();
        ui->restartButton->hide();
        ui->repentanceButton->hide();
        ui->metalabel->show();
        ui->metalabel->setText("请点击开始按钮，匹配游戏");

    }
    else if(mode == 1)
    {
        ui->Player1imglabel->setPixmap(playerimg);
        ui->Player2imglabel->setPixmap(AIimg);
        ui->p1namelabel->setText(QString("人类玩家"));
        ui->p2namelabel->setText(QString("人工智障"));
        ui->gamemodelabel->setPixmap(pvsa);
        gameMode = PLAYERVSAI;
        ui->startButton->show();
        ui->returnButton->show();
        ui->restartButton->show();
        ui->repentanceButton->show();
        ui->metalabel->hide();
    }
    else
    {
        ui->p1namelabel->setText(QString("人工智障"));
        ui->p2namelabel->setText(QString("人工智障"));
        ui->Player1imglabel->setPixmap(AIimg);
        ui->Player2imglabel->setPixmap(AIimg);
        ui->gamemodelabel->setPixmap(avsa);
        gameMode = AIVSAI;
        ui->startButton->show();
        ui->returnButton->show();
        ui->restartButton->show();
        ui->repentanceButton->hide();
        ui->metalabel->hide();
    }
    this->update();
    if(gameMode == PLAYERVSAI)
    {
        //设置先手或者后手
        if(playerfirst)firstplayer = PLAYER;
        else firstplayer = AI;
    }
    if(gameMode == AIVSAI)
        firstplayer = AI;

    if(gameMode == PLAYERVSPLAYER)
    {
        if(playerfirst)firstplayer = PLAYER;
        else firstplayer = OTHER;
    }
}


void GameWindow::showresult(bool peiceful)
{
    if(currentchesstype == blackChess && !peiceful)
    {
        QString path(":/images/blackchess.png");
        redialog->showresult(path);
    }
    if(currentchesstype == whiteChess && !peiceful)
    {
        QString path(":/images/whitechess.png");
        redialog->showresult(path);
    }
    if(peiceful)
    {
        redialog->showpeaceful();
    }
    redialog->show();

}
void GameWindow::process_allEvents(int times)
{

    QTime time;
    time.start();
    while(time.elapsed() < times)             //等待时间流逝times毫秒
        QCoreApplication::processEvents();   //不停地处理事件，让程序保持响应
}

bool GameWindow::reasonable(int x, int y)
{
    if(x>=0 && x<CHESSBOARD_SIZE && y>=0 && y<CHESSBOARD_SIZE)return true;
    return false;
}

//判断是否获胜
bool GameWindow::judge()
{
    QPoint point = chessBoard->getdropPoint();
    pathStack.push(point);
    int x = point.x();
    int y = point.y();
    int num;
    int xx,yy;
    for(xx=x,yy=y,num=-1; reasonable(xx,yy)&&chessInfo[x][y]==chessInfo[xx][yy];xx++,yy++)num++;
    for(xx=x,yy=y; reasonable(xx,yy)&&chessInfo[x][y]==chessInfo[xx][yy];xx--,yy--)num++;
    if(num >= 5)return true;

    //QT 的x坐标向右,y坐标向下
    for(xx=x,yy=y,num=-1; reasonable(xx,yy)&&chessInfo[x][y]==chessInfo[xx][yy];xx--,yy++)num++;
    for(xx=x,yy=y; reasonable(xx,yy)&&chessInfo[x][y]==chessInfo[xx][yy];xx++,yy--)num++;
    if(num >= 5)return true;

    for(xx=x,yy=y,num=-1; reasonable(xx,yy)&&chessInfo[x][y]==chessInfo[xx][yy];xx++)num++;
    for(xx=x,yy=y; reasonable(xx,yy)&&chessInfo[x][y]==chessInfo[xx][yy];xx--)num++;
    if(num >= 5)return true;

    for(xx=x,yy=y,num=-1; reasonable(xx,yy)&&chessInfo[x][y]==chessInfo[xx][yy];yy++)num++;
    for(xx=x,yy=y; reasonable(xx,yy)&&chessInfo[x][y]==chessInfo[xx][yy];yy--)num++;
    if(num >= 5)return true;

    return false;
}


//开始下棋
void GameWindow::running()
{
    //黑子先手
    currentchesstype = blackChess;
    totalchessnum = 0;
    while(gameState == GAMERUNNING)
    {
        //和棋
        if(totalchessnum == 225)
        {
            gameState = GAMEOVER;
            showresult(true);
            break;
        }
        chessBoard->setnowChesstype(currentchesstype);
        if(currentplayer == PLAYER)
        {
            chessBoard->setisPlayer(true);
            //循环等待玩家下棋
            chessBoard->setdroped(false);
            //处理事件
            while(!chessBoard->getdroped() && gameState == GAMERUNNING){
                //处理队列中的事件
                process_allEvents(50);
            }

            chessBoard->setisPlayer(false);
        }
        //AI下棋
        else
        {
            QPoint point = aiplayer->work(currentchesstype);
            chessBoard->setdropPoint(point);
            chessBoard->update();
            process_allEvents(500);
        }
        if(judge() && gameState == GAMERUNNING)
        {
            gameState = GAMEOVER;
            if(gameMode == PLAYERVSAI && currentplayer == AI)
                ui->p2namelabel->setText(QString("你太弱了"));
            showresult(false);
        }
        cheessTypeChange();
        //人机对战模式时改变下的棋人
        if(gameMode == PLAYERVSAI)
        {
            if(currentplayer == PLAYER)currentplayer = AI;
            else currentplayer = PLAYER;
        }
        totalchessnum++;
    }

}

void GameWindow::on_startButton_clicked()
{
    chessBoard->init();
    if(gameMode == PLAYERVSPLAYER)
    {
        connector->sendMessage("MATE\r\n");
        ui->metalabel->setText("正在匹配对手......");
    }
    else
    {
        pathStack.clear();
        ui->startButton->setEnabled(false);
        gameState = GAMERUNNING;
        currentplayer = firstplayer;
        running();
    }

}

void GameWindow::on_restartButton_clicked()
{

    chessBoard->init();
    gameState = GAMEOVER;
    ui->startButton->setEnabled(true);
}


void GameWindow::on_returnButton_clicked()
{
    chessBoard->init();
    gameState = GAMEOVER;
    ui->startButton->setEnabled(true);

    if(connector != nullptr)
        connector->closeConnect();

    emit showMainWindow();
}

//悔棋
void GameWindow::on_repentanceButton_clicked()
{
    if(gameState != GAMERUNNING || currentplayer != PLAYER)return ;
    int k=2;
    while(!pathStack.empty() && k)
    {
        QPoint point = pathStack.pop();
        chessInfo[point.x()][point.y()] = empty;
        k--;
        totalchessnum--;
    }
    chessBoard->update();
}
//处理配对信息
void GameWindow::mate_Slot(int mode)
{
    if(mode == 0)
    {
        ui->metalabel->setText("当前无玩家等待，请稍后再试");
    }
    else
    {
        pathStack.clear();
        ui->startButton->setEnabled(false);
        gameState = GAMERUNNING;
        //currentplayer = firstplayer;
        totalchessnum = 0;
        currentchesstype = blackChess;
        if(mode == 1)
        {
            ui->metalabel->setText("匹配成功，先手执黑棋");
            firstplayer = PLAYER;
            playchess();
        }
        else
        {
            ui->metalabel->setText("匹配成功，后手执白棋");
            firstplayer = OTHER;
        }

    }
}
//主动下棋
void GameWindow::playchess()
{
    if(gameState != GAMERUNNING)
        return;
    //设置当前棋子颜色
    chessBoard->setnowChesstype(currentchesstype);
    chessBoard->setisPlayer(true);
    //循环等待玩家下棋
    chessBoard->setdroped(false);
    //处理事件
    while(!chessBoard->getdroped() && gameState == GAMERUNNING){
        //处理队列中的事件
        process_allEvents(50);
    }
    chessBoard->setisPlayer(false);
    //状态改变
    if(gameState != GAMERUNNING)
        return;

    QPoint point = chessBoard->getdropPoint();
    //将落子信息传递给对方
    connector->sendMessage(QString("MESSAGE\r\n%1 %2\r\n").arg(point.x()).arg(point.y()));
    statusJudge(0);
}

//对方下棋
void GameWindow::drop_Slot(QPoint point)
{
    if(gameState != GAMERUNNING)
        return;
    chessBoard->setnowChesstype(currentchesstype);

    chessBoard->setdropChess(point);
    chessBoard->update();
    process_allEvents(500);
    statusJudge(1);
    //轮到玩家
    playchess();
}

//改变当前棋子颜色
void GameWindow::cheessTypeChange()
{
    if(currentchesstype == whiteChess)
        currentchesstype = blackChess;
    else currentchesstype = whiteChess;

}

void GameWindow::statusJudge(int mode)
{

    //获胜
    if(judge())
    {
        gameState = GAMEOVER;
        showresult(false);
        if(mode == 0)
            ui->metalabel->setText("恭喜你，获得胜利！");
        else
            ui->metalabel->setText("你被击败！");
    }
    totalchessnum++;
    //和棋
    if(totalchessnum == 225 && gameState != GAMEOVER)
    {
        gameState = GAMEOVER;
        showresult(true);
    }
    //改变棋子颜色
    cheessTypeChange();
}

//对手认输当前玩家获胜
void GameWindow::rivalloss_Slot()
{
    if(gameState == GAMEOVER)
        return;
    if(firstplayer == PLAYER && currentchesstype != blackChess)
        cheessTypeChange();
    if(firstplayer == OTHER && currentchesstype != whiteChess)
        cheessTypeChange();

    gameState = GAMEOVER;
    showresult(false);
    ui->metalabel->setText("对方逃跑认输");
}
