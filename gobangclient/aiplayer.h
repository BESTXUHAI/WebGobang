#ifndef AIPLAYER_H
#define AIPLAYER_H
#include "chessinfo.h"
#include <string>
#include <QMap>
#include <QString>
#include <QPoint>
#include <QVector>
#include <QTime>
class AIplayer
{
public:
    AIplayer(ChessType ** chessinfo);
    QPoint work(ChessType currentchesstype);
    void calculate();
    int tuplesum(int x,int y);
    bool reasonable(int x,int y);
    int singletruple(int x, int y, int dx, int dy);
    //棋盘信息
    ChessType ** chessInfo;
    int score[CHESSBOARD_SIZE][CHESSBOARD_SIZE];
    ChessType chesstype;
    QMap<QString, int> scoretable;
};

#endif // AIPLAYER_H
