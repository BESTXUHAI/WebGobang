#include "aiplayer.h"

AIplayer::AIplayer(ChessType ** chessinfo)
{
    //绑定棋盘信息
    chessInfo = chessinfo;
    //权值计算，0代表空，1代表当前棋子，2代表对方的棋子
    scoretable["00000"] = 7;
    scoretable["00001"] = 35;
    scoretable["00011"] = 800;
    scoretable["00111"] = 15000;
    scoretable["01111"] = 800000;
    scoretable["00002"] = 15;
    scoretable["00022"] = 400;
    scoretable["00222"] = 1800;
    scoretable["02222"] = 100000;
}

QPoint AIplayer::work(ChessType currentchesstype)
{
    chesstype = currentchesstype;
    //分数清零
    for(int i=0;i<CHESSBOARD_SIZE;i++)
        for(int j=0;j<CHESSBOARD_SIZE;j++)
            score[i][j] = 0;

    calculate();
    int maxsocore = -100;
    for(int i=0;i < CHESSBOARD_SIZE;i++)
        for(int j=0;j < CHESSBOARD_SIZE;j++)
        {
            if(score[i][j] > maxsocore)
            {
                maxsocore = score[i][j];
            }
        }
    //随机选择相同分数的点
    QVector<QPoint> vp;
    for(int i=0;i < CHESSBOARD_SIZE;i++)
        for(int j=0;j < CHESSBOARD_SIZE;j++)
            if(score[i][j] == maxsocore)
                vp.push_back(QPoint(i, j));
    //为了防止对零取余
    if(vp.size() == 0)return QPoint(0, 0);

    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    int len = qrand()%vp.size();
    chessInfo[vp[len].x()][vp[len].y()] = currentchesstype;
    return vp[len];
}

void AIplayer::calculate()
{
    for(int i=0;i<CHESSBOARD_SIZE;i++)
        for(int j=0;j<CHESSBOARD_SIZE;j++)
        {
            //不为空得零分
            if(chessInfo[i][j] != empty)score[i][j] = -1;
            else score[i][j] = tuplesum(i, j);
        }

}

bool AIplayer::reasonable(int x,int y)
{
    if(x>=0 && x<CHESSBOARD_SIZE && y>=0 && y<CHESSBOARD_SIZE)return true;
    return false;
}
//获取单个五元组的值
int AIplayer::singletruple(int x, int y, int dx, int dy)
{
    QString str("");
    for(int i=0;i<=4;i++)
    {
        if(chessInfo[x][y] == chesstype)
            str += "1";
        else if(chessInfo[x][y] == empty)
            str += "0";
        else
            str += "2";
        x += dx;
        y += dy;
    }
    std::sort(str.begin(), str.end());
    return scoretable[str];
}


int AIplayer::tuplesum(int x,int y)
{
    int sum = 0;
    int xx,yy;
    //横向
    for(xx=x-4,yy=y;xx<=x;xx++)
    {
        if(reasonable(xx,yy) && reasonable(xx+4,yy))
        {
            sum += singletruple(xx, yy, 1, 0);
        }
    }
    //竖向
    for(xx=x,yy=y-4;yy<=y;yy++)
    {
        if(reasonable(xx,yy) && reasonable(xx,yy+4))
        {
            sum += singletruple(xx, yy, 0, 1);
        }
    }
    //对角线
    for(xx=x-4,yy=y-4;xx<=x;xx++,yy++)
    {
        if(reasonable(xx,yy) && reasonable(xx+4,yy+4))
        {
            sum += singletruple(xx, yy, 1, 1);
        }
    }
    //反对角
    for(xx=x+4,yy=y-4;yy<=y;xx--,yy++)
    {
        if(reasonable(xx,yy) && reasonable(xx-4,yy+4))
        {
            sum += singletruple(xx, yy, -1, 1);
        }
    }

    return sum;
}
