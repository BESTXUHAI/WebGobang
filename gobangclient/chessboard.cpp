#include "chessboard.h"

ChessBoard::ChessBoard(QWidget *parent) : QWidget(parent)
{

    //固定棋盘大小
    chessboard_width = 600;
    cell_width = chessboard_width/(CHESSBOARD_SIZE);
    start_x = cell_width/2;
    chess_width = cell_width/20 * 19;
    this->resize(chessboard_width, chessboard_width);
    this->setMinimumSize(chessboard_width, chessboard_width);
    this->setMaximumSize(chessboard_width, chessboard_width);

    //动态分配棋盘
    chessInfo = new ChessType* [CHESSBOARD_SIZE];
    for(int i = 0; i < CHESSBOARD_SIZE; i++)
    {
        chessInfo[i] = new ChessType[CHESSBOARD_SIZE];
    }

    init();
}

ChessBoard::~ChessBoard()
{
    for(int i = 0; i < CHESSBOARD_SIZE; i++)
        delete [] chessInfo[i];
    delete [] chessInfo;
}


//返回棋盘
ChessType ** ChessBoard::getchessinfo()
{
    return chessInfo;
}
//开启或关闭鼠标追踪
void ChessBoard::setisPlayer(bool flag)
{
    isPlayer = flag;
    if(isPlayer)this->setMouseTracking(true);
    else this->setMouseTracking(false);
}
//获取当前是否落子
bool ChessBoard::getdroped()
{
    return droped;
}

void ChessBoard::setdroped(bool flag)
{
    droped = flag;
}
//获取当前落子的点位
QPoint ChessBoard::getdropPoint()
{
    return dropPoint;
}
//设置落子的点位
void ChessBoard::setdropPoint(QPoint point)
{
    dropPoint.setX(point.x());
    dropPoint.setY(point.y());
}

//改变棋子颜色
void ChessBoard::setnowChesstype(ChessType currentchesstype)
{
    nowChesstype = currentchesstype;
}



void ChessBoard::init()
{
    for(int i = 0; i < CHESSBOARD_SIZE; i++)
        for(int j = 0; j < CHESSBOARD_SIZE; j++)
            chessInfo[i][j] = empty;

    isPlayer = false;
    nowChesstype = whiteChess;
    pretipsChess_x = -1;
    pretipsChess_y = -1;
    this->update();


}
//绘制棋盘
void ChessBoard::paintEvent(QPaintEvent *event)
{

    //设置位图
    QPixmap chessBoardgrd(QString(":/images/chessboardgrd.jpg"));
    QPixmap blackimg(QString(":/images/blackchess.png"));
    QPixmap whiteimg(QString(":/images/whitechess.png"));
    QPixmap tipsimg(QString(":/images/tipschess.png"));
    //创建画家和画笔
    QPainter painter(this);
    //抗锯齿
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    //设置画笔
    QPen pen1,pen2;
    pen1.setColor(Qt::black);
    pen1.setWidth(2);
    pen2.setColor(Qt::black);
    pen2.setWidth(4);
    painter.setPen(pen1);


    //棋盘背景
    painter.drawPixmap(0, 0, chessboard_width, chessboard_width, chessBoardgrd);

    //绘制中间线条
    for(int i = 1; i < CHESSBOARD_SIZE - 1; i++)
    {
        painter.drawLine(start_x+i*cell_width, start_x, start_x+i*cell_width, start_x+chessboard_width-cell_width);
        painter.drawLine(start_x, start_x+i*cell_width, start_x+chessboard_width-cell_width, start_x+i*cell_width);
    }
    painter.setPen(pen2);
    painter.drawLine(start_x, start_x, start_x, start_x+chessboard_width-cell_width);
    painter.drawLine(start_x+chessboard_width-cell_width, start_x, start_x+chessboard_width-cell_width, start_x+chessboard_width-cell_width);
    painter.drawLine(start_x, start_x, start_x+chessboard_width-cell_width, start_x);
    painter.drawLine(start_x, start_x+chessboard_width-cell_width, start_x+chessboard_width-cell_width, start_x+chessboard_width-cell_width);

    //绘制棋子
    for(int i = 0; i < CHESSBOARD_SIZE; i++)
           for(int j = 0; j < CHESSBOARD_SIZE; j++)
           {
               //获取中心点坐标
                int xx = i*cell_width + start_x;
                int yy = j*cell_width + start_x;
                int half_chesswt = chess_width/2;
                if(chessInfo[i][j] == whiteChess)
                    painter.drawPixmap(xx - half_chesswt, yy - half_chesswt, chess_width, chess_width, whiteimg);
                else if(chessInfo[i][j] == blackChess)
                    painter.drawPixmap(xx - half_chesswt, yy - half_chesswt, chess_width, chess_width, blackimg);
                else if(chessInfo[i][j] == tipsChess)
                    painter.drawPixmap(xx - half_chesswt, yy - half_chesswt, chess_width, chess_width, tipsimg);
           }

}

//鼠标像素坐标转数组坐标
QPoint ChessBoard::getMousexy(int posx, int posy)
{
    int x = -1;
    int y = -1;
    for(int i = 0; i < CHESSBOARD_SIZE; i++)
        for(int j = 0; j < CHESSBOARD_SIZE; j++)
        {
            if(x >= 0) break;
            //获取中心点坐标
            int xx = i*cell_width + start_x;
            int yy = j*cell_width + start_x;
            if((posx >= xx - chess_width/2 && posx<= xx + chess_width/2)
                 && (posy >= yy - chess_width/2 && posy<= yy + chess_width/2))
            {
                x = i;
                y = j;
                break;
            }
        }
    return QPoint(x, y);
}
//鼠标移动事件追踪
void ChessBoard::mouseMoveEvent(QMouseEvent *event)
{
    if(!isPlayer)return ;
    int posx = event->x();
    int posy = event->y();
    QPoint pot = getMousexy(posx, posy);
    int x = pot.x(),y = pot.y();

        if(x < 0)return ;
        if(chessInfo[x][y] == empty)
        {
            chessInfo[x][y] = tipsChess;
            if(pretipsChess_x >= 0)chessInfo[pretipsChess_x][pretipsChess_y] = empty;
            pretipsChess_x = x;
            pretipsChess_y = y;
            this->update();
        }


}

//鼠标按压事件
void ChessBoard::mousePressEvent(QMouseEvent *event)
{
    if(!isPlayer)return ;
    int posx = event->x();
    int posy = event->y();
    QPoint pot = getMousexy(posx, posy);
    int x = pot.x(),y = pot.y();

    if(x < 0)return ;
    if(chessInfo[x][y] == tipsChess)
    {
        chessInfo[x][y] = nowChesstype;
        pretipsChess_x = -1;
        pretipsChess_y = -1;
        this->update();
        //下棋标志位
        droped = true;
        dropPoint.setX(x);
        dropPoint.setY(y);
    }

}

void ChessBoard::setdropChess(QPoint point)
{
    setdropPoint(point);
    chessInfo[point.x()][point.y()] = nowChesstype;
}
