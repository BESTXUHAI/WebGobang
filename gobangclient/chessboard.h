#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QWidget>
#include <QPaintEvent>
#include <QPen>
#include <QPainter>
#include <QColor>
#include <QPoint>
#include "chessinfo.h"
class ChessBoard : public QWidget
{
    Q_OBJECT
public:

    explicit ChessBoard(QWidget *parent = nullptr);
    ~ChessBoard();
    void init();
    QPoint getMousexy(int posx, int posy);
    ChessType ** getchessinfo();
    void setisPlayer(bool flag);
    bool getdroped();
    void setdroped(bool flag);
    QPoint getdropPoint();
    void setdropPoint(QPoint point);
    void setnowChesstype(ChessType nowchesstype);
    //设置棋盘落子
    void setdropChess(QPoint point);


protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    ChessType **chessInfo;
    int chessboard_width;
    int chess_width;
    int cell_width;
    int start_x;
    //玩家操作
    bool isPlayer;
    bool droped;
    ChessType nowChesstype;

    int pretipsChess_x;
    int pretipsChess_y;
    //最后一次落子点
    QPoint dropPoint;

};

#endif // CHESSBOARD_H
