#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <QObject>

// 宝石类
class Gemstone{

public:

    bool falling;

    int currentX;
    int currentY;

    int bomb_life;
    int GetType();
    Gemstone(int type_num);


private:

    int stone_type;
};

// 格子类
class Space {
public:
    Space();
    Space(int );
    ~Space();
    void SetGemstone(Gemstone* g);
    Gemstone* GetGemstone();
    void SetType(int );
    int GetType();

private:
    Gemstone* gemstone;
    int space_type;
};



#endif // GAMEMAP_H
